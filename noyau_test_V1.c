/* NOYAUTEST.C */
/*--------------------------------------------------------------------------*
 *			      Programme de tests avec les interruptions			    *
 *--------------------------------------------------------------------------*/

#include "kernel/noyau.h"
#include "hwsupport/stm32h7xx.h"
#include "io/serialio.h"

/*
 ** Test du noyau preemptif.
 */

TACHE tacheA(void);
TACHE tacheB(void);
TACHE tacheC(void);
TACHE tacheD(void);

TACHE tacheA(void)
{
    puts("------> EXEC tache A");
    active(cree(tacheB));
    active(cree(tacheC));
    active(cree(tacheD));
    fin_tache();
}

TACHE tacheB(void)
{
    int i = 0;
    puts("------> DEBUT tache B");
    while (1) {
        for (int j = 0; j < 30000L; j++) continue;
        printf("======> Dans tache B %d\n", i);
        i++;
    }
}

TACHE tacheC(void)
{
    int i = 0;
    puts("------> DEBUT tache C");
    while (1) {
        for (int j = 0; j < 60000L; j++) continue;
        printf("======> Dans tache C %d\n", i);
        i++;
    }
}

TACHE tacheD(void)
{
    int i = 0;
    puts("------> DEBUT tache D");
    while (1) {
        for (int j = 0; j < 120000L; j++) continue;
        printf("======> Dans tache D %d\n", i++);
        if (i == 50) {
            noyau_exit();
        }
    }
}

int main(void)
{
    usart_init(115200);
    puts("Test noyau");
    puts("Noyau preemptif");
    start(tacheA);
    puts("Fin");
    return (0);
}

// 	Chaque tâche a droit à 10ms de CPU maximum avant d'être interrompue :
/*
	Chaque fois que le SysTick interrompt :
	La tâche courante est suspendue
	Le scheduler choisit la tâche suivante dans la file circulaire
	Cette tâche s'exécute pendant 10ms
	Retour à l'étape 1
*/
