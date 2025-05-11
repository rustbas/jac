all: main.out

main.out: main.c files.o freq_table.o
	cc main.c *.o -ggdb --static -o main.out

files.o: files.c
	cc files.c --compile -o files.o

freq_table.o: freq_table.c
	cc freq_table.c --compile -o freq_table.o

clean:
	rm -f *.o *.out
