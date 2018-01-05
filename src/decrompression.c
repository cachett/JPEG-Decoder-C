#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "huffman.h"
#include "decrompression.h"

/* renvoie pour une certaine composante d'un certain bloc, un pointeur de type (int16_t)
vers un tableau contenant les valeurs décodées */

extern int16_t *reverse_huffman(struct jpeg_desc *jdesc, struct bitstream *stream, int16_t **predicateur, uint8_t frame_comp_index){
    //initialisation des variables
    int16_t *tab_reversed_huffman = calloc(64, sizeof(int16_t));
    assert(tab_reversed_huffman != NULL && "Pb allocation mémoire !\n");
    uint32_t i = 0;
    // mise à 1 de valeur_octet pour rentrer dans la boucle
    int8_t valeur_octet = 1;
    struct huff_table *current_table;
    // récupération des indices des tables de huffman correspondant au bloc et a la composante lu
    uint8_t index_huffman_table_dc = get_scan_component_huffman_index(jdesc, DC, frame_comp_index);
    uint8_t index_huffman_table_ac = get_scan_component_huffman_index(jdesc, AC, frame_comp_index);

    while (valeur_octet != 0 && i<64){
      //utilisation de la table DC pour le premier élement
      if (i == 0){
        current_table = get_huffman_table(jdesc, DC, index_huffman_table_dc);
        int8_t magnitude = next_huffman_value(current_table,  stream);
        tab_reversed_huffman[i] = decode_magnitude(magnitude, stream) + (*predicateur)[frame_comp_index];
        (*predicateur)[frame_comp_index] = tab_reversed_huffman[i];
      }

      else{
        current_table = get_huffman_table(jdesc, AC, index_huffman_table_ac);
        valeur_octet = next_huffman_value(current_table, stream);
        if (valeur_octet != 0){
          tab_reversed_huffman[i] = valeur_decodee_huffman_AC(valeur_octet, stream, &i);
        }
      }
      i++;
    }
    return tab_reversed_huffman;
}


/* retourne la valeur décodée d'un octet codé par
 les algorithmes de Huffman et de RLE */

extern int16_t valeur_decodee_huffman_AC(int8_t valeur_octet, struct bitstream *stream, uint32_t *index){
  // dans le cas de 16 composantes nulles
  if ((uint8_t) valeur_octet == 0xf0){
    // on "saute" 15 cases, qui sont initialisées à 0 grâce au calloc
    *index += 15;
    // et on retourne 0 pour mettre la 16ème à 0
    return 0;
  }
  // les 4 bits de poids faibles de valeur_octet codent la classe de magnitude
  uint8_t magnitude = (uint8_t) (valeur_octet & 0x0f) ;
  if (magnitude == 0 ){
    printf("symbole invalide (interdit !) \n");
    exit(0);
  }
  // les 4 bits de poids fort de valeur_octet correspondent au nombre de coefficient zéro
  uint8_t nbr_zero = (uint8_t) valeur_octet >> 4;
  *index += nbr_zero;
  // dans tout les autres cas, on utilise l'arbre DC pour retourner la valeur décodée
  return decode_magnitude(magnitude, stream);
}



/* retourne la valeur DC différentielle à partir de la classe de magnitude*/

extern int16_t decode_magnitude(uint8_t magnitude, struct bitstream *stream){
  /*byte contient les "magnitude" bits lus dans le flux de bits */
  uint32_t byte = 0;
  read_bitstream(stream, magnitude, &byte, true);
  uint8_t premier_bit = byte >> (magnitude - 1) ;
  if (premier_bit == 1){
    return byte;
  }else{
    return (-(1<<magnitude) + 1 + byte);
    }
}


/* retourne un nouvel indice frame_comp_index dans le cas ou l'ordre des composantes n'est pas YCbCr */

extern uint8_t maj_frame_comp_index(uint8_t identifiant_comp_scan ,uint8_t *frame_comp_index, uint8_t *identifiant_composantes){
  // si l'identifiant correspond bien à l'indice traité, on ne change rien car on lit dans le bon ordre
  if (identifiant_comp_scan == *frame_comp_index - 1){
    return *frame_comp_index;
  }else{
    // sinon on cherche l'indice de la composante qui a le même identifiant dans le frame Header
    for (uint8_t i = 0; i < 3; i++){
      if (identifiant_comp_scan == identifiant_composantes[i]){
        *frame_comp_index = i;
        return *frame_comp_index;
        }
      }
    }
}
