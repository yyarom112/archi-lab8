#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <elf.h>
#include <sys/mman.h>
#include <memory.h>
#include <unistd.h>

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
    if(fd!=-1){
        munmap(map_start, (size_t) fd_stat.st_size);
        close( fd );
    }
    exit(EXIT_SUCCESS);
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

void print_Section_format(char* pointer,Elf64_Shdr *sections, int section_index ){
    int i=0;
    for(;i<section_index;i++){
        if(i<10)
            printf("\t [ %d]\t",i );
        else
            printf("\t [%d]\t",i);
        printf("%s   ",pointer + sections[i].sh_name);
        size_t tmp=strlen((pointer + sections[i].sh_name));
        for(;tmp<10;tmp++)
            printf(" ");
        printf("%08lx \t %06lx    %06lx    %06x\n",sections[i].sh_addr,sections[i].sh_offset ,sections[i].sh_size,sections[i].sh_type);
    }

}

void Print_Section_Names(){
    Elf64_Ehdr *header;
    //check if the file is open
    if (map_start == MAP_FAILED) {
        printf("There are no file open\n");
        return;
    }
    header = (Elf64_Ehdr *) map_start;
    /* Section header table file offset */
    int section_offset = header->e_shoff;
    /* Section header table entry count */
    int section_index = header->e_shnum;

    //move the pointer ro point on the section table
    Elf64_Shdr *sections = (Elf64_Shdr *)((char *)map_start + section_offset);
    /* Section header string table index */
    Elf64_Shdr *point_to_section = &sections[header->e_shstrndx];
    /* Section file offset */
    char* pointer = map_start+point_to_section->sh_offset;/* pointer points to the string table of section headeras*/
    printf("\nSection Header:\n");
    printf("\t[index]   name       address     offset     size       type\n");
    printf("--------------------------------------------------------------------------\n");
    print_Section_format(pointer,sections, section_index);

}


int main() {
    int input;
    map_start = MAP_FAILED;
    struct main_menu menu[] = {{"1 - Examine ELF File", Examine_ELF_File},{"2 - Print Section Names", Print_Section_Names},
                               {"3 - Exit",quit},{NULL,NULL}};
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