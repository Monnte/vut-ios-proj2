CC=gcc
FLAGS= -std=gnu99 -Wall -Wextra -Werror -pedantic -lpthread

all:
	$(CC) proj2.c proj2.h -o proj2 $(FLAGS)