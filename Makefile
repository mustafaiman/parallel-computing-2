all:
	mpicc main.c utils.h utils.c -lm
clean:
	rm -f a.out utils.h.gch