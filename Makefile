all:
	mpicc main.c utils.h utils.c -lm
clean:
	rm *.out documentSearch *.gch