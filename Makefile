OPTS := -Wall -Wextra -pedantic -std=c11

all: enigma

%.o: %.c
	gcc $(OPTS) -c $< -o $@


enigma: enigma.o readlines.o
	gcc enigma.o readlines.o -o enigma

make clean:
	rm -f *.o *.so enigma