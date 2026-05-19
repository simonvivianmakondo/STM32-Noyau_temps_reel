/*----------------------------------------------------------------------------*
 * fichier : sem.c                                                            *
 * gestion des semaphores pour le mini-noyau temps reel                       *
 *----------------------------------------------------------------------------*/

#include "sem.h"
#include "fifo.h"
#include "noyau.h"
#include "noyau_file.h"

/*----------------------------------------------------------------------------*
 * declaration des structures                                                 *
 *----------------------------------------------------------------------------*/

/*
 * structure definissant un semaphore
 */

/*
 * Dans un sémaphore, e représente le nombre de ressources disponibles.
 *
 *  - e > 0  : des ressources sont libres, aucune tâche n’attend.
 *  - e == 0 : toutes les ressources sont utilisées.
 *  - e < 0  : des tâches sont bloquées dans la file d’attente.
 *
 * Lors d’un V(s), on incrémente e.
 * Si e <= 0 après l’incrémentation, cela signifie qu’au moins
 * une tâche était en attente avant le V(s), il faut donc en réveiller une.
 *
 * Si e > 0, aucune tâche n’est bloquée : on ne réveille personne.
 */

typedef struct {
    FIFO file;
    int8_t valeur;
} SEMAPHORE;

/*----------------------------------------------------------------------------*
 * variables globales internes                                                *
 *----------------------------------------------------------------------------*/
/*
 * variable stockant tous les semaphores du systeme
 */
SEMAPHORE _sem[MAX_SEM];

/*----------------------------------------------------------------------------*
 * definition des fonctions                                                   *
 *----------------------------------------------------------------------------*/

/*
 * /!!!!\ NOTE IMPORTANTE /!!!!\
 * pour faire les verifications de file, on pourra utiliser la variable de
 * file fifo_taille et la mettre a -1 dans le cas ou la file n'est pas
 * utilisee
 */

/*
 * initialise les sempapshore du systeme
 * entre  : sans
 * sortie : sans
 * description : initialise l'ensemble des semaphores du systeme
 */

//C’est une abstraction :
//on cache les variables internes pour rendre le noyau évolutif et propre.

void s_init(void)
{
    uint16_t i;
    for (i = 0; i < MAX_SEM; i++)
    {
        fifo_init(&_sem[i].file);
        _sem[i].valeur = 0;
    }
}

/*
 * cree un semaphore
 * entre  : valeur du semaphore a creer
 * sortie : numero du semaphore cree
 * description : cree un semaphore
 *               en cas d'erreur, le noyau doit etre arrete
 */
uint8_t s_cree(int8_t valeur)
{
    static uint8_t num_sem = 0;

    if (valeur < 0 || num_sem >= MAX_SEM)
        noyau_exit();

    _sem[num_sem].valeur = valeur;

    return num_sem++;
}

/*
 * ferme un semaphore pour qu'il puisse etre reutilise
 * entre  : numero du semaphore a fermer
 * sortie : sans
 * description : ferme un semaphore
 *               en cas d'erreur, le noyau doit etre arrete
 */
void s_close(uint8_t n)
{
    if(n >= MAX_SEM)
        noyau_exit();

    _lock_();
    _sem[n].valeur = 0;

    uint8_t t;

    while(fifo_retire(&_sem[n].file, &t) == -1)
    {
    	get_tcb(t)->status = EXEC;
        file_ajoute(t);
    }

    _unlock_();
    schedule();
}

/*
 * tente de prendre le semaphore dont le numero est passe en parametre
 * entre  : numero du semaphore a prendre
 * sortie : sans
 * description : prend le semaphore
 *               si echec, la tache doit etre suspendue
 *               en cas d'erreur, le noyau doit etre arrete
 */
void s_wait(uint8_t n)
{
	if(n >= MAX_SEM)
		noyau_exit();

    _lock_();

    uint16_t cur = get_tache_courante();
	_sem[n].valeur--;

	if(_sem[n].valeur < 0)
	{
		if(fifo_ajoute(&_sem[n].file,cur)==0)
		{
			noyau_exit();
			//return;
		}
		//_sem[n].file.fifo_queue = _tache_c; //deja gere par la fonction
		get_tcb(cur)->status = SUSP;
		file_retire(cur);
		_unlock_();
		schedule();
		return;
	}
	_unlock_();
}

/*
 * libere un semaphore
 * entre  : numero du semaphore a liberer
 * sortie : sans
 * description : libere un semaphore
 *               si des taches sont en attentes, la premiere doit etre reveillee
 *               en cas d'erreur, le noyau doit etre arrete
 */
void s_signal(uint8_t n)
{
	if(n >= MAX_SEM)
			noyau_exit();

    _lock_();
	_sem[n].valeur++;
	if(_sem[n].valeur <= 0)
	{
		uint8_t tache_retire;
		if(fifo_retire(&_sem[n].file,&tache_retire) == -1)
		{
			get_tcb(tache_retire)->status = EXEC;
			file_ajoute(tache_retire);
			_unlock_();
			schedule();
			return;
		}
		else
		{
			noyau_exit();
		}
	}
	_unlock_();
}





