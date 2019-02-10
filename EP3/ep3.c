/* ***************************************** */
/*										     */
/* EP3 - MAC0438 - Programação Concorrente   */
/*											 */
/* Bárbara de Castro Fernandes - 7577351	 */
/* Taís Aparecida Pereira Pinheiro - 7580421 */
/*											 */
/* ***************************************** */
#include "monitor.h"

/* Macros */
#define TRUE 0
#define FALSE 1

/* Funções */
void globalizaVariaveis(int n, int r);
void situacaoUniforme();
void situacaoComPeso();
void *filosofoModoUniforme(void *param);


/* Variáveis globais */
int N, R, refeicoesComidas = 0;
Filosofo *filosofos;
pthread_t *threads;


/* *************************************************************************************** */
/* *********************************  Funções Principal ********************************* */

int main(int argc, char *argv[])
{
    char *opcao;
    int i, n, r;
    FILE *f;

    /* Pega os argumentos passados e valida a entrada */
    if(argc != 4)
    {
        printf("Quantidade inválida de parâmetros! Utilização: arquivo R U|P\n");
        return EXIT_FAILURE;
    }

    f = fopen(argv[1], "r");
    if(fscanf(f, "%i", &n) == 0)
    {
        printf("Erro na leitura do arquivo texto!\n");
        return EXIT_FAILURE;
    }

    /* Inicializa variável proncipal*/
    filosofos = mallocSafe(n * sizeof(Filosofo));

    for(i = 0; i < n; i++)
    {
        if(fscanf(f, "%i", &filosofos[i].peso) == 0)
        {
            printf("Erro na leitura do arquivo texto!");
            return EXIT_FAILURE;
        }
    }
    fclose(f);

    r = atoi(argv[2]);
    opcao = argv[3];

    /* Inicializa o monitor*/
    inicializaMonitor(n,r,filosofos);

    if(*opcao == 'u' || *opcao == 'U') situacaoUniforme();
    else situacaoComPeso();

    return EXIT_SUCCESS;
}

/* Recebe n e r e transforma-os em variáveis globais */
void globalizaVariaveis(int n, int r)
{
    N = n;
    R = r;
}


/* *************************************************************************************** */
/* *********************************  Situação Uniforme ********************************* */
void situacaoUniforme()
{
    int i;

    threads = mallocSafe(N * sizeof(pthread_t));
    for(i = 0; i < N; i++)
        pthread_create(&threads[i], NULL, filosofoModoUniforme, (void *) &i);
}

void *filosofoModoUniforme(void *param)
{
    int i = *((int *)(param));

    while(refeicoesComidas < R)
    {
        /* pensa */
        ficaFaminto(i,filosofos);
        /* come */
        soltaGarfos(i, filosofos, &refeicoesComidas);

    }
    return NULL;
}

void situacaoComPeso()
{

}
