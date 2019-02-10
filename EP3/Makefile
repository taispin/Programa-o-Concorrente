# ######################################### #
#										    #
# EP3 - MAC0438 - Programação Concorrente   #
#											#
# Bárbara de Castro Fernandes - 7577351	    #
# Taís Aparecida Pereira Pinheiro - 7580421 #
#											#
# ######################################### #

CC = gcc
CFLAGS = -O2 -Wall -pedantic -ansi -pthread

ep3: ep3.o monitor.o
	$(CC) -o ep3 ep3.o monitor.o $(CFLAGS)

ep3.o: ep3.c monitor.h
	$(CC) ep3.c -c -o ep3.o $(CFLAGS)

monitor.o: monitor.h
	gcc -c monitor.c
clean:
	rm -f ep3 *.o *~
