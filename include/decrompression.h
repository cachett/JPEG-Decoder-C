#ifndef __DECOMPRESSION_H__
#define __DECOMPRESSION_H__


#include <stdlib.h>
#include <stdio.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "huffman.h"


/* renvoie pour une certaine composante d'un certain bloc, un pointeur de type (int16_t) vers un tableau contenant les valeurs décodées */
extern int16_t *reverse_huffman(struct jpeg_desc *jdesc, struct bitstream *stream, int16_t **predicateur, uint8_t frame_comp_index);

/* retourne la valeur décodée d'un octet codé par les algorithmes de Huffman et de RLE */
extern int16_t valeur_decodee_huffman_AC(int8_t valeur_octet, struct bitstream *stream, uint32_t *index);

/* retourne la valeur DC différentielle à partir de la classe de magnitude*/
extern int16_t decode_magnitude(uint8_t magnitude, struct bitstream *stream);

/* returne un nouvel indice frame_comp_index dans le cas ou l'ordre des composantes n'est pas YCbCr */
extern uint8_t maj_frame_comp_index(uint8_t identifiant_comp_scan ,uint8_t *frame_comp_index, uint8_t *identifiant_composantes);

#endif
