#ifndef __UPSAMPLING_H__
#define __UPSAMPLING_H__

#include "jpeg_reader.h"

/*La fonction upsampling permet de mettre à jour les valeurs de Cb et Cr en fonction du facteur de sous echantillonage
associé à coef_mcu_x et coef_mcu_y */
void upsampling(uint8_t *tab_idct, uint8_t horizontal, uint8_t vertical, uint8_t **tab_upsampling);

/* reconstitue le MCU dans le cas d'une taille différente de 8x8*/
void reconstitue_mcu(uint8_t **tab_concatene, uint8_t *tab_idct, uint8_t k, uint8_t coef_mcu_x, uint8_t coef_mcu_y);

/* écrit 8 pixels d'un bloc dans tab_concatene, puis saute 8 élements et ainsi de suite,
jusqu'a l'écriture entiere du bloc. Debut et fin permette de ne pas écrasé les données
écrites s'il y a eu un échantillonage vertical auparavant */
void intercalle_bloc(uint8_t **tab_concatene, uint8_t debut, uint8_t fin, uint8_t *tab_idct, uint8_t multiplicateur_valeur, uint8_t k);

/*la fonction "dédouble" toutes les valeurs du tableau (on passe de [1,2] à [1, 1, 2, 2]) */
uint8_t *upsampling_horizontal(uint8_t *tab_idct, uint8_t **tab_upsampling);

/* la fonction répète des séquences d'octets. La variable facteur peut à priori valoir 1 ou 2
facteur = 1 signifie le cas sous-echantillonage vertical ou horizontal donc répétition tous les 8 octets
facteur = 2 signifie le cas sous-échantillonage vertical et horizontal donc répétition tous les 16 octets */
void upsampling_vertical(uint8_t *tab_idct,uint8_t **tab_upsampling, uint8_t facteur);

/* retourne un tableau à 2 (image en noir et blanc) ou 6 (image en couleur) élements.
contenant les facteurs de sous-echantillonage hxv des composantes */
uint8_t *initialise_tab_sampling_factor(struct jpeg_desc *jdesc);

/* retourne un tableau à 1 (image noir et blanc) ou 3 (image en couleur) élements,
contenant les indices (ic) des composantes définis dans le frame Header */
uint8_t *initialise_tab_id(struct jpeg_desc *jdesc, uint8_t nb_components);

#endif
