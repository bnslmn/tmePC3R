/****
 *  to compile :     gcc -o tme1 tme1.c -lpthread
 *  to execute :     ./tme1
 * 
 * @author Amine Benslimane
 * amine.benslimane@etu.sorbonne-universite.fr
 * https://github/bnslmn
 * 
 * Producer-Consumer Model, Synchronization problem
 * 
 * Feel free to use this code, program under GPLv3
 * 
 * Sorbonne Université
 * March 2021,
 * 
*/


#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>

//separating helper functions
#include "helper.h"


//**********************  the main configuration *******************

static char * products[4]={"Pomme", "Datte", "Prune", "Orange"};
static int PROD_THREAD= sizeof(products)/sizeof(char*);
static int CONS_THREAD=3;
static int CAPMAX_CARPET = 20;
static int TARGET_PROD = 3;

// **********************  definition of the data structure *****************

// record paquet
typedef struct{
	char * nom;
}paquet;

// record carpet
typedef struct{
	paquet ** p;
	size_t p_size;
	size_t first;
	size_t size;
	pthread_cond_t  full;
	pthread_cond_t  empty ;
	pthread_mutex_t mutex;
	
}carpet;

//record thread producer	
typedef struct{
	char * nameProd;
	int TARGET_PROD;
	int thisProd;
	carpet * carpet;
}thread_prod;

//record thread consumer 
typedef struct{
	int id;
	int * cp;
	carpet * carpet;
}thread_cons;


// *******************   manipulating the data structure *********************

// get a new pack
void new_pack(paquet* p, char * tab){
	char * t = malloc((strlen(tab)+1)*sizeof(char));
	for(size_t i=0; i <=strlen(tab); i++)
		t[i]=tab[i];
	p->nom = t;

}

//free the pack
void free_pack(paquet *p){
	if (p !=NULL){
		free(p->nom); //p->nom = 0x0;
		free(p); //p = 0x0;
	}
}

// initialise a carpet
void new_carpet(size_t maxsize, carpet * t){

	t->p_size = maxsize;
	t->first  = 0;
	t->size   = 0;
	t->p      = malloc(maxsize*sizeof(paquet));

	//we could use perror
	if (pthread_mutex_init(&t->mutex, NULL) != 0 || pthread_cond_init(&t->empty, NULL) != 0 || pthread_cond_init(&t->full, NULL) != 0)
	printf("\n MUTEX FAILED TO INITIALIZE\n");
}


// remove a pack
paquet * defiler(carpet * t, int * cp){

	//manipulating the carpet --> critical section access
	pthread_mutex_lock(&t->mutex);

	//while carpet is full, waiting !
	while(t->size==0)
	pthread_cond_wait(&t->empty , &t->mutex);

	if(t->size ==t->p_size) 
	pthread_cond_signal(&t->full);

	paquet * p = t->p[t->first];
	t->size--;
	t->first=(t->first+1)%t->p_size;
	--(*cp);
	pthread_mutex_unlock(&t->mutex);

	return p;
}

//put on a pack
void enfiler(carpet *t, paquet * p){
	//manipulating the carpet --> critical section access
	pthread_mutex_lock(&t->mutex);

	//carpet full, producers are waiting
	while(t->size ==t->p_size) 
	pthread_cond_wait(&t->full , &t->mutex);

	//carpet empty, alert all
	if(t->size==0)
	pthread_cond_signal(&t->empty);

	//adding the packet (classical pattern)
	t->p[(t->first+t->size) % t->p_size] = p;
	t->size++;
	pthread_mutex_unlock(&t->mutex);
}

//creating a new pack && putting it on a carpet
void* produire(void * args){
	thread_prod * prod = args;

	while (prod->TARGET_PROD != prod->thisProd){
		paquet * p = malloc(sizeof(paquet));
		new_pack(p, concat(prod->nameProd,toString(prod->thisProd)));
		enfiler(prod->carpet, p);
		prod->thisProd++;
	}
	free(prod);
	return 0;
}

//consume a pack and remove it from the carpet
void* consommer(void * args){
	thread_cons * cons = (thread_cons*) args;

	while((*(cons->cp))>0){
		paquet * p =defiler(cons->carpet,cons->cp);
		printf("C%d mange %s\n", cons->id, p->nom);
		free_pack(p);
	}
	free(cons);
	return 0;
}

//**** main program

void main(void) {

	/*** initialisation ***/

	pthread_t th_prod[PROD_THREAD];
	pthread_t th_cons[CONS_THREAD];

	//threads running
	static int beginProd =0;
	static int beginCons =0;
	//threads ended
	static int endProd = 0;
	static int endCons = 0;

	carpet carpet;
	new_carpet(CAPMAX_CARPET , &carpet);
	int compteur = TARGET_PROD*PROD_THREAD;

	//to protect compteur
	pthread_mutex_t  comptLock;

	/*** creating producers ***/

	while(beginProd < PROD_THREAD){
		thread_prod *prodThread = malloc(sizeof (thread_prod));
		prodThread->nameProd=products[beginProd];
		prodThread->thisProd=0;
		prodThread->TARGET_PROD= TARGET_PROD;
		prodThread->carpet= &carpet;
		pthread_create(	& ( th_prod[beginProd] ), NULL, produire, prodThread );

		beginProd++;
	}

	/*** creating consumers ***/

	while(beginCons < CONS_THREAD){
		thread_cons * consThread = malloc(sizeof (thread_cons));
		consThread->cp=&compteur;
		consThread->id=beginCons;
		consThread->carpet=&carpet;
		pthread_create(&(th_cons[beginCons]), NULL, consommer, consThread);
		beginCons++;
	}

	/*** shutting down producers **/
	while(endProd < PROD_THREAD){
		pthread_join(th_prod[endProd],NULL);
		endProd++;
	}

	/*** shutting down consumers ***/
	while(endCons < CONS_THREAD){
		pthread_join(th_cons[endCons],NULL);
		endCons++;
	}
	sleep(1);

	printf("***  %d/%d producers & %d/%d consumers ended  ***\n",endProd,PROD_THREAD,endCons,CONS_THREAD);

	/*** free memory & destruction ***/
	free(carpet.p); //or carpet.p=0x0;

	pthread_cond_destroy(&carpet.full);
	pthread_cond_destroy(&carpet.empty);
	pthread_mutex_destroy(&carpet.mutex);

}