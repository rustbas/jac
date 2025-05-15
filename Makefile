CFLAGS=-ggdb

all: main.out

main.out: main.c files.o freq_table.o huffman.o
	$(CC) $(CFLAGS) main.c *.o --static -o main.out

files.o: files.c
	$(CC) $(CFLAGS) files.c --compile -o files.o

freq_table.o: freq_table.c
	$(CC) $(CFLAGS) freq_table.c --compile -o freq_table.o

huffman.o: huffman.c
	$(CC) $(CFLAGS) huffman.c --compile -o huffman.o	
clean:
	rm -f *.o *.out *.so
