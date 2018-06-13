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

int check_fd(){
    if (map_start == MAP_FAILED) {
        printf("There are no file open\n");
        return -1;
    }
    return 1;
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
    if(check_fd()==-1)
        return;
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

unsigned numDigits(const unsigned n) {
    if (n < 10) return 1;
    return 1 + numDigits(n / 10);
}

void Print_Symbols(){
    int i=0,j=0;
    Elf64_Ehdr *header;
    Elf64_Sym *symbol_table;
    //check if the file is open
    if(check_fd()==-1)
        return;
    header = (Elf64_Ehdr *) map_start;
    //get the size of section table
    int snum = header->e_shnum;
    //get the offset of section table
    int soff = header->e_shoff;

    //point to the begin of the symbol table
    Elf64_Shdr* section_table = (Elf64_Shdr *)((char *)map_start + soff);
    //
    Elf64_Shdr* str_section_header = (Elf64_Shdr*)((char*)section_table + (header->e_shstrndx)*(header->e_shentsize));
    char* string_section_table = (char*)map_start + str_section_header->sh_offset;


    printf("\nSymbol Table:\n");
    printf("\t[index]    value                id         name           name\n");
    printf("--------------------------------------------------------------------------------\n");
    for(i=0;i<snum;i++){
        //if these sections hold a symbol table.
        if(section_table[i].sh_type == SHT_SYMTAB || section_table[i].sh_type == SHT_DYNSYM){
            //point to the first symbol in symbol table
            symbol_table =(Elf64_Sym*)((char*)map_start + section_table[i].sh_offset);
            // Section size in bytes/size of symbol table entry
            int table_size = section_table[i].sh_size/(sizeof(Elf64_Sym));

            //sh_link= Link to another section
            Elf64_Shdr* strtab = &section_table[section_table[i].sh_link];
            int offset = (int) strtab->sh_offset;
            const char * const table_ptr = map_start + offset;
            char * type;
            if(section_table[i].sh_type == SHT_SYMTAB){
                type=".symtab";
            }
            else{
                type=".dynsym";
            }
            printf("Symbol table '%s' contains %d entries:\n",type,table_size);
            printf(" ");
            printf("  Num:\t\tValue\t\t\tSection Idx\tSection_Name\t\tSymbol_Name\n");

            for(j=0;j<table_size;j++){
                if (j < 10)
                    printf("\t [ %d]\t", j);
                else
                    printf("\t [%d]\t", j);
                printf("%016lx\t",symbol_table[j].st_value);
                int idx;
                switch (symbol_table[j].st_shndx){
                    case 0:
                        printf("UND       ");
                        break;
                    case 65521:
                        printf("ABS       ");
                        break;
                    default:
                        //print the idx
                        idx= symbol_table[j].st_shndx;
                        printf("%d",idx);
                        int pad=10-numDigits(idx);
                        for(int k=0;k<pad;k++)
                            printf(" ");
                        Elf64_Shdr* sec = (Elf64_Shdr*)((char*)section_table +idx* header->e_shentsize);//index*entry size
                        printf("%s",(string_section_table + sec->sh_name));
                        for(int k=0;k<16-strlen(string_section_table + sec->sh_name);k++){
                            printf(" ");
                        }
                }
                printf("%s\n",table_ptr + symbol_table[j].st_name);
            }
        }
    }
}

int main() {
    int input;
    map_start = MAP_FAILED;
    struct main_menu menu[] = {{"1 - Examine ELF File", Examine_ELF_File},{"2 - Print Section Names", Print_Section_Names},
                               {"3 - Print Symbols", Print_Symbols}, {"4 - Exit",quit},{NULL,NULL}};
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