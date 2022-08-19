source = src/main.c

default: src/main.c
	gcc $(source) -I ../ -Wall -lglfw3