/* ***************************************** */
/*										     */
/* EP2 - MAC0438 - Programação Concorrente   */
/*											 */
/* Bárbara de Castro Fernandes - 7577351	 */
/* Taís Aparecida Pereira Pinheiro - 7580421 */
/*											 */
/* ***************************************** */

/* Bibliotecas */
#include <stdlib.h>
#include <gmp.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "ep2.h"

/* Macros */
#define TRUE 0
#define FALSE 1
#define NUM_DIGITOS 100000

/* Variáveis globais */
int N, ntermos = -1, nRodadas = 1, fim = FALSE, paradaConc = FALSE, rodadaInicial = TRUE;
char *M = NULL, *O = NULL;
mpf_t cosseno, dif, auxcos, X;                                       /* Variáveis GMP */
sem_t semCosseno, semCossenoAux, semTermo, semRodadas, semImpressao; /* Semáforos */
pthread_barrier_t barreira;                                          /* Barreira de sincronização */
pthread_t *threads;

int main(int argc, char *argv[])
{
    int n, p;
    char *modo = NULL, *opcional = NULL;
    mpf_t x;

    /* Pega argumentos passados na entrada e valida a entrada */
    if(argc != 5 && argc != 6)
    {
        printf("\nQuantidade inválida de parâmetros!\nUtilização: n [f|m] p x [d|s]\n\n");
        return EXIT_FAILURE;
    }

    n = atoi(argv[1]); /* Se n != 0, n threads serão criadas. Caso contrário, a
    quantidade de threads criadas será equivalente ao número de núcleos do computador */
    if(n == 0) n = calculaNucleos();
    modo = argv[2];    /* Recebe f ou m */
    p = atoi(argv[3]); /* Precisão do segundo argumento */

    /* Define precisão padrão */
    mpf_set_default_prec(NUM_DIGITOS / log10(2)); /* Define precisão padrão */

    /* Inicializa variáveis do GMP */
    mpf_init(cosseno);
    mpf_init(auxcos);
    mpf_init(dif);
    mpf_init(x);
    gmp_sscanf(argv[4], "%Ff", x);

    /* Opção d ou s*/
    if(argc == 6) opcional = argv[5];

    /* Inicializa Barreiras e Semáforos*/
    inicializaControladores(n);

    /* Globaliza as váriaveis de parametros */
    globalizaParametros(n, x, modo, opcional);

    /*Determina padrão de parada*/
    retornaDiferenca(p);

    /* Se opcional = 's'-> Calcula cos(x) de forma sequencial */
    if (argc == 6 && *opcional == 's') cossenoSequencial();
    else cossenoConcorrente();

    pthread_barrier_destroy(&barreira);

    return EXIT_SUCCESS;
}

/**--------------------------------------------------------------------------------------------------------------**/
/**------------------------------------------------ FUNÇÕES AUXILIARES ----- ------------------------------------**/

/********************************************************************************************************************/
/**                                     FUNÇÕES PARA CÁLCULO DE COS(X) CONCORRENTE                                 **/
/********************************************************************************************************************/

/* Inicializa o cálculo concorrente de cosseno(x) */
void cossenoConcorrente()
{
    int i;
    int *thread_id = mallocSafe(N * sizeof(int));           /* Vetor de id de threads */
    threads = mallocSafe(N * sizeof(pthread_t));            /* Vetor das threads do programa */

    printf("\nProcessando...\n");

    if(O != NULL && *O == 'd') printf("\n %dª Rodada:\n",nRodadas);

    for(i = 0; i < N; i++)
    {
        thread_id[i] = i;
        pthread_create(&threads[i], NULL, calculaCossenoConc, (void *) &thread_id[i]);
    }
    pthread_exit(NULL);
}

/* Gerencia o comportamento das threads */
void *calculaCossenoConc(void *param)
{
    int id = *((int *)(param));
    mpf_t vazio;
    mpf_init(vazio);

    /* Enquanto um critério de parada não for atendido: */
    while(paradaConc == FALSE)
    {
        sem_wait(&semTermo);
        ntermos++;
        sem_post(&semTermo);

        /* Calcula um novo termo */
        calculaTermoCossenoConc(id, ntermos);

        if(O != NULL && *O == 'd')
        {
            sem_wait(&semImpressao);
            printf("\nThread %d atinge barreira\n", id);
            sem_post(&semImpressao);
        }

        if(pthread_barrier_wait(&barreira) == PTHREAD_BARRIER_SERIAL_THREAD)
        {
            if(O != NULL && *O == 'd')
            {
                sem_wait(&semImpressao);
                imprimeResultado(0);
                printf("\nRodada %dª:\n", nRodadas + 1);
                sem_post(&semImpressao);
            }
            /* Se modo = 'f' e N threads atigiram a barreira, verifica-se o critério de parada*/
            if(*M == 'f') condicaoDeParadaConc(vazio);
            sem_wait(&semRodadas);
            nRodadas++;
            sem_post(&semRodadas);
        }
        if(paradaConc == TRUE) pthread_exit(NULL);
    }
    mpf_clear(vazio);
    return NULL;
}

/* Cálculo de um termo de uma thread                                    */
/* Se *M = 'm', o critério de parada é verficado para este novo termo   */
void calculaTermoCossenoConc(int id, int n)
{
    mpf_t termo;
    mpf_init(termo);

    if(paradaConc == TRUE) pthread_exit(NULL);

    calculaTermoCosseno(termo, X, n);

    if(*M == 'f') incrementaCosseno(termo, 'a');
    else
    {
        incrementaCosseno(termo, 'c');
        condicaoDeParadaConc(termo);
    }
    mpf_clear(termo);
}

/* Atualiza valor de cosseno se c == 'c' ou cosseno auxiliar = 'a' */
void incrementaCosseno(mpf_t termo, char c)
{
    mpf_t temp;
    mpf_init(temp);

    if(paradaConc == TRUE) pthread_exit(NULL);

    if(c == 'c' )
    {
        sem_wait(&semCosseno); /* Mesmo que P(semCosseno) */

        mpf_add(temp, cosseno, termo);
        mpf_set(cosseno, temp);

        sem_post(&semCosseno); /* Mesmo que V(semCosseno) */
    }
    else
    {
        sem_wait(&semCossenoAux); /* Mesmo que P(semCossenoAux) */

        mpf_add(temp, auxcos, termo);
        mpf_set(auxcos, temp);

        sem_post(&semCossenoAux); /* Mesmo que V(semCossenoAux) */
    }

    mpf_clear(temp);
}

/* Recebe um novo termo e verifica se a condição de parada é atendida.      */
/* Se *M = 'f', este termo é vazio e a comparação é feita por dois valores  */
/* de cos(x) produzidos em duas rodadas consecutivas.                       */
void condicaoDeParadaConc(mpf_t termo)
{
    mpf_t temp;
    mpf_init(temp);

    if(paradaConc == TRUE) pthread_exit(NULL);

    switch(*M)
    {
    /* Verifica se módulo de (cos(x) - cos(x) da última rodada) é menor que dif */
    case 'f':
        if(rodadaInicial)
        {
            rodadaInicial = FALSE;
            sem_wait(&semCosseno);
            mpf_set(cosseno, auxcos);
            sem_post(&semCosseno);
        }
        else
        {
            if(mpf_cmp(cosseno, auxcos) >= 0) mpf_sub(temp, cosseno, auxcos);
            else mpf_sub(temp, auxcos, cosseno);
            if(mpf_cmp(temp, dif) < 0)
            {
                sem_wait(&semCosseno);
                paradaConc = TRUE;
                mpf_set(cosseno, auxcos);
                sem_post(&semCosseno);

                sem_wait(&semImpressao);
                imprimeResultado(1);
                exit(EXIT_SUCCESS);
                sem_post(&semImpressao);
            }
            else
            {
                if(paradaConc == TRUE) pthread_exit(NULL);
                else mpf_set(cosseno, auxcos);
            }
            break;
        }
    /* Verifica se módulo do termo recebido como parâmetro é menor que dif */
    case 'm':
        mpf_abs(temp, termo);
        if(mpf_cmp(temp, dif) < 0)
        {
            paradaConc = TRUE;
            sem_wait(&semImpressao);
            imprimeResultado(1);
            exit(EXIT_SUCCESS);
            sem_post(&semImpressao);
        }
        break;
    }
    mpf_clear(temp);

}

/********************************************************************************************************************/
/**                                     FUNÇÕES PARA CÁLCULO DE COS(X) SEQUENCIAL                                  **/
/********************************************************************************************************************/

/* Calcula o valor de cos(x) sequencialmente do modo 'f' ou 'm' com precisão 'p' */
void cossenoSequencial()
{
    int parada = FALSE, n = 0, ant = TRUE;

    mpf_t penultimoTermo, ultimoTermo, penultimoCos, ultimoCos;

    mpf_init(penultimoTermo);
    mpf_init(ultimoTermo);
    mpf_init(penultimoCos);
    mpf_init(ultimoCos);


    while(parada != TRUE)
    {
        if(ant == TRUE) calculaTermoCosseno(penultimoTermo, X, n); /* Calcula um termo e incrementa n */

        if(*M == 'f')
        {
            calculaTermoCosseno(ultimoTermo, X, ++n);

            if(ant == TRUE)
            {
                mpf_add(penultimoCos, cosseno, penultimoTermo); /* penultimoCos = cosseno + penultimoTermo */
                mpf_add(ultimoCos, penultimoCos, ultimoTermo);  /* ultimoCos = penultimoCos + ultimoTermo */
                ant = FALSE;
            }
            else mpf_add(ultimoCos, penultimoCos, ultimoTermo); /* ultimoCos = penultimoCos + ultimoTermo */

            if((condicaoDeParada(penultimoCos, ultimoCos)) == TRUE) parada = TRUE;
            else mpf_set(penultimoCos, ultimoCos);

            mpf_set(cosseno, ultimoCos);                        /* Atualiza valor de cosseno */
            imprimeResultado(0);
        }
        else
        {
            if(condicaoDeParada(ultimoTermo, penultimoTermo) == TRUE) parada = TRUE;
            else n++;

            mpf_add(penultimoCos, cosseno, penultimoTermo);  /* penultimoCos = cosseno + penultimoTermo */
            mpf_set(cosseno, penultimoCos);                  /* Atualiza valor de cosseno */
            imprimeResultado(0);
        }
    }

    imprimeResultado(n);

    mpf_clear(penultimoTermo);
    mpf_clear(ultimoTermo);
    mpf_clear(penultimoCos);
    mpf_clear(ultimoCos);
}

/* Recebe dois valores e devolve TRUE se o critério de parada foi atendido*/
int condicaoDeParada(mpf_t penultimo, mpf_t ultimo)
{
    int bool = FALSE;
    mpf_t temp;

    mpf_init(temp);
    switch(*M)
    {
    /* Para quando o módulo de (penultimo - ultimo) for menor que dif */
    case 'f':
        if(mpf_cmp(penultimo, ultimo) >= 0) mpf_sub(temp, penultimo, ultimo);
        else mpf_sub(temp, ultimo, penultimo);
        if(mpf_cmp(temp, dif) < 0) bool = TRUE;
        break;
    /* Para quando o último termo calculado da série for menor que dif */
    case 'm':
        mpf_abs(temp, ultimo);
        if(mpf_cmp(temp, dif) < 0) bool = TRUE;
        break;
    }
    mpf_clear(temp);

    return bool;
}

/********************************************************************************************************************/
/**                                         FUNÇÕES PARA CÁLCULO DE UM TERMO                                       **/
/********************************************************************************************************************/

/** Potência **/
/* Calcula pot = b ^ e, e >= 0  */
void potencia(mpf_t pot, mpf_t b, int e)
{
    mpf_t base;
    mpf_init(base);

    mpf_set(base, b);
    mpf_init_set_ui(pot, 1);

    /* op1 * op2 */
    while(e != 0)
    {
        mpf_mul(pot, pot, base);
        e--;
    }
}

/** Fatorial **/
/* Recebe um mpf_t n e devolve n! */
void fatorial(mpf_t res, int n)
{
    int i;

    mpf_init_set_ui(res, 1);

    for(i = n; i > 0; i--)
    {
        mpf_mul_ui(res, res, i);
    }
}

/* Calcula (-1) ^ n */
int partA(int n)
{
    if((n % 2) == 0) return 1;
    else return -1;
}

/* Recebe um inteiro n e calcula b = (2n)! */
void partB(mpf_t b, int n)
{
    fatorial(b, (2 * n));
}

/* Recebe um inteiro n e calcula (x ^ 2n)! */
void partC(mpf_t c, mpf_t x, int n)
{
    potencia(c, x, (2 * n));
}

/* Calcula o n-ésimo termo da série do cosseno */
void calculaTermoCosseno(mpf_t termo, mpf_t x, int n)
{
    int a = partA(n); /* Verifica se n é par ou impar*/

    /* Variaveis auxiliares */
    mpf_t b, c, temp;
    mpf_init(b);
    mpf_init(c);
    mpf_init(temp);

    partB(b, n); /* Computa (2n)!*/
    partC(c, x, n); /* Computa (x ^ 2n)! */

    if(a == 1) mpf_div(termo, c, b);
    else
    {
        mpf_neg(temp, c);
        mpf_div(termo, temp, b);
    }

    mpf_clear(b);
    mpf_clear(c);
    mpf_clear(temp);
}

/********************************************************************************************************************/
/**                                                 FUNÇÃO DE IMPRESSÃO                                            **/
/********************************************************************************************************************/

/* Recebe o número de termos. Se n = 0, imprime valores parciais de cos(x); caso contrário, imprime o valor final e n */
void imprimeResultado(int n)
{
    if(n == 0)
    {
        if(O != NULL && *O == 'd') gmp_printf("\n Valor parcial de cos(x): %.*Ff\n", NUM_DIGITOS, auxcos);
        else gmp_printf("\n Valor parcial de cos(x): %.*Ff\n", NUM_DIGITOS, cosseno);
    }
    else
    {
        printf("\n\n -> Término da execução.\n");
        gmp_printf("\n -> Valor encontrado para cos(x): %.*Ff\n", NUM_DIGITOS, cosseno);
        if(O != NULL && *O == 's') printf("\n -> Quantidade de termos: %d.\n\n", n);
        else printf("\n -> Número de rodadas: %d\n\n", nRodadas);
    }
}

/********************************************************************************************************************/
/**                                       FUNÇÕES AUXILIARES E DE INICIALIZAÇÃO                                    **/
/********************************************************************************************************************/

/* Recebe n, x, *modo e *opcional e transforma-os em variáveis globais */
void globalizaParametros(int n, mpf_t x, char *modo, char *opcional)
{
    M = modo;
    N = n;
    O = opcional;
    mpf_init(X);
    mpf_set(X, x);
}

/* Inicializa todos os semáforos */
void inicializaControladores(int n)
{
    if(pthread_barrier_init(&barreira, NULL, n))
    {
        printf("\nErro na criação da barreira!\n");
        exit(EXIT_FAILURE);
    }

    /* Controla a atualização do valor de cosseno */
    if(sem_init(&semCosseno, 0, 1))
    {
        printf("Erro ao criar o semáforo!\n");
        exit(EXIT_FAILURE);
    }

    /* Controla a quantidade de termos */
    if(sem_init(&semTermo, 0, 1))
    {
        printf("Erro ao criar o semáforo!\n");
        exit(EXIT_FAILURE);
    }

    /* Controla a atualização do valor de cosseno auxiliar */
    if(sem_init(&semCossenoAux, 0, 1))
    {
        printf("Erro ao criar o semáforo!\n");
        exit(EXIT_FAILURE);
    }

    /* Controla a atualização do valor de rodadas */
    if(sem_init(&semRodadas, 0, 1))
    {
        printf("Erro ao criar o semáforo!\n");
        exit(EXIT_FAILURE);
    }

    /* Controla a impressão dos valores*/
    if(sem_init(&semImpressao, 0, 1))
    {
        printf("Erro ao criar o semáforo!\n");
        exit(EXIT_FAILURE);
    }

}

/* Calcula a quantidade de núcleos existente no computador */
int calculaNucleos()
{
    FILE *f;
    int i;
    char *c = mallocSafe(3 * sizeof(char));

    if(system("nproc > nproc_temp"))
    {
        printf("Erro ao calcular número de núcleos!\n");
        exit(EXIT_FAILURE);
    }
    f = fopen("nproc_temp", "r");
    if(fgets(c, 3, f) == NULL)
    {
        printf("Erro ao calcular número de núcleos!\n");
        exit(EXIT_FAILURE);
    }
    i = atoi(c);
    fclose(f);
    remove("nproc_temp");

    return i;
}

/* Recebe um parâmetro p e calcula dif = 10 ^ (-p) */
void retornaDiferenca(int p)
{
    mpf_t um;
    mpf_t pot;
    mpf_t base;

    mpf_init(um);
    mpf_init(pot);
    mpf_init(base);


    mpf_init_set_ui(um, 1);
    mpf_init_set_ui(base, 10);

    if(p > 0)
    {
        potencia(pot, base, p);
        mpf_div(dif, um, pot);
    }
    else if(p < 0)
    {
        potencia(pot, base, (-1) * p);
        mpf_set(dif, pot);
    }
    else mpf_set(dif, um);
}

/* Aloca a memória requerida pela variável e devolve uma mensagem de erro caso a memória não seja alocada corretamente */
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
