#ifndef __YCBCRTORGB_H__
#define __YCBCRTORGB_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "bitstream.h"


/* construction et initialisation d'un tableau de pointeur vers les MCU des trois composantes*/
extern void build_tabYCbCr(uint8_t *new_tab,uint8_t nb_components, uint8_t frame_comp_index, uint8_t ***tabYCbCr);

/* retourne un pointeur de type (uint32_t) vers un tableau de taille 64 contenant
les élèments entièrement décodés : la fonction réalise la
dernière étape du décodage, en mettant les pixels du bloc sous forme RGB*/
extern uint32_t *ycbcr_to_rgb(uint8_t **tab, uint8_t coef_mcu_x, uint8_t coef_mcu_y);


/* libération de la mémoire allouée */
extern void free_tabYCrCb(uint8_t **tabYCbCr);

#endif
