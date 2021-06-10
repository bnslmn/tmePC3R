#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sched.h>
#include <stdbool.h>
#include <unistd.h>
#include "ft_v1.0/src/fthread.h"

#define NBPROD 2
#define NBCONS 2
#define MSG 2
#define TARGET_PRODUCTION 3
#define TAILLE_MAX 5
#define ISCONS 0
#define ISPROD 1



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

typedef struct _ProdArgs
{
    int cibleProd;
    char nom[100];
    Tapis *tapis;
    FILE* prodLog;
} ProdArgs;

typedef struct ConsoArgs
{
    int id;
    Tapis *tapis;
    FILE *ConsoLog;
} ConsoArgs;

typedef struct MessArgs
{
    ft_scheduler_t tcons_sched;
    ft_scheduler_t tprod_sched;
    Tapis *tapisC;
    Tapis *tapisP;
    int id;
    FILE* MessLog;
} MessArgs;


ft_event_t prodNotEmpty;
ft_event_t prodNotFull;

ft_event_t consNotEmpty;
ft_event_t consNotFull;
ft_event_t consFinished;

static int id = 0;



bool decremente(int *cpt)
{
    if (*cpt > 0)
    {
        (*cpt)--;
        return true;
    }
    else
    {
        return false;
    }
}



void enfiler(Tapis *t, paquet p, int tapis_type)
{

    //si full on attend
    while (t->cap == t->nbrElement)
    {
        if(tapis_type == ISCONS)
            ft_await(consNotFull);
        if(tapis_type == ISPROD)
            ft_await(prodNotFull);
    }
    if(t->nbrElement + 1 > t->cap)
        perror("tapis overflow in empiler");
    //on place le paquet en fin de file
    t -> file[t -> nbrElement] = p;
    t->nbrElement++;
    //on previent que le tapis n'est plus vide
    if(tapis_type== ISCONS)
        ft_broadcast(consNotEmpty);
    if(tapis_type == ISPROD)
        ft_broadcast(prodNotEmpty);
    
}




paquet defiler(Tapis *t, int tapis_type)
{
    //si vide on attend
    while (t->nbrElement == 0)
    {   
        if(tapis_type == ISCONS)
            ft_await(consNotEmpty);
        if(tapis_type == ISPROD)
            ft_await(prodNotEmpty);
    }
    //recuperation du premier element (headpop)
    paquet p = t->file[0];
    //copie et decalage du tapis
    Tapis * tmp;
    tmp -> cap = t -> cap;
    int i;
    for( i = 0 ; i < (t -> nbrElement) - 1 ; i++ ){  //TODO a voir s'il n'y a qu'un elem
        tmp -> file[i] = t -> file[i+1];
    }
    t = tmp;
    t->nbrElement--;
    //on previent que le tapis n'est plus plein
    if(tapis_type == ISCONS)
        ft_broadcast(consNotFull);
    if(tapis_type == ISPROD)
        ft_broadcast(prodNotFull);
    return p;
}

int cp = NBPROD * TARGET_PRODUCTION;

//==========Routines=============

void *consommateur(void *args)
{
    printf("Debut consommateur\n");
    ConsoArgs *arg = (ConsoArgs *)args;
    Tapis *tapis = arg->tapis;
    FILE* clog = arg ->ConsoLog;
    id++;

    while (decremente(&cp))
    {   
        paquet p =defiler(tapis, ISCONS);
        printf("C%d mange %s\n", id, p.produit);
        fputs(p.produit, clog);
        ft_cooperate();
    }
    ft_broadcast(consFinished);
    printf("Fin consommateur\n");
}



void *producteur(void *args)
{
    printf("Debut producteur\n");
    ProdArgs *arg = (ProdArgs *)args;
    int cibleProd = arg->cibleProd;
    char name[100];
    strcpy(name, arg->nom);
    Tapis *tapis = &arg->tapis;
    int cpt = 1;
    FILE* plog = arg->prodLog; 

    while (cibleProd != 0)
    {
        char newname[100];
        char buff[100];
        strcpy(newname, name);
        strcat(newname, " ");
        sprintf(buff, "%d", cpt);
        strcat(newname, buff);
        paquet* p = (paquet *) malloc(sizeof(paquet));
        strcpy(p -> produit,newname);

        enfiler(tapis, *p, ISPROD);
        printf("Producteur: %s\n", newname);

        cibleProd--;
        cpt++;
        fputs(newname, plog);
        ft_cooperate();
    }

    printf("Fin producteur\n");
}



void* messager(void *args){
    MessArgs* arg = (MessArgs *) args;
    ft_scheduler_t psched = arg ->tprod_sched;
    ft_scheduler_t csched = arg ->tcons_sched;
    Tapis  *tcons = arg ->tapisC;
    Tapis  *tprod = arg ->tapisP;
    FILE* mlog = arg ->MessLog;

    while(cp > 0){
        ft_thread_link(psched);
        paquet p = defiler(tprod,ISPROD);
        ft_thread_unlink();
        fputs(p.produit,mlog);
        ft_thread_link(csched);
        enfiler(tprod, p, ISCONS);
        ft_thread_unlink();
    }

}


//=============Main================

int main()
{

    // Les tapis initialisés
    Tapis tapis_prod;
    Tapis tapis_cons;
    tapis_cons.cap = TAILLE_MAX;       
    tapis_cons.nbrElement = 0;
    tapis_prod.cap = TAILLE_MAX;       
    tapis_prod.nbrElement = 0;

    //schedulers
    ft_scheduler_t cp_sched = ft_scheduler_create();
    ft_scheduler_t tcons_sched = ft_scheduler_create();
    ft_scheduler_t tprod_sched = ft_scheduler_create();

    //events
    consFinished = ft_event_create(cp_sched);
    consNotEmpty = ft_event_create(tcons_sched);
    consNotFull = ft_event_create(tcons_sched);
    prodNotEmpty = ft_event_create(tprod_sched);
    prodNotFull = ft_event_create(tprod_sched);

    //log files
    FILE* clog = fopen("ConsoLog.txt","a+");
    FILE* plog = fopen("ProdLog.txt","a+");
    FILE* mlog = fopen("MessLog.txt","a+");

    //routine args init
    ProdArgs pargs;
    pargs.tapis = &tapis_prod;
    pargs.cibleProd = 3;
    strcpy(pargs.nom, "pomme");
    pargs.prodLog = plog;

    ConsoArgs cargs;
    cargs.tapis = &tapis_cons;
    cargs.ConsoLog = clog;

    MessArgs margs;
    margs.tapisC = &tapis_cons;
    margs.tapisP = &tapis_prod;
    margs.tcons_sched = tcons_sched;
    margs.tprod_sched = tprod_sched;
    margs.MessLog = mlog;

    //Lancement thread producteurs

    int i;
    for (i = 0; i < NBPROD; i++)
    {
        strcpy(pargs.nom, "pomme");
        ft_thread_create(tprod_sched,producteur, NULL,&pargs);
    }

    for (i = 0; i < NBCONS; i++)
    {
        cargs.id = i;
        ft_thread_create(tcons_sched,consommateur, NULL,&cargs);
    }

    for (i = 0; i < MSG; i++)
    {
        ft_thread_create(NULL,messager, NULL,&margs);
    }

    while(compteur>0){      
        ft_await(consFinished);
    }

    printf("END\n");
    
}

