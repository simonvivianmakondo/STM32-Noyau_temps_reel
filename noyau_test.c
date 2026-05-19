/*
 * noyau_test.c
 *
 *  Created on: 7 avr. 2026
 *      Author: mi11p002
 *
 * file_suivant() fait :
 *   _queue = _file[_queue];
 *   return _file[_queue];
 *
 * Donc pour une file [a, b, c, d] ajoutee dans cet ordre (_queue = d) :
 *   appel 1 : _queue passe a a, retourne _file[a] = b -> b
 *   appel 2 : _queue passe a b, retourne _file[b] = c -> c
 *   appel 3 : _queue passe a c, retourne _file[c] = d -> d
 *   appel 4 : _queue passe a d, retourne _file[d] = a -> a (cycle)
 *
 * Autrement dit : file_suivant() saute le 1er element (la "tete naturelle")
 * et commence par retourner le 2eme element (= le plus ancien apres avancement).
 *
 * Cas particuliers :
 *   - File vide : retourne F_VIDE (= MAX_TACHES)
 *   - File a 1 element x : retourne toujours x (l'element pointe sur lui-meme)
 *   - File a 2 elements [a, b] : retourne b, a, b, a, ... en alternant
 */
#include "kernel/noyau_file.h"
#include "kernel/noyau.h"
#include "hwsupport/stm32h7xx.h"
#include "io/serialio.h"

/*
 * Test 1 : ajout dans une file vide
 */
void test_ajout_file_vide() {
	printf("\nTest ajout dans file vide\n");
	file_init();
	file_ajoute(4);
	uint16_t res = file_suivant();
	printf("ajoute(4) dans file vide, suivant: %d (attendu 4) -> %s\n",
		res, res == 4 ? "OK" : "FAIL");
}

/*
 * Test 2 : ajout quand il reste une seule place
 */
void test_ajout_presque_pleine() {
	printf("\nTest ajout quand il reste une place\n");
	file_init();
	for (uint16_t i = 0; i < MAX_TACHES - 1; i++)
		file_ajoute(i);
	file_ajoute(MAX_TACHES - 1);
	uint16_t trouve = 0;
	for (uint16_t i = 0; i < MAX_TACHES; i++) {
		uint16_t res = file_suivant();
		if (res == MAX_TACHES - 1) {
			trouve = 1;
			break;
		}
	}
	printf("derniere place remplie avec %d -> %s\n",
		MAX_TACHES - 1, trouve ? "OK" : "FAIL");
}

/*
 * Test 3 : ajout avec indice invalide
 */
void test_ajout_indice_invalide() {
	printf("\nTest ajout avec indice invalide\n");
	file_init();
	file_ajoute(0);
	file_ajoute(MAX_TACHES);
	file_ajoute(MAX_TACHES + 5);
	uint16_t r0 = file_suivant();
	uint16_t r1 = file_suivant();
	printf("ajoute(0,MAX_TACHES,MAX+5): %d %d (attendu 0 0) -> %s\n",
		r0, r1, (r0 == 0 && r1 == 0) ? "OK" : "FAIL");
}

/*
 * Test 4 : ajout d'un doublon
 * File [2, 5] -> suivant() retourne 5, 2, 5
 */
void test_ajout_doublon() {
	printf("\nTest ajout doublon\n");
	file_init();
	file_ajoute(2);
	file_ajoute(5);
	file_ajoute(2); // doublon
	uint16_t r0 = file_suivant();
	uint16_t r1 = file_suivant();
	uint16_t r2 = file_suivant();
	printf("ajoute(2,5,2): %d %d %d (attendu 5 2 5) -> %s\n",
		r0, r1, r2,
		(r0 == 5 && r1 == 2 && r2 == 5) ? "OK" : "FAIL");
}

/*
 * Test 5 : retrait dans une file vide
 */
void test_retire_file_vide() {
	printf("\nTest retire dans file vide\n");
	file_init();
	file_retire(3);
	uint16_t res = file_suivant();
	printf("retire(3) sur file vide, suivant: %d (attendu %d) -> %s\n",
		res, F_VIDE, res == F_VIDE ? "OK" : "FAIL");
}

/*
 * Test 6 : retrait d'une tache qui n'existe pas
 * File [1, 3] -> suivant() retourne 3, 1
 */
void test_retire_inexistante() {
	printf("\nTest retire tache inexistante\n");
	file_init();
	file_ajoute(1);
	file_ajoute(3);
	file_retire(7);
	uint16_t r0 = file_suivant();
	uint16_t r1 = file_suivant();
	printf("retire(7) inexistante: %d %d (attendu 3 1) -> %s\n",
		r0, r1, (r0 == 3 && r1 == 1) ? "OK" : "FAIL");
}

/*
 * Test 7 : retrait avec indice invalide
 * File [1, 2] -> suivant() retourne 2, 1
 */
void test_retire_indice_invalide() {
	printf("\nTest retire indice invalide\n");
	file_init();
	file_ajoute(1);
	file_ajoute(2);
	file_retire(MAX_TACHES);
	file_retire(MAX_TACHES + 3);
	uint16_t r0 = file_suivant();
	uint16_t r1 = file_suivant();
	printf("retire(MAX_TACHES) ignore: %d %d (attendu 2 1) -> %s\n",
		r0, r1, (r0 == 2 && r1 == 1) ? "OK" : "FAIL");
}

/*
 * Test 8 : retrait du seul element de la file
 */
void test_retire_seul_element() {
	printf("\nTest retire le seul element\n");
	file_init();
	file_ajoute(6);
	file_retire(6);
	uint16_t res = file_suivant();
	printf("ajoute(6) puis retire(6), suivant: %d (attendu %d) -> %s\n",
		res, F_VIDE, res == F_VIDE ? "OK" : "FAIL");
}

/*
 * Test 9 : retrait de la tete de la file
 * Apres retire(1) : reste [3, 5], _queue = 5
 * suivant() retourne 5, 3
 */
void test_retire_tete() {
	printf("\nTest retire tete\n");
	file_init();
	file_ajoute(1);
	file_ajoute(3);
	file_ajoute(5);
	file_retire(1);
	uint16_t r0 = file_suivant();
	uint16_t r1 = file_suivant();
	printf("retire tete(1): %d %d (attendu 5 3) -> %s\n",
		r0, r1, (r0 == 5 && r1 == 3) ? "OK" : "FAIL");
}

/*
 * Test 10 : retrait d'un element au milieu
 * Apres retire(3) : reste [1, 5], _queue = 5
 * suivant() retourne 5, 1
 */
void test_retire_milieu() {
	printf("\nTest retire milieu\n");
	file_init();
	file_ajoute(1);
	file_ajoute(3);
	file_ajoute(5);
	file_retire(3);
	uint16_t r0 = file_suivant();
	uint16_t r1 = file_suivant();
	printf("retire milieu(3): %d %d (attendu 5 1) -> %s\n",
		r0, r1, (r0 == 5 && r1 == 1) ? "OK" : "FAIL");
}

/*
 * Test 11 : retrait de la queue
 * Apres retire(5) : reste [1, 3], _queue = 3
 * suivant() retourne 3, 1, 3
 */
void test_retire_queue() {
	printf("\nTest retire queue\n");
	file_init();
	file_ajoute(1);
	file_ajoute(3);
	file_ajoute(5);
	file_retire(5);
	uint16_t r0 = file_suivant();
	uint16_t r1 = file_suivant();
	uint16_t r2 = file_suivant();
	printf("retire queue(5): %d %d %d (attendu 3 1 3) -> %s\n",
		r0, r1, r2,
		(r0 == 3 && r1 == 1 && r2 == 3) ? "OK" : "FAIL");
}

/*
 * Test 12 : retrait deux fois de la meme tache
 * Apres : reste 4 seul. suivant() retourne 4 toujours.
 */
void test_retire_deux_fois() {
	printf("\nTest retire deux fois la meme tache\n");
	file_init();
	file_ajoute(2);
	file_ajoute(4);
	file_retire(2);
	file_retire(2);
	uint16_t r0 = file_suivant();
	uint16_t r1 = file_suivant();
	printf("retire(2) deux fois: %d %d (attendu 4 4) -> %s\n",
		r0, r1, (r0 == 4 && r1 == 4) ? "OK" : "FAIL");
}

/*
 * Test 13 : ajouter, retirer, re-ajouter la meme tache
 * Apres : file = [5, 2], _queue = 2.
 * suivant() retourne 2, 5
 */
void test_cycle_ajout_retrait() {
	printf("\nTest cycle ajout/retrait/re-ajout\n");
	file_init();
	file_ajoute(2);
	file_ajoute(5);
	file_retire(2);
	file_ajoute(2);
	uint16_t r0 = file_suivant();
	uint16_t r1 = file_suivant();
	printf("ajoute/retire/re-ajoute(2): %d %d (attendu 2 5) -> %s\n",
		r0, r1, (r0 == 2 && r1 == 5) ? "OK" : "FAIL");
}

/*
 * Test 14 : suivant dans une file vide
 */
void test_suivant_vide() {
	printf("\nTest suivant dans file vide\n");
	file_init();
	uint16_t res = file_suivant();
	printf("suivant() sur file vide: %d (attendu %d) -> %s\n",
		res, F_VIDE, res == F_VIDE ? "OK" : "FAIL");
}

/*
 * Test 15 : suivant avec une seule tache
 */
void test_suivant_un_element() {
	printf("\nTest suivant avec un seul element\n");
	file_init();
	file_ajoute(7);
	uint16_t r0 = file_suivant();
	uint16_t r1 = file_suivant();
	uint16_t r2 = file_suivant();
	printf("3 appels suivant() sur 1 element: %d %d %d (attendu 7 7 7) -> %s\n",
		r0, r1, r2,
		(r0 == 7 && r1 == 7 && r2 == 7) ? "OK" : "FAIL");
}

/*
 * Test 16 : suivant avec deux taches
 * File [2, 8] -> suivant() alterne 8, 2, 8, 2, ...
 */
void test_suivant_deux_elements() {
	printf("\nTest suivant avec deux elements\n");
	file_init();
	file_ajoute(2);
	file_ajoute(8);
	uint16_t r0 = file_suivant();
	uint16_t r1 = file_suivant();
	printf("suivant() avec 2 elements: %d %d (attendu 8 2) -> %s\n",
		r0, r1, (r0 == 8 && r1 == 2) ? "OK" : "FAIL");
}

/*
 * Test 17 : cycle complet (circularite)
 * File [0,1,2,3] _queue=3 -> suivant() retourne 1, 2, 3, 0, 1, ...
 * 4 suivant() consomment 1, 2, 3, 0 ; le 5eme retourne 1 (cycle).
 */
void test_cycle_complet() {
	printf("\nTest cycle complet (circularite)\n");
	file_init();
	file_ajoute(0);
	file_ajoute(1);
	file_ajoute(2);
	file_ajoute(3);
	file_suivant(); // 1
	file_suivant(); // 2
	file_suivant(); // 3
	file_suivant(); // 0
	uint16_t res = file_suivant(); // 1 (cycle)
	printf("apres 4 suivant() puis 1 de plus: %d (attendu 1) -> %s\n",
		res, res == 1 ? "OK" : "FAIL");
}

/*
 * Test 18 : vider completement puis remplir a nouveau
 * Apres re-remplissage : file [4, 5], _queue = 5
 * suivant() retourne 5, 4
 */
void test_vider_remplir() {
	printf("\nTest vider puis remplir\n");
	file_init();
	file_ajoute(1);
	file_ajoute(2);
	file_ajoute(3);
	file_retire(1);
	file_retire(2);
	file_retire(3);
	uint16_t vide = file_suivant();
	printf("apres tout vider: %d (attendu %d) -> %s\n",
		vide, F_VIDE, vide == F_VIDE ? "OK" : "FAIL");
	file_ajoute(4);
	file_ajoute(5);
	uint16_t r0 = file_suivant();
	uint16_t r1 = file_suivant();
	printf("apres re-remplissage: %d %d (attendu 5 4) -> %s\n",
		r0, r1, (r0 == 5 && r1 == 4) ? "OK" : "FAIL");
}

/*
 * Test du sujet (conserve)
 */
void test_suivant() {
	printf("\nTest du sujet\n");
	file_init();
	file_ajoute(3);
	printf("3 ajouté !\n");
	file_ajoute(5);
	printf("5 ajouté !\n");
	file_ajoute(1);
	printf("1 ajouté !\n");
	file_ajoute(0);
	printf("0 ajouté !\n");
	file_ajoute(2);
	printf("2 ajouté !\n");
	file_affiche_queue();
	file_affiche();

	printf("\n\n");
	file_suivant();
	printf("Suivant\n");
	file_affiche_queue();
	file_affiche();

	printf("\n\n");
	file_retire(0);
	printf("0 retiré !\n");
	file_affiche_queue();
	file_affiche();
	printf("6 ajouté !\n");
	file_ajoute(6);
	file_affiche_queue();
	file_affiche();
}

int main(void) {
	usart_init(115200);
	/*printf("Hello world\n");

	test_ajout_file_vide();
	test_ajout_presque_pleine();
	test_ajout_indice_invalide();
	test_ajout_doublon();

	test_retire_file_vide();
	test_retire_inexistante();
	test_retire_indice_invalide();
	test_retire_seul_element();
	test_retire_tete();
	test_retire_milieu();
	test_retire_queue();
	test_retire_deux_fois();

	test_cycle_ajout_retrait();

	test_suivant_vide();
	test_suivant_un_element();
	test_suivant_deux_elements();
	test_cycle_complet();
	test_vider_remplir();

	*/
	test_suivant();

	return 0;
}
