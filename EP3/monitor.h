
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

typedef sem_t condicao;

typedef enum estado {PENSANDO, COMENDO, FAMINTO} Estado;

typedef struct filosofo
{
    int id;
    int peso;
    int refeicoesComidas;
    Estado estado;
} Filosofo;

/* Funções */
void inicializaMonitor(int n, int r, Filosofo *filosofos);
void inicializaControladores();
void *mallocSafe(size_t n);
void wait(condicao *var);
void signal(condicao *var);
void ficaFaminto(int i, Filosofo *filosofos);
int direita(int i);
int esquerda(int i);
void tentaPegarGarfos(int i, Filosofo *filosofos);
void soltaGarfos(int i,Filosofo *filosofos, int *refeicoes);
