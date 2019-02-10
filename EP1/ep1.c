/* ***************************************** */
/*										     */
/* EP1 - MAC0438 - Programação Concorrente   */
/*											 */
/* Bárbara de Castro Fernandes - 7577351	 */
/* Taís Aparecida Pereira Pinheiro - 7580421 */
/*											 */
/* ***************************************** */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include "ep1.h"

/* Variáveis globais */
int N, D, elimina = 0, uniforme = FALSE, debug = FALSE;
int nVoltas = 0; /* nVoltas calcula a quantidade de voltas que o primeiro colocado deu */
Ciclista *ciclistas; /* Vetor de ciclistas */
Lugar *pista; /* Vetor com as posições da pista */
pthread_t *threads; /* Vetor das threads do programa */

sem_t mutex; /* Semáforo para controlar o acesso à cada ciclista*/
sem_t control; /* Semáforo para destruição das threads */
sem_t control_pista; /* Semáforo para destruição das threads */
sem_t contador; /* Semáforo para controlar os eliminados */
sem_t global; /* Semáforo para controlar as variáveis compartilhadas*/
sem_t global2; /* Semáforo para controlar as variáveis compartilhadas*/


int main(int argc, char *argv[])
{
    int d, n;

    /* Pega argumentos passados na entrada e valida a entrada */
    if(argc != 4 && argc != 5)
    {
        printf("\nQuantidade inválida de parâmetros!\nUtilização: d n [v|u], onde v é usado para definir "
               "simulações com velocidades aleatórias a cada volta e u para definir simulações com velocidades "
               "uniformes de 50Km/h.\n");
        return EXIT_FAILURE;
    }
    d = atoi(argv[1]);
    n = atoi(argv[2]);
    if(argc == 5 && strcmp(argv[4], "debug")) debug = TRUE;

    globalizaVariaveis(n, d);
    inicializaSemaforos();

    /* Iniciação da Pista */
    pista = mallocSafe(d * sizeof(Lugar));
    inicializaPista();

    /* Iniciação dos Ciclistas */
    ciclistas = mallocSafe(n * sizeof(Ciclista));
    defineId();
    definePosicoesIniciais();

    /* Corrida com Velocidade de 25Km/h na primeira volta e aleatória nas restantes */
    if(argv[3][0] == 'v')
    {
        realizaCorrida(25);
    }

    /* Corrida com Velocidade Constante de 50Km/h a cada volta*/
    else if(argv[3][0] == 'u')
    {
        uniforme = TRUE;
        realizaCorrida(50);
    }
    else
    {
        printf("Parâmetro inválido!\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/* --------------------------------------------------------------------------------------------------------------*/
/* ------------------------------------------------ FUNÇÕES AUXILIARES ----- ------------------------------------*/

/* Globaliza as variáveis n e d */
void globalizaVariaveis(int n, int d)
{
    N = n;
    D = d;
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


/* --------------------------------------------------------------------------------------------------------------*/
/* ------------------------------------------ FUNÇÕES PARA INICIAÇÃO  -------------------------------------------*/

/* Inicializa todos os semáforos */
void inicializaSemaforos()
{
    /* Controla a destruição das threads*/
    if (sem_init(&control, 0, 1))
    {
        printf("Erro ao criar o semáforo!\n");
        exit(EXIT_FAILURE);
    }

    /* Controla o acesso à pista */
    if (sem_init(&control_pista, 0, 1))
    {
        printf("Erro ao criar o semáforo!\n");
        exit(EXIT_FAILURE);
    }

    /* Controla a criação dos ciclistas */
    if (sem_init(&mutex, 0, 1))
    {
        printf("Erro ao criar o semáforo!\n");
        exit(EXIT_FAILURE);
    }

    /* Controla as variáveis globais */
    if (sem_init(&global, 0, 1))
    {
        printf("Erro ao criar o semáforo!\n");
        exit(EXIT_FAILURE);
    }

    /* Controla as variáveis globais */
    if (sem_init(&global2, 0, 1))
    {
        printf("Erro ao criar o semáforo!\n");
        exit(EXIT_FAILURE);
    }
}

/* Inicializa cada metro da pista com o valor VAZIO para indicar que aquela colocação está vazia */
void inicializaPista()
{
    int i;

    for(i = 0; i < D; i++)
    {
        pista[i].cic1 = VAZIO;
        pista[i].cic2 = VAZIO;
        pista[i].cic3 = VAZIO;
        pista[i].cic4 = VAZIO;
    }
}

/* Embaralha a posição inicial dos ciclistas de forma aleatória */
void definePosicoesIniciais()
{
    int i, j, k = N;

    /* Atribui a cada ciclista uma posição entre 1 e n */
    for(i = 0; i < k; i++)
    {
        ciclistas[i].colocacao = i + 1;
    }

    /* Decrementa k, sorteia um número entre 0 e k-1, e troca a posição entre este elemento e k */
    while(--k > 0)
    {
        i = rand() % k;
        j = ciclistas[i].colocacao;
        ciclistas[i].colocacao = ciclistas[k].colocacao;
        ciclistas[k].colocacao = j;
    }
    for(i = 0; i < N; i++)
    {
        k = ciclistas[i].colocacao;
        pista[D-N+k-1].cic1 = ciclistas[i].id;
    }
}

/* Define o ID de um determinado ciclista */
void defineId()
{
    int i;

    for(i = 0; i < N; i++) ciclistas[i].id = i;
}

/* Define a velocidade de um determinado ciclista */
void defineVelocidade(int id, float vel)
{
    ciclistas[id].velocidade = vel;
}

/*-----------------------------------------------------------------------------------------------------------------*/
/* -------------------------------- FUNÇÕES PARA CORRIDA NAS MODALIDADES (U) E (v) --------------------------------*/

/* Define se um ciclista "quebrou" com a chance de 1% */
int defineQuebra()
{
    int p;

    /* Gera um número entre 0 e 99 */
    p = (rand() % 100);
    if(p == 0) return TRUE;
    else return FALSE;
}

/* Retorna aleatoriamente 25 ou 50 para ser a velocidade do ciclista */
int velocidadeAleatoria()
{
    int p;

    p = (rand() % 2);
    if(p == 0) return 50;
    else return 25;
}


void realizaCorrida(int vel)
{
    int i;

    threads = mallocSafe(N * sizeof(pthread_t));
    for(i = 0; i < N; i++)
    {
        sem_wait(&mutex);
        threads[i] = ciclistas[i].thread;
        defineVelocidade(i, vel);
        defineVolta(i, 0); /* Volta inicial */
        if(pthread_create(&threads[i], NULL, corrida, &i))
        {
            printf("Erro ao criar a thread %d\n", i);
            exit(EXIT_FAILURE);
        }
        sem_post(&mutex);
    }
    imprimeRelatorioFinal();
}

void *corrida(void *i)
{
    int id = *(int *) i;

    passouFinal(id);
    return NULL;
}

/* Faz o ciclista andar e verifica se algum participante será eliminado da competição */
void passouFinal(int id)
{
    int i, nC = N;
    while(ciclistas[id].velocidade != 0)
    {
        passouParcial(id);
        /* A cada quatro voltas há 1% de chance de algum ciclista quebrar */
        sem_wait(&global);

        if(nVoltas > 0)
        {
            if(nC > 3 && nVoltas % 4 == 0 && defineQuebra() == TRUE)
            {
                do
                {
                    i = rand() % nC;
                }
                while(ciclistas[i].velocidade == 0);   /* Procura um ciclista aleatório que ainda está na corrida para ser eliminado */
                destroiCiclista(i, TRUE);
                nC--;
            }
            /* A cada duas voltas o ciclista na última colocação é eliminado */
            if(nVoltas % 2 == 0 && elimina == 1)
            {
                for(i = 0; ciclistas[i].colocacao < nC; i++);
                destroiCiclista(i, FALSE);
                imprimeRelatorioParcial();
                nC--;
                elimina = 0;
            }
        }

        sem_post(&global);
    }
}

/* Faz o ciclista avançar uma posição e verifica se ele passou pela linha de chegada */
void passouParcial(int id)
{
    if(id == pista[0].cic1 || id == pista[0].cic2 || id == pista[0].cic3 || id == pista[0].cic4)
    {
        ciclistas[id].volta++;

        if(uniforme != TRUE && nVoltas != 0) ciclistas[id].velocidade = velocidadeAleatoria();

        ciclistas[id].colocacao = calculaColocacao(id); /** Como passar o vetor de ciclistas??*/

        if(ciclistas[id].colocacao == 1) /* Se estiver em primeiro lugar, nVoltas é incrementado */
        {
            sem_wait(&global2);

            nVoltas++;
            if(nVoltas % 2 == 0) elimina = 1;

            sem_post(&global2);
        }

    }
    avancaCiclista(id);
}

/* Faz com que um determinado ciclista avance uma posição na pista */
void avancaCiclista(int id)
{
    int i, j, avanca = FALSE;

    sem_wait(&control_pista);

    localizaCiclista(id, &i, &j);

    /* Certifica-se de que a pista é cíclica*/
    if(i == 0) i = D;
    /* Verifica se há como avançar para a próxima posição */
    if(pista[i-1].cic1 == VAZIO)
    {
        pista[i-1].cic1 = id;
        avanca = TRUE;
    }
    else if(pista[i-1].cic2 == VAZIO)
    {
        pista[i-1].cic2 = id;
        avanca = TRUE;
    }
    else if(pista[i-1].cic3 == VAZIO)
    {
        pista[i-1].cic3 = id;
        avanca = TRUE;
    }
    else if(pista[i-1].cic4 == VAZIO)
    {
        pista[i-1].cic4 = id;
        avanca = TRUE;
    }
    if(avanca == TRUE)
    {
        if(j == 1)
        {
            pista[i % D].cic1 = VAZIO;
        }
        else if(j == 2)
        {
            pista[i % D].cic2 = VAZIO;
        }
        else if(j == 3)
        {
            pista[i % D].cic3 = VAZIO;
        }
        else if(j == 4)
        {
            pista[i % D].cic4 = VAZIO;
        }
    }

    sem_post(&control_pista);
}

/* Um ciclista que foi desclassificado receberá velocidade igual a zero para diferenciá-lo dos demais no vetor de ciclistas */
void anulaCiclista(int id)
{
    ciclistas[id].velocidade = 0;
}

/* Define a volta em que um determinado ciclista se encontra */
void defineVolta(int id, int n)
{
    ciclistas[id].volta = n;
}

/* Calcula a colocação de cada ciclista na corrida baseado na volta e na posição em que se encontram na pista */
int calculaColocacao(int id)
{
    int i, j, k, l, cont;

    for(i = 0, cont = 1; i < N; i++)
    {
        if(ciclistas[i].volta > ciclistas[id].volta) cont++;
        else if(ciclistas[i].volta == ciclistas[id].volta)
        {
           localizaCiclista(i, &j, &k);
           localizaCiclista(id, &l, &k);
           if(j < l) cont++;
        }
    }
    return cont;
}

/* --------------------------------------------------------------------------------------------------------*/
/* ------------------------------ FUNÇÕES PARA EXIBIÇÃO DE RESULTADOS -------------------------------------*/

/* i representa o id do primeiro colocado, j do segundo e k do terceiro */
void calculaPrimeirasColocacoes(int *i, int *j, int *k)
{
    int u;

    for(u = 0; u < N; u++)
    {
        if(ciclistas[u].colocacao == 1) *i = u;
        else if(ciclistas[u].colocacao == 2) *j = u;
        else if(ciclistas[u].colocacao == 3) *k = u;
    }
}

/* i representa o id do último colocado, j do penúltimo e k do antepenúltimo */
void calculaUltimasColocacoes(int *i, int *j, int *k)
{
    int u;

    for(u = 0; u < N; u++)
    {
        if(ciclistas[u].colocacao == N) *i = u;
        else if(ciclistas[u].colocacao == N-1) *j = u;
        else if(ciclistas[u].colocacao == N-2) *k = u;
    }
}

/* A cada volta completada um relatório é impresso quem foi eliminado e os últimos colocados */
void imprimeRelatorioParcial()
{
    int i = 0, j = 0, k = 0;

    calculaUltimasColocacoes(&i, &j, &k);
    printf("Volta número %d:\n\tEliminado (último colocado): id = %d\n\tPenúltimo colocado: id = %d\n"
           "\tAntepenúltimo colocado: id = %d\n", nVoltas, ciclistas[i].id, ciclistas[j].id, ciclistas[k].id);
}

/* Imprime o relatório final, que informa a colocação final de todos os ciclistas e quais destes ganharam medalhas */
void imprimeRelatorioFinal()
{
    int i = 0, j = 0, k = 0;

    calculaPrimeirasColocacoes(&i, &j, &k);
    printf("Resultado final %d:\n\tMedalha de ouro: id = %d\n\tMedalha de prata: id = %d\n"
           "\tMedalha de bronze: id = %d\n", nVoltas, ciclistas[i].id, ciclistas[j].id, ciclistas[k].id);
}


/* Devolve a localização de um ciclista na pista */
void localizaCiclista(int id, int *i, int *j)
{
    int k;

    for(k = 0; k < D; k++)
    {
        if(pista[k].cic1 == id)
        {
            *i = k;
            *j = 1;
            break;
        }
        else if(pista[k].cic2 == id)
        {
            *i = k;
            *j = 2;
            break;
        }
        else if(pista[k].cic3 == id)
        {
            *i = k;
            *j = 3;
            break;
        }
        else if(pista[k].cic4 == id)
        {
            *i = k;
            *j = 4;
            break;
        }
    }
}

/* ----------------------------------------------------------------------------------------------------------------*/
/* --------------------------------------- ELIMINAÇÃO DA THREADS --------------------------------------------------*/

/* Destrói a thread correspondente a um determinado ciclista */
void destroiCiclista(int id, int quebrou)
{
    sem_wait(&control);
    if(pthread_join(ciclistas[id].thread, NULL))
    {
        printf("Erro na destruição da thread para o ciclista %d!\n", id);
        exit(EXIT_FAILURE);
    }
    else if(quebrou == TRUE)
    {
        printf("Thread para o ciclista de id %d foi destruída na sua volta de número %d, na posição %d.\n", id,
         ciclistas[id].volta, ciclistas[id].colocacao);
    }
    anulaCiclista(id);
    sem_post(&control);
}
