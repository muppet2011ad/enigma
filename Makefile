OPTS := -Wall -pedantic -std=c11

all: frontend cracker fastcracker

build/%.o: %.c
	gcc $(OPTS) -c $< -O3 -fpic -o $@


build/libenigma.so: build/enigma.o build/readlines.o build/rotor.o
	gcc build/enigma.o build/readlines.o build/rotor.o -shared -O3 -o build/libenigma.so

frontend: build/libenigma.so
	gcc -Lbuild -Wl,-rpath=build -O3 -o build/enigma frontend.c -lenigma

cracker_debug: build/libenigma.so
	gcc -Lbuild -Wl,-rpath=build -pg -g -o build/cracker cracker.c -lenigma

fastcracker: build/libenigma.so
	gcc -Lbuild -Wl,-rpath=build -O3 -o build/fastcracker cracker.c -lenigma

make clean:
	rm -f build/*.o build/*.so build/enigma build/cracker build/fastcracker