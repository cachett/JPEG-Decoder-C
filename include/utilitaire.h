#ifndef __UTILITAIRE_H__
#define __UTILITAIRE_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


/* retourne le nombre de MCU de l'image */
extern uint32_t calcul_nb_mcu(uint16_t size_x, uint16_t size_y, uint8_t *sampling_factor);

/* affiche les élèments d'un tableau de type (int16_t) en héxadécimal */
void affiche_tableau(FILE *f, int16_t *tab);

/* affiche les élèments d'un tableau de type (uint8_t) en héxadécimal*/
void affiche_tableau_idct(FILE *f,uint8_t *tab);

/* s'occupe de l'affichage du mode verbose */
extern void affiche_info_entete(struct jpeg_desc *jdesc);
#endif
