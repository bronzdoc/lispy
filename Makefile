GCC=gcc

default:
	$(GCC) -g -Wall lispy.c mpc/mpc.c -lreadline -lm -o lispy

