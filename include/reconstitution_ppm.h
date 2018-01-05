#ifndef __RECONSTITUTION_H__
#define __RECONSTITUTION_H__


#include "jpeg_reader.h"

/* Initialisation de l'image ppm ou pgm et  écriture de l'entête textuel */
extern char *initialise_picture(char *filename, uint16_t size_x, uint16_t size_y, uint8_t nb_components);

/* chargement a la bonne place dans le tableau "tab_image" des pixels décodés.
Ainsi, les pixels dans tab_image sont dans le même ordre que pour l'image réelle */
extern void set_current_pixels(uint32_t *tab_decode, uint32_t **tab_image, uint32_t index, uint16_t size_x, uint16_t size_y, uint8_t coef_mcu_x, uint8_t coef_mcu_y);


/* écriture des élements du tableau en binaire dans le fichier ppm*/
extern void write_picture_pgm(char *filename, uint32_t *tab_image, uint16_t size_x, uint16_t size_y);

/* écriture des élements du tableau en binaire dans le fichier pgm */
extern void write_picture_ppm(char *filename, uint32_t *tab_image, uint16_t size_x, uint16_t size_y);

#endif
