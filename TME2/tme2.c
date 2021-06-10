/**
 * 
 * 
 * 
 * Producers-Consumers Modelisation by Fair-Threads API.
 * 
 * Amine Benslimane,
 * Master 1 STL,
 * Sorbonne-Université.
 * 
 * May 2021
 * 
*/



#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "ft_v1.0/src/fthread.h"


// *****************   Configuration *******************

#define NBPROD 3
#define NBCONS 2
#define MSG 3
#define TARGET_PRODUCTION 3
#define TAILLE_MAX 5
#define ISCONS 0
#define ISPROD 1



// ******************  DATA STRUCTURE **************************


int cp = NBPROD * TARGET_PRODUCTION;



typedef struct paquet paquet;
struct paquet
{
    char produit[20];
};


typedef struct Tapis Tapis;
struct Tapis
{
    paquet file[TAILLE_MAX];
    int cap;
    int nbrElement;
};

typedef struct _Prod
{
    int cibleProd;
    char nom[100];
    Tapis *tapis;
    FILE* prodLog;
} Prod;

typedef struct Cons
{
    int id;
    Tapis *tapis;
    FILE *ConsoLog;
} Cons;

typedef struct Msg
{
    ft_scheduler_t consSched;
    ft_scheduler_t prodSched;
    Tapis *tapisCons;
    Tapis *tapisProd;
    int id;
    FILE* msgLog;
} Msg;


ft_event_t prodNotEmpty;
ft_event_t prodNotFull;

ft_event_t consNotEmpty;
ft_event_t consNotFull;
ft_event_t consFinished;

static int id = 0;



// ************************    FUNCTIONS *******************************

bool decrementer(int *cpt)
{
    if ((*cpt > 0) && (*cpt)-- )
        return true;
    else
        return false;
}



void enfiler(Tapis *carpet, paquet packet, int carpetType)
{

    while (carpet->cap == carpet->nbrElement)
    {
        if(carpetType == ISCONS)
            ft_await(consNotFull);
        if(carpetType == ISPROD)
            ft_await(prodNotFull);
    }
    if(carpet->nbrElement >= carpet->cap)
        perror("the carpet's size is not sufficient! can't enqueue more ! ");

    //enqueue the packet & send signal !
    carpet -> file[carpet -> nbrElement] = packet;
    carpet->nbrElement++;

    if(carpetType== ISCONS)
        ft_broadcast(consNotEmpty);
    if(carpetType == ISPROD)
        ft_broadcast(prodNotEmpty);
    
}




paquet defiler(Tapis *tapis, int carpetType)
{
    int i=0;

    //if the carpet is empty, wait
    while (tapis->nbrElement == 0)
    {   
        if(carpetType == ISCONS)
            ft_await(consNotEmpty);
        if(carpetType == ISPROD)
            ft_await(prodNotEmpty);
    }
    //Unqueue the first element of the carpet
    paquet packet = tapis->file[0];
    Tapis * t;
    t -> cap = tapis -> cap;

    for( i = 0 ; i < (tapis -> nbrElement) - 1 ; i++ )
    t -> file[i] = tapis -> file[i+1];

    tapis->nbrElement--;
    tapis = t;

    //the carpet can welcome objects, send signal !
    if(carpetType == ISCONS)
    ft_broadcast(consNotFull);

    if(carpetType == ISPROD)
    ft_broadcast(prodNotFull);

    return packet;
}



void *consommateur(void *args)
{
    Cons *consArg = (Cons *) args;
    Tapis *tapis = consArg->tapis;
    FILE* consLog = consArg ->ConsoLog;
    id++;

    while (decrementer(&cp))
    {   
        paquet p =defiler(tapis, ISCONS);
        printf("C%d mange %s\n", id, p.produit);
        fputs(p.produit, consLog);
        ft_cooperate();
    }
    ft_broadcast(consFinished);
}



void *producteur(void *args)
{
    Prod *prodArg = (Prod *)args;
    Tapis *tapis = &prodArg->tapis;
    FILE* prodLog = prodArg->prodLog; 

    int cibleProd = prodArg->cibleProd;
    char name[100];
    strcpy(name, prodArg->nom);
    int cp = 1;

    while (cibleProd > 0)
    {
        char newName[100];
        char buffer[100];
        strcpy(newName, name);
        strcat(newName, " ");
        sprintf(buffer, "%d", cp);
        strcat(newName, buffer);
        paquet* packet = (paquet *) malloc(sizeof(paquet));
        strcpy(packet -> produit,newName);

        enfiler(tapis, *packet, ISPROD);
        printf("Producteur: %s\n", newName);

        cibleProd--;
        cp++;
        fputs(newName, prodLog);
        ft_cooperate();
    }

}



void* messager(void *args){
    Msg* msgArg = (Msg *) args;
    ft_scheduler_t prodSched = msgArg ->prodSched;
    ft_scheduler_t consSched = msgArg ->consSched;
    Tapis  *tcons = msgArg ->tapisCons;
    Tapis  *tprod = msgArg ->tapisProd;
    FILE* mlog = msgArg ->msgLog;

    while(cp > 0){
        ft_thread_link(prodSched);
        paquet packet = defiler(tprod,ISPROD);
        ft_thread_unlink();
        fputs(packet.produit,mlog);
        ft_thread_link(consSched);
        enfiler(tprod, packet, ISCONS);
        ft_thread_unlink();
    }

}


// **************   Main Program *************************

int main()
{

    // Initialisation of the configuration ***********************

    Tapis carpetProd;
    Tapis carpetCons;
    carpetCons.cap = TAILLE_MAX;       
    carpetCons.nbrElement = 0;
    carpetProd.cap = TAILLE_MAX;       
    carpetProd.nbrElement = 0;

    ft_scheduler_t cpSched = ft_scheduler_create();
    ft_scheduler_t consSched = ft_scheduler_create();
    ft_scheduler_t prodSched = ft_scheduler_create();

    consFinished = ft_event_create(cpSched);
    consNotEmpty = ft_event_create(consSched);
    consNotFull = ft_event_create(consSched);
    prodNotEmpty = ft_event_create(prodSched);
    prodNotFull = ft_event_create(prodSched);

    FILE* consLog = fopen("consLog.txt","a+");
    FILE* prodLog = fopen("prodLog.txt","a+");
    FILE* msgLog = fopen("msgLog.txt","a+");

    Prod prodArg;
    prodArg.tapis = &carpetProd;
    prodArg.cibleProd = 3;
    strcpy(prodArg.nom, "pomme");
    prodArg.prodLog = prodLog;

    Cons consArg;
    consArg.tapis = &carpetCons;
    consArg.ConsoLog = consLog;

    Msg msgArg;
    msgArg.tapisProd = &carpetProd;
    msgArg.tapisCons = &carpetCons;
    msgArg.consSched = consSched;
    msgArg.prodSched = prodSched;
    msgArg.msgLog = msgLog;

    int i;

    // launching threads
    for (i = 0; i < NBPROD; i++)
    {
        strcpy(prodArg.nom, "pomme");
        ft_thread_create(prodSched,producteur, NULL,&prodArg);
    }

    for (i = 0; i < NBCONS; i++)
    {
        consArg.id = i;
        ft_thread_create(consSched,consommateur, NULL,&consArg);
    }

    for (i = 0; i < MSG; i++)
    {
        ft_thread_create(NULL,messager, NULL,&msgArg);
    }

    while(cp>0){      
        ft_await(consFinished);
    }

    printf("END\n");
    
}
