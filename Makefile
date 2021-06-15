OPTS := -Wall -Wextra -pedantic -std=c11

all: enigma

%.o: %.c
	gcc $(OPTS) -c $< -g -o build/$@


enigma: enigma.o readlines.o rotor.o
	gcc build/enigma.o build/readlines.o build/rotor.o -o build/enigma

make clean:
	rm -f build/*.o build/*.so build/enigma