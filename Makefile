OPTS := -Wall -pedantic -std=c11

all: frontend cracker_debug fastcracker

build/%.o: %.c
	gcc $(OPTS) -c $< -O3 -fpic -o $@


build/libenigma.so: build/enigma.o build/readlines.o build/rotor.o
	gcc build/enigma.o build/readlines.o build/rotor.o -shared -O3 -o build/libenigma.so

build/libdatastructures.so: build/data_structures/linked_list.o build/data_structures/hashmap.o
	gcc -shared -O3 -o build/libdatastructures.so build/data_structures/linked_list.o build/data_structures/hashmap.o

frontend: build/libenigma.so
	gcc -Lbuild -Wl,-rpath=build -O3 -o build/enigma frontend.c -lenigma

cracker_debug: build/libenigma.so build/libdatastructures.so
	gcc -Lbuild -Wl,-rpath=build -pg -g -o build/cracker cracker.c -lenigma -ldatastructures -lm

fastcracker: build/libenigma.so build/libdatastructures.so
	gcc -Lbuild -Wl,-rpath=build -O3 -o build/fastcracker cracker.c -lenigma -ldatastructures -lm

make clean:
	rm -f build/*.o build/*.so build/data_structures/*.o build/enigma build/cracker build/fastcracker