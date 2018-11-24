.PHONY: build run clean

build: 2048

2048: 2048.o
	gcc -Wall 2048.o -o 2048 -lncurses -lpanel
2048.o: 2048.c
	gcc -c 2048.c -o 2048.o
run: 2048
	./2048
clean:
	rm -rf 2048.o 2048
