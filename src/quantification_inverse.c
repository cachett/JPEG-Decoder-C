#include <stdlib.h>
#include <stdio.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "huffman.h"



/* retourne un pointeur de type (int16_t) vers un tableau contenant les élèments en sortie
de la décompression, multipliés par les élèments de la table de
quantification correspondante à la composante lue */

extern int16_t *quantification_inverse(struct jpeg_desc *jdesc, int16_t *tab, uint8_t frame_comp_index){
  // récupération de l'index de la table de quantification associée à la composante lue
  uint8_t quant_index = get_frame_component_quant_index (jdesc , frame_comp_index);
  // récupération de la table
  uint8_t *table = get_quantization_table(jdesc, quant_index);
  uint32_t i = 0;
  while (i < 64){
    tab[i] = tab[i] * table[i];
    i++;
    }
  return tab;
}
