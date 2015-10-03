GCC=gcc

default:
	$(GCC) -Wall parser.c mpc/mpc.c -lreadline -lm -o parser

