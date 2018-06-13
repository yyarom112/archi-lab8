#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <elf.h>
#include <sys/mman.h>

typedef struct main_menu{
    char *name;
    void (*func)();
}main_menu;

int fd;
void *map_start; /* will point to the start of the memory mapped file */
struct stat fd_stat; /* this is needed to  the size of the file */
Elf64_Ehdr *header; /* this will point to the header structure */
int num_of_section_headers;


void quit(){
    exit(0);
}

int open_file_stream(char* file_name,int option){
    // Check if filename is null, and if it is print an error message and return.
    if(!file_name){
        perror("file name is null\n");
        return -1;
    }
    // Open filename for reading. If this fails, print an error message and return.
    if((fd = open(file_name, option))<0){
        perror("Failed open file stream\n");
        return -1;
    }

    return 0;
}
char*  Get_File_Name(){
    char * file_name=malloc(100* sizeof(char));
    printf("Please enter a file name: ");
    scanf("%s", file_name);
    return file_name;
}
void Examine_ELF_File(){
    char* file_name=Get_File_Name();
    if(open_file_stream(file_name,O_RDWR)==-1)
        return;
    if(fstat(fd, &fd_stat) != 0 ) {
        perror("Cannot access to file state");
        return;
    }
    if (map_start != MAP_FAILED){
        munmap(map_start, fd_stat.st_size);
    }
    if ((map_start = mmap(0, (size_t) fd_stat.st_size, PROT_READ | PROT_WRITE , MAP_SHARED, fd, 0)) == MAP_FAILED ) {
        perror("mmap failed");
        exit(-4);
    }

    //now, the file is mapped starting at map_start.
    //the header point to the header file
    header = (Elf64_Ehdr *) map_start;
    printf("\n");
    printf("ELF Header:\n");

    //print the magic number
    printf("  Magic: \t\t\t%x %x %x\n", *(header->e_ident+1),*(header->e_ident+2),*(header->e_ident+3));
    if (*(header->e_ident+1)!='E' || *(header->e_ident+2)!='L' || *(header->e_ident+3)!='F') {
        printf("The number is not consistent with an ELF file!\n");
        return;
    }
    //printf("  Data: \t\t\t%s\n", (char *) header->e_machine);
    printf("  Entry point address: \t\t0x%x\n", (unsigned int) header->e_entry);
    printf("  Start of program headers: \t%d  (bytes into file)\n", header->e_phoff);
    printf("  Start of section headers: \t%d  (bytes into file)\n", header->e_shoff);
    printf("  Size of program headers: \t%d  (bytes)\n",header->e_phentsize);
    printf("  Size of section headers: \t%d  (bytes)\n", header->e_shentsize);
    printf("  Number of program headers: \t%d\n",header->e_phnum);
    printf("  Number of section headers: \t%d\n", header->e_shnum);
    printf("\n\n----------------------------------------------------------------------\n");


    free(file_name);
}


int main() {
    int input;

    struct main_menu menu[] = {{"1 - Examine ELF File", Examine_ELF_File},
                               {"2 - Exit",quit},{NULL,NULL}};
    int size_arr = sizeof(menu)/sizeof(main_menu);
    for( ; ; ){
        printf("Choose action: \n");
        for(int i=0; i<size_arr-1; i++){
            printf("%s \n", menu[i].name);
        }
        scanf("%d", &input);
        if(input<=0 || input>(size_arr-1)) {
            printf("Illegal input, please try again.\n");
            break;//TODO-remove this line
        }
        else {
            input--;
            menu[(input)].func();
        }
    }
    return 0;
}