#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "pthread.h"
#include "semaphore.h"
#include <random>
#include <iostream>
#define TRUE 1
#define Q_CADEIRAS 5                /*quantidade cadeiras para os clientes aguardando atendimento*/

std::random_device rd;
std::mt19937 gen(rd()); 
std::uniform_int_distribution<> distr(2, 3); 

sem_t qBarbeiros;               /* quantidade de barbeiros aguardando clientes */
sem_t qClientes;                /* quantidade de clientes aguardando atendimento */
sem_t mutex;                  
int clienteAguardando = 0;      /* clientes aguardando atendimento */

float meuDia = 144; 			//1 HORA = 6 SEGUNDOS, ENTAO 24 HORAS * 6 SEGUNDOS = 144 SEGUNDOS, 144 SEGUNDOS = 1 DIA
int clienteHora = 0;
int clienteTempo = 6;
int barbeiroDormindo = 0;
int contClientesAtendidos = 0;
 
void* fBarbeiro(void *arg);
void fBarbeiroDormindo();
void fCorteCabelo();
void* fCliente(void *arg);
void fClienteChegou();
void fSendoAtendido();
void fDesiste();

int main() {
	sem_init(&qClientes, TRUE, 0);
	sem_init(&qBarbeiros, TRUE, 0);
	sem_init(&mutex, TRUE, 1);

	pthread_t b, c;

	pthread_create(&b, NULL, fBarbeiro, NULL);

	while(meuDia >= 0) { 
		
		printf("Tempo (horas) restante: %f \n", meuDia/6);
		fClienteChegou();
		pthread_create(&c, NULL, fCliente, NULL);
		sleep(3); //dois clientes chegam em uma hora (6 seg), em 3 seg um cliente chega.
		meuDia -= 3;
	}

	printf("O barbeiro atendeu %d clientes \n", contClientesAtendidos);
	printf("O barbeiro conseguiu dormir por %d minutos\n", barbeiroDormindo*10);
	return 0;
}

void* fBarbeiro(void *arg) {
	while(TRUE) {
		sem_wait(&qClientes);  /*barbeiro dorme se qCliente = 0*/
		sem_wait(&mutex);       /* acessa 'clienteAguardando' */
		clienteAguardando = clienteAguardando - 1;  /*descresce de um o contador de clientes aguardando */
		sem_post(&qBarbeiros);     /* barbeiro pronto para cortar cabelo */
		sem_post(&mutex);       /* libera 'clienteAguardando' */
		fCorteCabelo();             /* corta o cabelo */
		if(clienteAguardando == 0){
			fBarbeiroDormindo();
		}
	}
	pthread_exit(NULL);
}

void* fCliente(void *arg) {
	sem_wait(&mutex);         

	if(clienteAguardando < Q_CADEIRAS) {      
		clienteAguardando = clienteAguardando + 1;  /* incrementa o contador de clientes aguardando */
		sem_post(&qClientes);   /* acorda o barbeiro se precisar */
		sem_post(&mutex);       /* acessa 'clienteAguardando' */
		sem_wait(&qBarbeiros);     /* dorme se o número de barbeiros livres for 0 */
		fSendoAtendido();          /* sentado e sendo atendido */
	} else {
		sem_post(&mutex);       /* cliente desiste e  vai embora*/
		fDesiste();
	}
	pthread_exit(NULL);
}

void fCorteCabelo() {
	float tempoCorte = distr(gen);
	printf("Barbeiro trabalhando...\n");
	meuDia = meuDia - tempoCorte;
	contClientesAtendidos++;
	printf("Tempo (horas) restante: %f \n", meuDia/6);
	sleep(tempoCorte);	
}

void fClienteChegou() {
	printf("Cliente chegou na barbearia!\n");
}
void fSendoAtendido() {
	printf("Cliente foi atendido!\n");
}

void fDesiste() {
	printf("Cliente deistiu e foi embora!\n");
}

void fBarbeiroDormindo(){
	printf("Barbeiro dormindo...!\n");
	meuDia--; 
	barbeiroDormindo++;
	printf("Tempo (horas) restante: %f \n", meuDia/6);
}
