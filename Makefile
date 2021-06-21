OPTS := -Wall -pedantic -std=c11

all: frontend

build/%.o: %.c
	gcc $(OPTS) -c $< -g -fpic -o $@


build/libenigma.so: build/enigma.o build/readlines.o build/rotor.o
	gcc build/enigma.o build/readlines.o build/rotor.o -shared -o build/libenigma.so

frontend: build/libenigma.so
	gcc -Lbuild -Wl,-rpath=build -o build/enigma frontend.c -lenigma

make clean:
	rm -f build/*.o build/*.so build/enigma