/*----------------------------------------------------------------------------*
 * fichier : noyau_file.c                                                     *
 * gestion de la file d'attente des taches pretes et actives                  *
 * la file est rangee dans un tableau. ce fichier definit toutes              *
 * les primitives de base                                                     *
 *----------------------------------------------------------------------------*/

#include <stdint.h>
#include "noyau_file.h"

// recuperation du bon fichier selon l'architecture pour la fonction printf
#include "../io/serialio.h"

/*----------------------------------------------------------------------------*
 * variables communes a toutes les procedures                                 *
 *----------------------------------------------------------------------------*/

/*
 * tableau qui stocke les taches
 * indice = numero de tache
 * valeur = tache suivante
 */
static uint16_t _file[MAX_TACHES];

/*
 * index de queue
 * valeur de l'index de la tache en cours d'execution
 * pointe sur la prochaine tache a activer
 */
static uint16_t _queue;

/*----------------------------------------------------------------------------*
 * fonctions de gestion de la file                                            *
 *----------------------------------------------------------------------------*/

/*
 * initialise la file
 * entre  : sans
 * sortie : sans
 * description : la queue est initialisee à une valeur de tache impossible
 */
void file_init(void) {
    _queue = MAX_TACHES;

    for(uint16_t i = 0; i<MAX_TACHES;i++)
    {
    	_file[i] = MAX_TACHES;
    }
}

/*
 * ajoute une tache dans la file
 * entre  : n numero de la tache a ajouter
 * sortie : sans
 * description : ajoute la tache n en fin de file
 */
void file_ajoute(uint16_t n) {
	if(n >= MAX_TACHES)
		return;
	if(_queue == MAX_TACHES)
    {
    	_queue = n;
    	_file[_queue] = n;
    	return;
    }
    else if(_file[n] != MAX_TACHES) //_file[n] est le sucesseur de la tâche d'indice n
    //donc si _file[n] != MAX_TACHES, alors la tâche n existe car il a un sucesseur
    {
    	printf("La tâche %d existe déjà !\n",n);
    }
    else
    {
    	_file[n] = _file[_queue]; //Pour propager l'indice de la premiere tâche ajoutée
    	_file[_queue] = n;
    	_queue = n;
    }
	//Techniquement on a pas besoin de verifier si la file est pleine, car c'est fait
	//De maniere implicite dans _file[n] != MAX_TACHES
}

/*
 * retire une tache de la file
 * entre  : t numero de la tache a retirer
 * sortie : sans
 * description : retire la tache t de la file. L'ordre de la file n'est pas
                 modifie
 */
void file_retire(uint16_t t) {
	if(t >= MAX_TACHES)
		return;
	if(_queue == MAX_TACHES)
	{
		printf("La file est vide !\n");
		return;
	}
	else if(_file[t] == MAX_TACHES) //Car s'il existe, il a forcement, un suivant
	{
		printf("La tâche n'existe pas !");
	}
	else if(_file[_queue] == _queue) //La file contient un seul élement
	{
		_file[_queue] = MAX_TACHES;
		_queue = MAX_TACHES;
	}
	else
	{
		for(uint16_t i = 0; i<MAX_TACHES;i++)
		{
			if(_file[i] == t)//Cela signifie que i est le predecesseur
			{
				_file[i] = _file[t];
				_file[t] = MAX_TACHES;
				if(_queue == t) //Si la queue était t, alors, on met à jour
					_queue = i;
				break;
			}
		}

	}
}
//_queue == n vérifie certes qu'on a une seule valeur, mais aussi, qu'on est positionnés
//Sur la derniere valeur ajoutée dans la file
// 5 -> 6 -> 2 Ici, _queue == 2, si n == 2, alors on est dans ce cas

/*
 * recherche la tache suivante a executer
 * entre  : sans
 * sortie : numero de la tache a activer
 * description : queue pointe sur la tache suivante
 */
uint16_t file_suivant(void) {
    if(_queue == MAX_TACHES)
        return MAX_TACHES;

    _queue = _file[_queue];
    return _queue;
}

/*
 * affiche la queue, donc la derniere tache
 * entre  : sans
 * sortie : sans
 * description : affiche la valeur de queue
 */
void file_affiche_queue() {
    printf("_queue = %d\n", _queue);
}

/*
 * affiche la file
 * entre  : sans
 * sortie : sans
 * description : affiche les valeurs de la file
 */
void file_affiche() {
    int i;

    printf("Tache   | ");
    for (i = 0; i < MAX_TACHES; i++) {
        printf("%03d | ", i);
    }

    printf("\nSuivant | ");
    for (i = 0; i < MAX_TACHES; i++) {
        printf("%03d | ", _file[i]);
    }
    printf("\n");
}
