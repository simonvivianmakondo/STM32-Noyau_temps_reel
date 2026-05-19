 /*----------------------------------------------------------------------------*
 * fichier : noyau.c                                                          *
 * mini noyau temps reel fonctionnant sur MC9328MXL                           *
 * ce fichier definit toutes les fonctions du noyau                           *
 *----------------------------------------------------------------------------*/

#include <stdint.h>

#include "../hwsupport/stm32h7xx.h"
#include "../io/serialio.h"
#include "noyau.h"
#include "noyau_file.h"

#define ALIGN(x, a) (((x) + ((a)-1)) & ~((a)-1))

/*--------------------------------------------------------------------------*
 *           Constantes pour la création du contexte CPU initial            *
 *--------------------------------------------------------------------------*/
#define THUMB_ADDRESS_MASK (0xfffffffe) /* Masque pointeurs de code Thumb   */
#define TASK_EXC_RETURN (0xfffffff9)    /* Valeur de retour d'exception :   */
                                        /* msp active, mode Thread          */
#define TASK_PSR (0x01000000)           /* PSR initial                      */

/*--------------------------------------------------------------------------*
 *            Variables internes du noyau                                   *
 *--------------------------------------------------------------------------*/
static int compteurs[MAX_TACHES]; /* Compteurs d'activations                */
NOYAU_TCB  _noyau_tcb[MAX_TACHES]; /* tableau des contextes                 */
volatile uint16_t _tache_c;     /* numéro de tache courante                 */
uint32_t _tos;                  /* adresse du sommet de pile des tâches     */

/*--------------------------------------------------------------------------*
 * fonctions du noyau                                                       *
 *--------------------------------------------------------------------------*/
uint16_t get_tache_courante(void)
{
    return _tache_c;
}

NOYAU_TCB *get_tcb(uint8_t t)
{
    return &_noyau_tcb[t];
}
/*
 * termine l'execution du noyau
 * entre  : sans
 * sortie : sans
 * description : termine l'execution du noyau, execute en boucle une
 *               instruction vide
 */
 void noyau_exit(void) {
    /* Q2.1 : desactivation des interruptions */
	 _irq_disable_();
    /* affichage du nombre d'activation de chaque tache !                   */
    printf("Sortie du noyau\n");
    for (int j = 0; j < MAX_TACHES; j++) {
        printf("\nActivations tache %d : %d", j, compteurs[j]);
    }
    /* Q2.2 : Que faire quand on termine l'execution du noyau ? */
    //_irq_enable_();
    for(;;);
}

/*
 * termine l'execution d'une tache
 * entre  : sans
 * sortie : sans
 * description : une tâche dont l'execution se termine n'est plus exécutée
 *               par le noyau
 *               cette fonction doit être appelée à la fin de chaque tâche
 */
 void fin_tache(void) {
    /* Q2.3 :             Début section critique                             */
	 _lock_();
    /* Q2.4 : la tâche est enlevée de la file des tâches et ... ?            */
    /* 4 instructions */
	 _noyau_tcb[_tache_c].status = CREE; /*change l’état d’une tâche active en CREE*/
	 file_retire(_tache_c); /*Elle est retirée de la file des tâches prêtes*/
	 _unlock_();
	 schedule(); /*schedule() est nécessaire ici, sinon aucune nouvelle tâche ne prend le relais*/
	 /*Donc le système doit immédiatement choisir une autre tâche à exécuter.*/
	 //for(;;); //Empecher le code appelant de s'executer, car fin de tache est vraiment fin de tache
	 //mais inutile car le scheduler, changera deja de tache
 }

/*
 * demarrage du system en mode multitache
 * entre  : adresse de la tache a lancer
 * sortie : sans
 * description : active la tache et lance le scheduler
 */
 void start(TACHE_ADR adr_tache) {
    uint16_t j;
    register unsigned int sp asm("sp");

    /* Q2.5 : initialisation de l'etat des taches                            */
    for (j = 0; j < MAX_TACHES; j++) {
    	_noyau_tcb[j].status = NCREE;
    }
    /* Q2.6 : initialisation de la tache courante */
    _tache_c = UINT16_MAX;
    /* initialisation de la file circulaire de gestion des tâches           */                  
    file_init();                     
    /* Q2.7 : initialisation de la variable _tos sommet de la pile           */
    /* Haut de la pile des tâches avec réservation  pour le noyau           */
    _tos = (uint32_t)sp - PILE_NOYAU;
    /* Q2.8 : on interdit les interruptions  */
    _irq_disable_();
    /* Q2.9 : initialisation du timer system a 100 Hz   (voir cortex.c)        */
    systick_start(CORE_CLK / 100);
    /* Q2.10 : initialisation de l'interruption systick  (voir cortex.c) */
    systick_irq_enable();
    /* Q2.11 : creation et activation de la premiere tache                          */
    active(cree(adr_tache));
    /* Q2.12 : on autorise les interruptions */
    _irq_enable_();
}

/*
 * creation d'une nouvelle tache
 * entre  : adresse de la tache a creer
 * sortie : numero de la tache cree
 * description : la tache est creee en lui allouant une pile et un numero
 *               en cas d'erreur, le noyau doit etre arrete
 * Err. fatale: priorite erronnee, depassement du nb. maximal de taches 
 */
 uint16_t cree(TACHE_ADR adr_tache){
    /* pointeur d'une case de _contexte         */
    NOYAU_TCB *p;               
    /* Q2.13 : numero de la derniere tache créée */ 
    static uint16_t tache = UINT16_MAX;
    
    /* Q2.14: debut section critique */
    _lock_();
    /* Q2.15 : generation du numero de la tache suivante */
    tache++;
    /* Q2.16 : arret du noyau si plus de MAX_TACHES^*/
    if(tache >= MAX_TACHES)
    	noyau_exit();
    
    /* creation du contexte de la nouvelle tache */
    p = &_noyau_tcb[tache];
    /* Q2.17 : allocation d'une pile a la tache */ 
    p->sp_start = _tos;
    //correspond à la base / sommet mémoire réservé à la pile
    //correspond au stack pointer initial après préparation
    /* Q2.18 : decrementation du pointeur de pile general, afin que la prochaine tache */
	/* n'utilise pas la pile allouee pour la tache courante */
    _tos =  _tos - PILE_TACHE;
    /* Q2.19 : memorisation de l'adresse de debut de la tache */
    p->task_adr = adr_tache;
    /* Q2.20 : mise a jour de l'etat de la tache a CREE */
    p->status = CREE;
    /* Q2.21 : fin section critique */
    _unlock_();

    return (tache); /* tache est un uint16_t */
}

/*
 * ajout d'une tache pouvant etre executee a la liste des taches eligibles
 * entre  : numero de la tache a ajouter
 * sortie : sans
 * description : ajouter la tache dans la file d'attente des taches eligibles
 *               en cas d'erreur, le noyau doit etre arrete
 */
 void active(uint16_t tache){

 	/*
 	 * Le contexte du CPU est l’ensemble des informations internes du processeur à un instant donné.
 	 * Autrement dit, c’est “l’état mental du processeur”.
 	 *
 	 * Dans un système multitâche, le CPU ne peut exécuter qu’une tâche à la fois.
 	 * Donc pour passer d’une tâche à une autre, on doit sauvegarder et restaurer cet état.
 	 *
 	 * Ici, on fabrique artificiellement un contexte CPU INITIAL pour une nouvelle tâche,
 	 * car elle n’a jamais été exécutée et n’a donc aucun état CPU existant.
 	 *
 	 * Sans ce contexte, le scheduler ne pourrait pas la lancer.
 	 * Sans contexte CPU initial, une tâche ne peut pas être démarrée correctement
 	 * par le scheduler dans un système multitâche.
 	 *
 	 * Alors pourquoi on parle de “restaurer” ?
	 * Parce que le CPU, lui, ne sait faire que ça : reprendre un état existant.
	 * Même pour une nouvelle tâche, l’OS est obligé de fabriquer un faux état initial.
	 * Donc on “simule une restauration”.
	 *
 	 */

     NOYAU_TCB *p = &_noyau_tcb[tache]; /* accès au bloc de contrôle de la tâche (état + pile + info exécution) */

     /* Q2.22 : vérifie que la tâche existe bien dans le système */
     if(p->status == NCREE)
         noyau_exit();

     /* Q2.23 : début section critique
      * On empêche les interruptions pour éviter qu’un changement de tâche
      * corrompe la création du contexte en cours */
     _lock_();

     /* Q2.24 : activation de la tâche seulement si elle est à l'état CREE */
     if (p->status == CREE) {
          /* Ici on simule une situation très importante :
          * "la tâche a déjà commencé et a été interrompue"
          *
          * Donc on construit manuellement la pile comme si le CPU avait
          * déjà sauvegardé ses registres.
          *
          * Cette pile va être utilisée plus tard par le scheduler pour
          * restaurer la tâche comme si elle avait toujours existé.
          */

         /* Calcul théorique de la zone où le contexte va être placé dans la pile */
    	 uint32_t contexte = (uint32_t)(p->sp_start - sizeof(CONTEXTE_CPU_BASE));
         contexte = ALIGN(contexte, 8);
         /* On réserve réellement de l’espace sur la pile pour stocker le contexte CPU */
         /* On aligne la pile sur 8 octets (obligatoire sur ARM pour stabilité CPU) */
         p->sp_ini = contexte;
         p->sp = contexte;

         /*
          * On crée un pointeur vers cette zone mémoire :
          * c’est ici que l’on va "fabriquer" les registres CPU
          */
         CONTEXTE_CPU_BASE *c = (CONTEXTE_CPU_BASE*) p->sp_ini;

         /*
          * PC (Program Counter)
          * → point d’entrée de la tâche
          * → première instruction exécutée quand la tâche démarre
          *
          * On masque le bit Thumb car ARM utilise ce bit pour indiquer le mode d’exécution
          */
         c->pc = ((uint32_t)p->task_adr) & THUMB_ADDRESS_MASK;

         /*
          * LR (Link Register)
          * → adresse de retour de la tâche
          *
          * Ici on force le retour vers fin_tache :
          * si la tâche se termine, elle ne plante pas,
          * elle revient dans une fonction du noyau
          */
         c->lr = (uint32_t) fin_tache;

         /*
          * lr_exc (Exception Return)
          * → indique au CPU comment sortir d’un mode exception
          *
          * Ici on simule un retour d’interruption vers une tâche normale
          */
         c->lr_exc = TASK_EXC_RETURN;

         /*
          * PSR (Program Status Register)
          * → état initial du CPU (flags, mode Thumb, etc.)
          */
         c->psr = TASK_PSR;

         /*
          * À ce stade :
          * la tâche possède maintenant un contexte CPU COMPLET,
          * comme si elle avait déjà été exécutée puis interrompue.
          */

         /* La tâche devient prête à être exécutée */
         p->status = PRET;

         /* On la place dans la file des tâches prêtes */
         file_ajoute(tache);

         /*
          * On demande immédiatement au scheduler de choisir une tâche :
          * soit elle-même si elle est prioritaire,
          * soit une autre tâche prête.
          */
         schedule();
     }

     /* Q2.25 : fin section critique
      * On réautorise les interruptions */
     _unlock_();
 }
/*--------------------------------------------------------------------------*
 *                  ORDONNANCEUR preemptif optimise                         *
 * Entrée : pointeur de contexte CPU de la tâche courante                   *
 * Sortie : pointeur de contexte CPU de la nouvelle tâche courante          *
 * Descrip: sauvegarde le pointeur de contexte de la tâche courante,        *
 *      Élit une nouvelle tâche et retourne son pointeur de contexte.       *
 *      Si aucune tâche n'est éligible, termine l'exécution du noyau.       *
 *--------------------------------------------------------------------------*/

 uint32_t task_switch(uint32_t sp)
{
    NOYAU_TCB *p = &_noyau_tcb[_tache_c]; /* acces au contexte tache courante */

    /* Q2.26 : sauvegarde du pointeur sur le contexte sauvegardé sur la pile 
       dans le contexte de la tache */
    printf("task_switch: _tache_c=%d\n", _tache_c);
    if (_tache_c < MAX_TACHES)
    	p->sp = sp; //sert à sauvegarder le contexte de la tâche courante avant de la quitter.
    /* on bascule sur la nouvelle tache a executer */
    /* Q2.27 : recherche la prochaine tache a executer */
    uint16_t next = file_suivant();
    printf("task_switch: next=%d status=%d\n", next, _noyau_tcb[next].status);
    /* Q2.28 : acces contexte suivant                   */
    NOYAU_TCB *next_ctx = &_noyau_tcb[next];
    /* Q2.29 : verifie qu'une tache suivante existe, sinon arret du noyau */
    if(next_ctx->status != PRET && next_ctx->status != EXEC) {
        printf("Il n'y a pas de tâche éligible !\n");
        noyau_exit();
    }
    _tache_c = next;
    NOYAU_TCB *contexte = next_ctx;
    /* incrémentation du compter d'activation de la nouvelle tache */
    compteurs[_tache_c]++;   

    /* Q2.30 : retourner la bonne valeur de pointeur de pile 
     Deux cas possible en fonction du statut de la tâche */

    if(contexte->status == PRET) //Tache n'a jamais été excecutée
    {//Le pointeur de pile sera le pointeur vers le debut du contexte de la tache de la tache courante
    	contexte->status = EXEC;
    	return contexte->sp_ini;
    	//démarre la tâche depuis le début, comme un premier lancement.
    }
    else if(contexte->status == EXEC) //Tache a déjà été excecutée
    {//on restaure le contexte
    	return contexte->sp;
    	//reprend la tâche là où elle avait été interrompue.
    }
    /*
     * sp_ini → premier lancement d’une tâche
     * sp → reprise d’une tâche déjà exécutée/suspendue
     * */
    return (uint32_t)contexte->sp; // fallback sécurité
}

/*--------------------------------------------------------------------------*
 *              --- Gestionnaire d'exception PEND SVC ---                   *
 * Descrip: Appelé lors de l'exception PEND SVC provoquée par pendsv_trigger*
 *      Sauvegarde le contexte CPU sur la pile de la tâche et provoque une  *
 *      commutation de contexte.                                            *
 *------------------------------------------------------------------------- */
 void __attribute__((naked)) _pend_svc(void) {
     /* Q2.31 : sauvegarde du complément de contexte et appel de
                  l'ordonnaceur */
     __asm__ __volatile__ (
                 "push   {r4-r11,lr} \n" /* Sauvegarder le complément de contexte
                                            sur la pile                          */
                 "mov    r0, sp      \n" /* r0 = 1er paramètre de task_switch    */
                 "bl     task_switch \n" /* Commutation de contexte              */
                 "mov    sp, r0      \n" /* r0 = nouveau pointeur de pile        */
                 "pop    {r4-r11,lr} \n" /* Restituer le contexte                */
                 "bx     lr          \n" /* Retour d'exception                   */
     );
 }

/*--------------------------------------------------------------------------*
 *                  --- Provoquer une commutation ---                       *
 *                                                                          *
 * Descrip: planifie une exception PEND SVC pour forcer une commutation     *
 *      de tâche                                                            *
 *--------------------------------------------------------------------------*/
void schedule(void)
{
    pendsv_trigger();
}



/*
 * endort la tache courante
 * entre  : sans
 * sortie : sans
 * description : endort la tache courante et lance l'execution d'une nouvelle
 *               tache
 */
void dort(void) {
    _lock_();
    _noyau_tcb[_tache_c].status = SUSP;
    file_retire(_tache_c);   // la sortir de la queue
    //Car si on endort une tache, on doit retirer de la file
    _unlock_();
    schedule();              // céder la main immédiatement
}
/*
 * reveille une tache
 * entre  : numero de la tache a reveiller
 * sortie : sans
 * description : reveille une tache
 *               en cas d'erreur, le noyau doit etre arrete
 */
void reveille(uint16_t t) {
    NOYAU_TCB *p = &_noyau_tcb[t];
    _lock_();
    if (p->status == SUSP) {
        p->status = PRET;
        file_ajoute(t);   // la remettre dans la queue
    }
    _unlock_();
    schedule();           // laisser le scheduler décider si elle doit tourner maintenant
}
