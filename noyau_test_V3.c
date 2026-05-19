/* NOYAU_TEST_SEMAPHORE2.C */
/*--------------------------------------------------------------------------*
 *  Test : sémaphores — 3 scénarios                                         *
 *    1) Section critique partagée  (semaphore binaire)                     *
 *    2) Synchronisation producteur / consommateur                          *
 *    3) Fermeture : s_close libère les tâches bloquées                     *
 *                                                                           *
 *  IMPORTANT : start(f) appelle active(cree(f)) en interne ET fait         *
 *  file_init() — toutes les autres tâches doivent être créées              *
 *  DEPUIS la première tâche, pas depuis main().                             *
 *                                                                           *
 *  NOTE : toutes les boucles de délai utilisent `volatile int d` pour      *
 *  empêcher le compilateur de les supprimer par optimisation.               *
 *--------------------------------------------------------------------------*/

#include "kernel/noyau.h"
#include "kernel/sem.h"
#include "hwsupport/stm32h7xx.h"
#include "io/serialio.h"

/*--------------------------------------------------------------------------*
 *  Sémaphores globaux                                                       *
 *--------------------------------------------------------------------------*/
uint8_t sem_section;  /* section critique — valeur initiale 1               */
uint8_t sem_sync;     /* synchronisation  — valeur initiale 0               */
uint8_t sem_close;    /* test s_close     — valeur initiale 0               */

/*--------------------------------------------------------------------------*
 *  Prototypes                                                               *
 *--------------------------------------------------------------------------*/
TACHE tache_init(void);

/* --- scénario 1 : section critique --- */
TACHE tache_ecrivain_A(void);
TACHE tache_ecrivain_B(void);

/* --- scénario 2 : producteur / consommateur --- */
TACHE tache_prod(void);
TACHE tache_cons(void);

/* --- scénario 3 : s_close --- */
TACHE tache_bloquee(void);
TACHE tache_closeur(void);

/* --- arrêt propre --- */
TACHE tache_fin(void);

/*==========================================================================*
 *  TÂCHE RACINE                                                             *
 *  start() la crée en interne ; elle crée toutes les autres.               *
 *==========================================================================*/
TACHE tache_init(void)
{
    puts("[INIT] creation des taches");

    active(cree(tache_ecrivain_A));
    active(cree(tache_ecrivain_B));
    active(cree(tache_prod));
    active(cree(tache_cons));
    active(cree(tache_bloquee));
    active(cree(tache_closeur));
    active(cree(tache_fin));

    puts("[INIT] toutes les taches actives");
    fin_tache();
}

/*==========================================================================*
 *  SCÉNARIO 1 — SECTION CRITIQUE                                           *
 *  Deux tâches incrémentent un compteur partagé protégé par sémaphore.    *
 *  schedule() après chaque libération force l'entrelacement A <-> B.      *
 *  Résultat attendu : compteur == 10.                                      *
 *==========================================================================*/
volatile int compteur_partage = 0;

TACHE tache_ecrivain_A(void)
{
    volatile int d;
    puts("[SC-A] demarrage");

    for (int i = 0; i < 5; i++)
    {
        s_wait(sem_section);                         /* entre en section critique */

        int tmp = compteur_partage;
        for (d = 0; d < 20000L; d++) continue;      /* simule un calcul long     */
        compteur_partage = tmp + 1;
        printf("[SC-A] compteur = %d\n", compteur_partage);

        s_signal(sem_section);                       /* libère la section         */
        schedule();                                  /* force l'entrelacement     */
    }

    puts("[SC-A] termine");
    fin_tache();
}

TACHE tache_ecrivain_B(void)
{
    volatile int d;
    puts("[SC-B] demarrage");

    for (int i = 0; i < 5; i++)
    {
        s_wait(sem_section);

        int tmp = compteur_partage;
        for (d = 0; d < 20000L; d++) continue;
        compteur_partage = tmp + 1;
        printf("[SC-B] compteur = %d\n", compteur_partage);

        s_signal(sem_section);
        schedule();
    }

    puts("[SC-B] termine");
    printf("[SC] compteur final = %d (attendu 10) %s\n",
           compteur_partage,
           compteur_partage == 10 ? "[OK]" : "[ERREUR]");
    fin_tache();
}

/*==========================================================================*
 *  SCÉNARIO 2 — PRODUCTEUR / CONSOMMATEUR                                  *
 *  Le producteur génère 6 tokens un par un avec un délai entre chaque.    *
 *  Le consommateur est bloqué sur s_wait() jusqu'à disponibilité.          *
 *==========================================================================*/
TACHE tache_prod(void)
{
    volatile int d;
    puts("[PROD] demarrage");

    for (int i = 1; i <= 6; i++)
    {
        for (d = 0; d < 60000L; d++) continue;      /* cadence de production     */
        printf("[PROD] token %d disponible\n", i);
        s_signal(sem_sync);                          /* signale un token pret     */
    }

    puts("[PROD] termine");
    fin_tache();
}

TACHE tache_cons(void)
{
    puts("[CONS] demarrage");

    for (int i = 1; i <= 6; i++)
    {
        s_wait(sem_sync);                            /* attend un token           */
        printf("[CONS] token %d consomme\n", i);
    }

    puts("[CONS] termine [OK]");
    fin_tache();
}

/*==========================================================================*
 *  SCÉNARIO 3 — s_close                                                    *
 *  tache_bloquee se bloque sur s_wait(sem_close) car valeur = 0.          *
 *  tache_closeur appelle s_close() après un délai :                        *
 *  tache_bloquee doit être réveillée et poursuivre son exécution.          *
 *==========================================================================*/
TACHE tache_bloquee(void)
{
    puts("[CLOSE] tache_bloquee : entre dans s_wait -> va se bloquer");
    s_wait(sem_close);
    puts("[CLOSE] tache_bloquee : reveillee par s_close [OK]");
    fin_tache();
}

TACHE tache_closeur(void)
{
    volatile int d;
    puts("[CLOSE] tache_closeur : attend avant de fermer");
    for (d = 0; d < 500000L; d++) continue;

    puts("[CLOSE] tache_closeur : appel s_close");
    s_close(sem_close);                              /* réveille tache_bloquee   */

    puts("[CLOSE] tache_closeur : termine");
    fin_tache();
}

/*==========================================================================*
 *  TÂCHE FIN                                                               *
 *  Délai long pour laisser tous les scénarios se terminer proprement.      *
 *==========================================================================*/
TACHE tache_fin(void)
{
    volatile int d;
    for (d = 0; d < 5000000L; d++) continue;

    puts("\n===== FIN DE TOUS LES TESTS =====");
    noyau_exit();
}

/*==========================================================================*
 *  MAIN                                                                     *
 *==========================================================================*/
int main(void)
{
    usart_init(115200);

    puts("=== TEST SEMAPHORES 2 ===");
    puts("Scenarios : section critique / prod-cons / s_close\n");

    s_init();

    sem_section = s_cree(1);  /* 1 seul accès autorisé à la fois            */
    sem_sync    = s_cree(0);  /* 0 token au départ                          */
    sem_close   = s_cree(0);  /* tache_bloquee se bloquera immédiatement    */

    start(tache_init);        /* crée tache_init + lance le scheduler        */

    return 0;                 /* jamais atteint                              */
}
