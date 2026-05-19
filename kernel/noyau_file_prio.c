/*
 * noyau_prio_file.c
 *
 *  Created on: 2 mai 2025
 *      Author: mi11
 */

/*----------------------------------------------------------------------------*
 * fichier : noyau_file.c                                                     *
 * gestion de la file d'attente des taches pretes et actives                  *
 * la file est rangee dans un tableau. ce fichier definit toutes              *
 * les primitives de base                                                     *
 *----------------------------------------------------------------------------*/
/*
    On utilise une matrice de files car chaque priorité possède
    son propre tourniquet (file circulaire).

    _file[p][i]
    p = niveau de priorité
    i = position dans la file de cette priorité

    Chaque priorité doit donc gérer :
    - ses propres tâches
    - son propre ordre d’exécution

    _queue devient aussi un tableau car chaque tourniquet
    possède son propre pointeur de queue.
*/
#include <stdint.h>
#include "../kernel/noyau_file.h"

// recuperation du bon fichier selon l'architecture pour la fonction printf
#include "io/serialio.h"

/*----------------------------------------------------------------------------*
 * variables communes a toutes les procedures                                 *
 *----------------------------------------------------------------------------*/

/*
 * tableau qui stocke les taches
 * indice = numero de tache
 * valeur = tache suivante
 */
static uint16_t _file[MAX_PRIO][MAX_TACHES];

uint16_t num(int x)
{
    return x & 0b111;
}

uint16_t prio(int x)
{
    return x >> 3;
}

/*
 * index de queue
 * valeur de l'index de la tache en cours d'execution
 * pointe sur la prochaine tache a activer
 */
static uint16_t _queue[MAX_PRIO];

/*----------------------------------------------------------------------------*
 * fonctions de gestion de la file                                            *
 *----------------------------------------------------------------------------*/

/*
 * initialise la file
 * entre  : sans
 * sortie : sans
 * description : la queue est initialisee à une valeur de tache impossible
 */
void file_init(void)
{
	uint16_t i;
	uint16_t j;
	for(i=0;i<MAX_PRIO;i++)
	{
		_queue[i] = MAX_TACHES;
		for(j = 0; j < MAX_TACHES;j++)
		{
			_file[i][j] = MAX_TACHES;
		}
	}
}

/*
 * ajoute une tache dans la file
 * entre  : n numero de la tache a ajouter
 * sortie : sans
 * description : ajoute la tache n en fin de file
 */
void file_ajoute(uint16_t n)
{
    uint16_t prio_tache = prio(n);
    uint16_t num_tache  = num(n);

    if(prio_tache >= MAX_PRIO || num_tache >= MAX_TACHES)
        return;

    // tâche déjà présente
    if(_file[prio_tache][num_tache] != MAX_TACHES)
    {
        printf("La tâche %d existe déjà !\n", n);
        return;
    }

    // file vide
    if(_queue[prio_tache] == MAX_TACHES)
    {
        _queue[prio_tache] = num_tache;
        _file[prio_tache][num_tache] = num_tache;
    }
    else
    {
        _file[prio_tache][num_tache] =
            _file[prio_tache][_queue[prio_tache]];

        _file[prio_tache][_queue[prio_tache]] =
            num_tache;

        _queue[prio_tache] = num_tache;
    }
}

/*
 * retire une tache de la file
 * entre  : t numero de la tache a retirer
 * sortie : sans
 * description : retire la tache t de la file. L'ordre de la file n'est pas
                 modifie
 */
void file_retire(uint16_t t)
{
	uint16_t prio_tache = prio(t);
	uint16_t num_tache  = num(t);

	if(prio_tache >= MAX_PRIO || num_tache >= MAX_TACHES)
		return;

	if(_queue[prio_tache] == MAX_TACHES)
	{
		printf("La file est vide !\n");
		return;
	}
	else if(_file[prio_tache][num_tache] == MAX_TACHES) //Car s'il existe, il a forcement, un suivant
	{
		printf("La tâche n'existe pas !");
		return;
	}
	else if(_file[prio_tache][num_tache] == num_tache) //La file contient un seul élement
	{
		_file[prio_tache][num_tache] = MAX_TACHES;
		_queue[prio_tache] = MAX_TACHES;
	}
	else
	{
		for(uint16_t i = 0; i<MAX_TACHES;i++)
		{
			if(_file[prio_tache][i] == num_tache)//Cela signifie que i est le predecesseur
			{
				_file[prio_tache][i] = _file[prio_tache][num_tache];
				_file[prio_tache][num_tache] = MAX_TACHES;
				if(_queue[prio_tache] == num_tache) //Si la queue était t, alors, on met à jour
					_queue[prio_tache] = i;
				break;
			}
		}

	}
}

/*
 * recherche la tache suivante a executer
 * entre  : sans
 * sortie : numero de la tache a activer
 * description : queue pointe sur la tache suivante
 */
uint16_t file_suivant(void)
{
    for(uint16_t i = 0; i < MAX_PRIO; i++)
    {
        if(_queue[i] == MAX_TACHES)
            continue;

        // avancer dans le tourniquet
        _queue[i] = _file[i][_queue[i]];

        return (i << 3) | _queue[i];
    }

    return  MAX_TACHES_NOYAU;
}

/*
 * affiche la queue, donc la derniere tache
 * entre  : sans
 * sortie : sans
 * description : affiche la valeur de queue
 */
void file_affiche_queue() {
	uint16_t i;
	for (i=0; i<MAX_PRIO; i++){
		 printf("_queue[%d] = %d\n", i, _queue[i]);
	}
}

/*
 * affiche la file
 * entre  : sans
 * sortie : sans
 * description : affiche les valeurs de la file
 */
void file_affiche() {
	uint16_t i,j;

    for (j=0; j<MAX_PRIO; j++){
		printf("Tache   | ");
		for (i = 0; i < MAX_TACHES; i++) {
			printf("%03d | ", i);
		}

		printf("\nSuivant | ");
		for (i = 0; i < MAX_TACHES; i++) {
			printf("%03d | ", _file[j][i]);
		}
		printf("\n");
    }
}

