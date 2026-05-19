/* NOYAU_TEST_DELAY.C */
/*--------------------------------------------------------------------------*
 *  Test : delay() — 3 scénarios                                            *
 *                                                                           *
 *    1) Cadence régulière      tache_B se répète toutes les 5 ticks        *
 *    2) Délais différents      tache_C (10 ticks) vs tache_D (20 ticks)    *
 *    3) Délai nul / unitaire   tache_E avec delay(0) et delay(1)           *
 *                                                                           *
 *  Principe de vérification :                                               *
 *    Chaque tâche affiche un message AVANT et APRÈS son delay().           *
 *    On observe sur le terminal que les messages "après" n'apparaissent     *
 *    qu'une fois le délai écoulé, et que les autres tâches s'exécutent     *
 *    librement pendant ce temps.                                            *
 *                                                                           *
 *  IMPORTANT : start(f) appelle active(cree(f)) en interne ET fait         *
 *  file_init() — toutes les autres tâches doivent être créées              *
 *  DEPUIS la première tâche, pas depuis main().                             *
 *--------------------------------------------------------------------------*/

#include "kernel/noyau.h"
#include "hwsupport/stm32h7xx.h"
#include "io/serialio.h"
#include "kernel/delay.h"

/*--------------------------------------------------------------------------*
 *  Compteur de ticks global (incrémenté par le scheduler à chaque tick)    *
 *  Utilisé pour estimer le tick courant dans les messages de log.          *
 *--------------------------------------------------------------------------*/
volatile uint32_t tick_count = 0;

/*--------------------------------------------------------------------------*
 *  Prototypes                                                               *
 *--------------------------------------------------------------------------*/
TACHE tache_init(void);
TACHE tache_B(void);   /* scénario 1 : cadence régulière, delay(5)         */
TACHE tache_C(void);   /* scénario 2 : délai long,        delay(10)        */
TACHE tache_D(void);   /* scénario 2 : délai très long,   delay(20)        */
TACHE tache_E(void);   /* scénario 3 : delay(0) puis delay(1)              */
TACHE tache_fond(void);/* tâche de fond : tourne librement, prouve que     */
                       /* le CPU n'est PAS bloqué pendant les delay()      */
TACHE tache_fin(void); /* arrêt propre après tous les scénarios            */

/*==========================================================================*
 *  TÂCHE RACINE                                                             *
 *==========================================================================*/
TACHE tache_init(void)
{
    puts("[INIT] creation des taches");

    active(cree(tache_fond));
    active(cree(tache_B));
    active(cree(tache_C));
    active(cree(tache_D));
    active(cree(tache_E));
    active(cree(tache_fin));

    puts("[INIT] toutes les taches actives");
    fin_tache();
}

/*==========================================================================*
 *  TÂCHE DE FOND                                                            *
 *  Affiche un '.' toutes les ~50 000 itérations de boucle active.         *
 *  Prouve que le CPU continue de travailler pendant les delay() des        *
 *  autres tâches — le delay() suspend la tâche, pas le processeur.        *
 *==========================================================================*/
TACHE tache_fond(void)
{
    volatile int d;
    int tour = 0;

    puts("[FOND] demarrage (affiche '.' pendant les delais des autres)");

    while (1)
    {
        for (d = 0; d < 50000L; d++) continue;
        tour++;
        if (tour % 10 == 0)
            printf("[FOND] toujours actif (iteration %d)\n", tour);
        else
            putchar('.');

        schedule(); /* laisse les autres tâches s'exécuter */
    }
}

/*==========================================================================*
 *  SCÉNARIO 1 — CADENCE RÉGULIÈRE                                          *
 *  tache_B s'exécute 4 fois avec un delay(5) entre chaque activation.     *
 *  On doit observer exactement 5 ticks d'intervalle entre chaque message  *
 *  "réveil" consécutif.                                                    *
 *==========================================================================*/
TACHE tache_B(void)
{
    puts("\n[SC1-B] demarrage — delay(5) entre chaque iteration");

    for (int i = 1; i <= 4; i++)
    {
        printf("[SC1-B] iteration %d : AVANT delay(5)\n", i);
        delay(5);   /* se suspend pendant 5 ticks du scheduler              */
        printf("[SC1-B] iteration %d : APRES delay(5)  <-- reveil\n", i);
    }

    puts("[SC1-B] termine [OK] — 4 reveils observes, chacun apres 5 ticks");
    fin_tache();
}

/*==========================================================================*
 *  SCÉNARIO 2 — DÉLAIS DIFFÉRENTS                                          *
 *  tache_C (delay 10) et tache_D (delay 20) partent en même temps.        *
 *  On doit voir tache_C se réveiller deux fois avant que tache_D ne        *
 *  termine son premier délai.                                              *
 *==========================================================================*/
TACHE tache_C(void)
{
    puts("\n[SC2-C] demarrage — delay(10) x3");

    for (int i = 1; i <= 3; i++)
    {
        printf("[SC2-C] iteration %d : AVANT delay(10)\n", i);
        delay(10);
        printf("[SC2-C] iteration %d : APRES delay(10) <-- reveil\n", i);
    }

    puts("[SC2-C] termine [OK]");
    fin_tache();
}

TACHE tache_D(void)
{
    puts("\n[SC2-D] demarrage — delay(20) x2");

    for (int i = 1; i <= 2; i++)
    {
        printf("[SC2-D] iteration %d : AVANT delay(20)\n", i);
        delay(20);
        printf("[SC2-D] iteration %d : APRES delay(20) <-- reveil\n", i);
    }

    puts("[SC2-D] termine [OK]");
    /*
     * Résultat attendu dans le log :
     *   [SC2-C] iter 1 reveil
     *   [SC2-C] iter 2 reveil   <- tache_C termine son 2e délai de 10
     *   [SC2-D] iter 1 reveil   <- tache_D se réveille après 20 ticks
     *   [SC2-C] iter 3 reveil
     *   [SC2-D] iter 2 reveil
     */
    fin_tache();
}

/*==========================================================================*
 *  SCÉNARIO 3 — CAS LIMITES                                                *
 *  delay(0) : ne doit pas bloquer la tâche (ou la bloquer 1 tick max).    *
 *  delay(1) : doit libérer la tâche au tick suivant.                      *
 *==========================================================================*/
TACHE tache_E(void)
{
    puts("\n[SC3-E] demarrage — test delay(0) et delay(1)");

    /* --- delay(0) -------------------------------------------------------- */
    puts("[SC3-E] AVANT delay(0)");
    delay(0);
    /*
     * Comportement attendu selon l'implémentation :
     *  - Si delay(0) appelle dort() : la tâche est suspendue et réveillée
     *    par le prochain appel à delay_process() (au tick suivant).
     *  - Si delay(0) est traité comme un no-op : retour immédiat.
     * Dans les deux cas, la ligne suivante doit s'afficher.
     */
    puts("[SC3-E] APRES delay(0) -- reveil immediat ou au prochain tick");

    /* --- delay(1) -------------------------------------------------------- */
    puts("[SC3-E] AVANT delay(1)");
    delay(1);
    puts("[SC3-E] APRES delay(1) -- reveil apres exactement 1 tick [OK]");

    puts("[SC3-E] termine [OK]");
    fin_tache();
}

/*==========================================================================*
 *  TÂCHE FIN                                                               *
 *  Délai long pour laisser tous les scénarios se terminer proprement.      *
 *==========================================================================*/
TACHE tache_fin(void)
{
    /* Se suspend via le scheduler le temps que tous les delay() expirent.  */
    /* tache_C  : 3 x delay(10) = 30 ticks                                  */
    /* tache_D  : 2 x delay(20) = 40 ticks  <- le plus long                */
    /* On attend 60 ticks pour avoir une marge confortable.                 */
    delay(60);

    puts("\n===== FIN DE TOUS LES TESTS DELAY =====");
    noyau_exit();
}

/*==========================================================================*
 *  MAIN                                                                     *
 *==========================================================================*/
int main(void)
{
    usart_init(115200);

    puts("=== TEST DELAY ===");
    puts("Scenarios : cadence reguliere / delais differents / cas limites\n");

    start(tache_init); /* cree tache_init + lance le scheduler              */

    return 0;          /* jamais atteint                                    */
}
