/* ***************************************** */
/*										     */
/* EP3 - MAC0438 - Programação Concorrente   */
/*											 */
/* Bárbara de Castro Fernandes - 7577351	 */
/* Taís Aparecida Pereira Pinheiro - 7580421 */
/*											 */
/* ***************************************** */


#include "monitor.h"

int N, R, refeicoes = 0;

/* Variáveis de Condição */
condicao *semFilosofos;
sem_t semGarfos,semRefeicoes, semImpressao;

/* *************************************************************************************** */
/* *****************************  Inicialização do Monitor ******************************* */

void inicializaMonitor(int n, int r, Filosofo *filosofos)
{
    int i;
    N = n;
    R = r;
    for(i = 0; i < n; i++)
    {
        filosofos[i].id = i + 1; /* Id dos filósofos irá de 1 até N */
        filosofos[i].refeicoesComidas = 0;
        filosofos[i].estado = PENSANDO;
    }
    inicializaControladores();
}

/* Inicialização das variáveis de condição */
void inicializaControladores()
{
    int i;

    /* Controla a utilização dos garfos */
    if(sem_init(&semGarfos, 0, 1))
    {
        printf("Erro ao iniciar variavel semGarfos!\n");
        exit(EXIT_FAILURE);
    }

    /* Controla quando um filósofo está comendo */
    semFilosofos = mallocSafe(N * sizeof(condicao));
    for(i = 0; i < N; i++)
    {
        if(sem_init(&semFilosofos[i], 0, 1))
        {
            printf("Erro ao iniciar variavel semFilosofos!\n");
            exit(EXIT_FAILURE);
        }
    }

    /* Controla a quantidade de refeições já consumidas */
    if(sem_init(&semRefeicoes, 0, 1))
    {
        printf("Erro ao iniciar variavel semRefeicoes!\n");
        exit(EXIT_FAILURE);
    }

    /* Controla a impressão dos valores*/
    if(sem_init(&semImpressao, 0, 1))
    {
        printf("Erro ao criar iniciar variavel semImpressao!\n");
        exit(EXIT_FAILURE);
    }
}

/* *************************************************************************************** */
/* *****************  Implementação das Funções wait(cv) e signal(cv) ******************** */


void wait(condicao *var)
{
    sem_wait(var);
}

void signal(condicao *var)
{
    sem_post(var);
}

/* *************************************************************************************** */
/* *********************************  Funções Auxiliares ********************************* */

/* Devolve o índice do filósofo à esquerda do atual */
int esquerda(int i)
{
    return (i - 1) % N;
}

/* Devolve o índice do filósofo à direita do atual */
int direita(int i)
{
    return (i + 1) % N;
}

/* *************************************************************************************** */
/* **********************************  Métodos do Monitor ******************************** */

/* Faz com que o i-ésimo filósofo fique faminto e, consequentemente, tente pegar os garfos */
void ficaFaminto(int i, Filosofo *filosofos)
{
    sem_wait(&semGarfos);
    filosofos[i].estado = FAMINTO;

    sem_wait(&semImpressao);
    printf("Filósofo %d ficou com fome\n", filosofos[i].id);
    sem_post(&semImpressao);

    tentaPegarGarfos(i,filosofos);

    sem_post(&semGarfos);
    /*sem_wait(&semFilosofos[i]);*/
}

/* Faz com que o filósofo tente pegar os garfos para comer */
void tentaPegarGarfos(int i, Filosofo *filosofos)
{
    if(filosofos[i].estado == FAMINTO && filosofos[esquerda(i)].estado != COMENDO && filosofos[direita(i)].estado != COMENDO)
    {
        filosofos[i].estado = COMENDO;

        sem_wait(&semImpressao);
        printf("Filósofo %d está comendo\n", filosofos[i].id);
        sem_post(&semImpressao);

        signal(&semFilosofos[i]);
    }
    else
        wait(&semFilosofos[i]);
}

/* Solta os garfos e os deixa disponíveis para caso os filósofos vizinhos estejam famintos */
void soltaGarfos(int i, Filosofo *filosofos, int *ref)
{
    sem_wait(&semGarfos);
    filosofos[i].refeicoesComidas++;

    sem_wait(&semRefeicoes);
    refeicoes++;
    *ref = refeicoes;
    sem_post(&semRefeicoes);

    filosofos[i].estado = PENSANDO;

    sem_wait(&semImpressao);
    printf("Filósofo %d terminou de comer\n", filosofos[i].id);
    printf("\nrefeicoesComidas: %d\n\n", refeicoes);
    sem_post(&semImpressao);

    tentaPegarGarfos(esquerda(i),filosofos);
    tentaPegarGarfos(direita(i),filosofos);
    sem_post(&semGarfos);
}



/* Aloca a memória requerida pela variável e devolve uma mensuagem de erro caso a memória não seja alocada corretamente */
void *mallocSafe(size_t n)
{
    void *pt;
    pt = malloc(n);
    if(pt == NULL)
    {
        printf("Erro na alocação de memória!\n");
        exit(EXIT_FAILURE);
    }
    return pt;
}
