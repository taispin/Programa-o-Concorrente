/* ***************************************** */
/*										     */
/* EP2 - MAC0438 - Programação Concorrente   */
/*											 */
/* Bárbara de Castro Fernandes - 7577351	 */
/* Taís Aparecida Pereira Pinheiro - 7580421 */
/*											 */
/* ***************************************** */
/* Macros */
#define TRUE 0
#define FALSE 1

/* Funções auxiliares e de inicialização */
int calculaNucleos();
void retornaDiferenca(int p);
void *mallocSafe(size_t n);
void imprimeResultado(int n);
void inicializaControladores(int n);
void globalizaParametros(int n, mpf_t x, char *modo, char *opcional);

/* Funções para cálculo de um termo */
void potencia(mpf_t pot, mpf_t b, int e);
void fatorial(mpf_t res, int n);
int partA(int n);
void partB(mpf_t b, int n);
void partC(mpf_t c, mpf_t x, int n);
void calculaTermoCosseno(mpf_t termo, mpf_t x, int n);

/* Funções para cálculo de cos(x) concorrente */
void cossenoConcorrente();
void *calculaCossenoConc(void *param);
void incrementaCosseno (mpf_t termo, char c);
void calculaTermoCossenoConc(int id, int n);
void condicaoDeParadaConc(mpf_t termo);

/* Funções para cálculo de cos(x) sequencial */
void cossenoSequencial();
int condicaoDeParada(mpf_t penultimo, mpf_t ultimo);
