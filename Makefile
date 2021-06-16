OPTS := -Wall -pedantic -std=c11

all: enigma

build/%.o: %.c
	gcc $(OPTS) -c $< -g -o $@


enigma: build/enigma.o build/readlines.o build/rotor.o
	gcc build/enigma.o build/readlines.o build/rotor.o -o build/enigma

make clean:
	rm -f build/*.o build/*.so build/enigma