/*
 * utils.c
 *
 *  Created on: 28 mai 2020
 *      Author: jdm
 */

#include <stdint.h>
#include <stdlib.h>

#include "noyau_prio.h"
#include "noyau_file.h"
#include "delay.h"

/*----------------------------------------------------------------------------*
 * fonctions de gestion des délais                                            *
 *----------------------------------------------------------------------------*/

/*
 * entrée  : nombre de tick d'attente
 * sortie  : sans
 * description : renseigne le compteur de délai de la structure _contexte de la tâche courante
 * 				 endort la tâche
 */


void delay(uint32_t nticks)
{
	if(nticks == 0)
		return;

	NOYAU_TCB *p = noyau_get_p_tcb(noyau_get_tc());
	_lock_();
	p->delay = nticks;
	dort();
	_unlock_();
}

/*
 * entrée  : sans (fonction appelée dans scheduler)
 * sortie : sans
 * description : parcours l'ensemble des contextes de tâches pour vérifier celles qui sont SUSP
 * 				 pour celles dont le compteur est non nul
 * 				 	décrémente le compteur
 * 				 	remet la tache en exécution si son compteur arrive à zéro
 *
 */
void delay_process(void)
{
	uint16_t i = 0;
	NOYAU_TCB *p;
	_lock_();
	for(i=0;i<MAX_TACHES_NOYAU;i++)
	{
		p = noyau_get_p_tcb(i);
		if(p->delay > 0)
		{
			p->delay--;
			if(p->delay == 0)
			{
				reveille(i);
			}
		}
	}
	_unlock_();
}

