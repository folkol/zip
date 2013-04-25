all: fzip

fzip: main.o
	gcc main.o -lz -o fzip

main.o: src/main.c
	gcc -c src/main.c

clean:
	rm -rf *o fzip