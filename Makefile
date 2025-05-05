all: main.out

main.out: main.c files.o
	cc main.c files.o -ggdb --static -o main.out

files.o: files.c
	cc files.c --compile -o files.o
