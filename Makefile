CFLAGS=-ggdb
OBJ_FOLDER=obj

all: main.out

main.out: main.c files.o freq_table.o huffman.o
	$(CC) $(CFLAGS) main.c $(OBJ_FOLDER)/*.o --static -o main.out

files.o: files.c
	$(CC) $(CFLAGS) files.c --compile -o $(OBJ_FOLDER)/files.o

freq_table.o: freq_table.c
	$(CC) $(CFLAGS) freq_table.c --compile -o $(OBJ_FOLDER)/freq_table.o

huffman.o: huffman.c
	$(CC) $(CFLAGS) huffman.c --compile -o $(OBJ_FOLDER)/huffman.o	
clean:
	rm -f $(OBJ_FOLDER)/*.o *.out *.so

check: main.out
	./main.out -c -i data/data3.txt -o archive.jacz
	./main.out -x -i archive.jacz -o test.txt
	diff test.txt data/data3.txt
