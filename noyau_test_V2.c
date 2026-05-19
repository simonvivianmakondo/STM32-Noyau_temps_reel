/* NOYAU_TEST_DORT_REVEILLE.C */
/*--------------------------------------------------------------------------*
 *                Programme de test : dort / reveille                      *
 *--------------------------------------------------------------------------*/
/*
 * COMPARAISON ENTRE DEUX MECANISMES DU NOYAU
 *
 * --------------------------------------------------------------------------
 * 1) MULTITACHE AVEC SysTick (NOYAUTEST.C)
 * --------------------------------------------------------------------------
 *
 * Dans ce mode, le changement de tâche est principalement piloté par le temps.
 *
 * - SysTick déclenche une interruption périodique (ex: 10 ms)
 * - cette interruption provoque un passage par PendSV
 * - le scheduler sélectionne la prochaine tâche prête (PRET/EXEC)
 *
 * CARACTERISTIQUES :
 *  - toutes les tâches actives sont toujours candidates à l'exécution
 *  - aucune tâche ne peut volontairement se retirer du système
 *  - le partage CPU est basé uniquement sur le temps
 *
 * RESULTAT :
 *  - système préemptif temporel
 *  - équité basée sur le quantum de temps
 *  - aucune notion de blocage volontaire
 *
 *
 * --------------------------------------------------------------------------
 * 2) MULTITACHE AVEC dort() / reveille()
 * --------------------------------------------------------------------------
 *
 * Ici, on ajoute un mécanisme de gestion d'état des tâches.
 *
 * - dort() met la tâche courante à l'état SUSP (retirée de la file)
 * - reveille() remet une tâche SUSP dans l'état PRET
 * - schedule() force un changement de contexte via PendSV
 *
 * CARACTERISTIQUES :
 *  - certaines tâches peuvent être exclues temporairement de l'ordonnancement
 *  - le scheduler ne considère que les tâches PRET/EXEC
 *  - les tâches peuvent attendre un événement externe
 *
 * RESULTAT :
 *  - système préemptif avec blocage volontaire
 *  - gestion des événements / synchronisation entre tâches
 *  - réduction de l'exécution inutile des tâches en attente
 *
 *
 * --------------------------------------------------------------------------
 * DIFFERENCE FONDAMENTALE
 * --------------------------------------------------------------------------
 *
 * SysTick :
 *   → le temps décide qui exécute
 *
 * dort/reveille :
 *   → l’état des tâches décide qui a le droit d’exécuter,
 *     en plus du découpage temporel du SysTick
 *
 *
 * EN RESUME :
 * - SysTick = partage automatique du CPU
 * - dort/reveille = contrôle du droit d’exécution par état + synchronisation
 */
#include "kernel/noyau.h"
#include "hwsupport/stm32h7xx.h"
#include "io/serialio.h"

/*
 * Taches de test
 */
TACHE tacheA(void);
TACHE tacheB(void);
TACHE tacheC(void);

/* identifiant global pour réveiller B */
uint16_t idB;

TACHE tacheA(void)
{
    puts("------> EXEC tache A");

    idB = cree(tacheB);
    active(idB);

    active(cree(tacheC));

    puts("------> A va s'endormir");
    dort();

    puts("------> A est reveillee");

    reveille(idB);

}

TACHE tacheB(void)
{
    int i = 0;

    puts("------> DEBUT tache B");

    while (1) {
        for (int j = 0; j < 50000L; j++) continue;
        printf("======> B tourne %d\n", i++);
    }
}

TACHE tacheC(void)
{
    int i = 0;

    puts("------> DEBUT tache C");

    while (1) {
        for (int j = 0; j < 80000L; j++) continue;
        printf("======> C tourne %d\n", i++);
        if(i == 50)
        {
            puts("------> Fin du test");
            noyau_exit();
        }
    }
}

int main(void)
{
    usart_init(115200);

    puts("Test noyau dort / reveille");
    start(tacheA);

    puts("Fin");
    return 0;
}

/*
 * RESUME DU TEST :
 *
 * 1. A cree B et C
 * 2. B et C tournent
 * 3. A s’endort avec dort()
 *    -> A est retiree de la file et bloquee
 * 4. B et C continuent
 * 5. A est reveillee avec reveille()
 *    -> A revient dans la file PRET
 * 6. Scheduler decide quand elle reprend
 *
 * BUT :
 * - verifier que SUSP bloque bien une tache
 * - verifier que file_retire / file_ajoute fonctionnent
 * - verifier que schedule relance correctement
 */
