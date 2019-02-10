# ######################################### #
#										    #
# EP2 - MAC0438 - Programação Concorrente   #
#											#
# Bárbara de Castro Fernandes - 7577351	    #
# Taís Aparecida Pereira Pinheiro - 7580421 #
#											#
# ######################################### #

CC = gcc
CFLAGS = -O2 -Wall -pedantic -pthread -lgmp

ep2: ep2.o
	$(CC) ep2.o -o ep2 $(CFLAGS)

ep2.o: ep2.c ep2.h
	$(CC) ep2.c -c -o ep2.o $(CFLAGS)

clean:
	rm -f ep2 *.o *~
