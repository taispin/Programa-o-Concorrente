# ######################################### #
#										    #
# EP1 - MAC0438 - Programação Concorrente   #
#											#
# Bárbara de Castro Fernandes - 7577351	    #
# Taís Aparecida Pereira Pinheiro - 7580421 #
#											#
# ######################################### #

CC = gcc
CFLAGS = -O2 -Wall -pedantic -ansi -pthread

ep1: ep1.o
	$(CC) ep1.o -o ep1 $(CFLAGS)

ep1.o: ep1.c ep1.h
	$(CC) ep1.c -c -o ep1.o $(CFLAGS)

clean:
	rm -f ep1 *.o *~