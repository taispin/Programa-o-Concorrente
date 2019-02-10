/* ***************************************** */
/*										     */
/* EP1 - MAC0438 - Programação Concorrente   */
/*											 */
/* Bárbara de Castro Fernandes - 7577351	 */
/* Taís Aparecida Pereira Pinheiro - 7580421 */
/*											 */
/* ***************************************** */

#define TRUE 0
#define FALSE 1
#define VAZIO -1

typedef struct ciclista
{
	pthread_t thread;
	int velocidade;
	int id;
	int volta;
	int colocacao;
} Ciclista;

typedef struct lugar
{
	int cic1;
	int cic2;
	int cic3;
	int cic4;
} Lugar;

void globalizaVariaveis(int n, int d);
void *mallocSafe(size_t n);
void inicializaSemaforos();
void inicializaPista();
void definePosicoesIniciais();
void defineId();
void defineVelocidade(int id, float vel);
int defineQuebra();
int velocidadeAleatoria();
void realizaCorrida(int vel);
void *corrida(void *i);
void passouFinal(int id);
void passouParcial(int id);
void avancaCiclista(int id);
void anulaCiclista(int id);
void defineVolta(int id, int n);
int calculaColocacao(int id);
void calculaPrimeirasColocacoes(int *i, int *j, int *k);
void calculaUltimasColocacoes(int *i, int *j, int *k);
void imprimeRelatorioParcial();
void imprimeRelatorioFinal();
void localizaCiclista(int id, int *i, int *j);
void destroiCiclista(int id, int quebrou);
