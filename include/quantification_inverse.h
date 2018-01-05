#ifndef __QUANT_INV_H__
#define __QUANT_INV_H__

#include <stdint.h>
#include <stdbool.h>
#include "jpeg_reader.h"
#include "bitstream.h"
#include "huffman.h"
#include "utilitaire.h"


/* retourne un pointeur de type (int16_t) vers un tableau contenant les élèments en sortie
de la décompression, multipliés par les élèments de la table de
quantification correspondante à la composante lue */

extern int16_t *quantification_inverse(struct jpeg_desc *jdesc, int16_t *tab, uint8_t frame_comp_index);

#endif
