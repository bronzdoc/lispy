GCC=gcc

default:
	$(GCC) -g -Wall parser.c mpc/mpc.c -lreadline -lm -o parser.o

