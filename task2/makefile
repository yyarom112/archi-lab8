
all: myELF

myELF:  main.o 
	gcc -g -m64 -Wall -o myELF main.o 

main.o: main.c 
	gcc -g -m64 -Wall -c -o main.o main.c 
 

.PHONY: clean

clean: 
	rm -f *.o main
