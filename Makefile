all: main.out

main.out: main.c
	cc main.c -ggdb -o main.out
