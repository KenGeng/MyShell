MyShell: main.o MyShell.o
	gcc -o MyShell main.o MyShell.o
main.o:   main.c
	gcc -w -c main.c -o main.o
MyShell.o: MyShell.c
	gcc -w -c MyShell.c -o MyShell.o
clean:
	rm -f *.o

