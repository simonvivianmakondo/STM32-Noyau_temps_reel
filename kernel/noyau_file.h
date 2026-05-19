/*----------------------------------------------------------------------------*
 * fichier : noyau_file.h                                                     *
 * gestion de la file d'attente des taches pretes et actives                  *
 * ce fichier declare toutes les primitives de base                           *
 *----------------------------------------------------------------------------*/

#ifndef __NOYAU_FILE_H__
#define __NOYAU_FILE_H__

#include <stdint.h>

/*----------------------------------------------------------------------------*
 * declaration des constantes                                                 *
 *----------------------------------------------------------------------------*/

/*
 * nombre maximum de taches dans chaque file
 */
#define MAX_TACHES  8
#define MAX_PRIO    8

#define MAX_TACHES_NOYAU MAX_PRIO * MAX_TACHES

/*
 * numero de tache impossible, utilise pour savoir si la file est initialisee
 * ou non
 */
#define F_VIDE      MAX_TACHES

/*----------------------------------------------------------------------------*
 * prototypes des fonctions de gestion de la file                             *
 * voir le fichier noyau_file.c pour avoir le comportement des fonctions      *
 *----------------------------------------------------------------------------*/

void file_init(void);
void file_ajoute(uint16_t t);
void file_retire(uint16_t t);
uint16_t file_suivant(void);
void file_affiche(void);
uint16_t num(int x);
uint16_t prio(int x);


#endif //__NOYAU_FILE_H__
