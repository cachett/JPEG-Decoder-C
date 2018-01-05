#ifndef __HUFFMAN_H__
#define __HUFFMAN_H__

#include <stdint.h>
#include <stdbool.h>
#include "bitstream.h"


struct huff_table;

/* fonction d'affichage de l'arbre construit pour effectuer du debeugage (inutilisée) */
void affiche_arbre(struct huff_table *table, char *path);


/* constuit la table de huffman (sous forme d'arbre) propre a l'image en cours de décodage,
et met a jour le nombre de byte lu pendant la création de l'arbre  */
extern struct huff_table *load_huffman_table(struct bitstream *stream,
                                             uint16_t *nb_byte_read);

/* retourne la prochaine valeur atteinte en parcourant la table de Huffman table
selon les bits extraits du flux stream . */
extern int8_t next_huffman_value(struct huff_table *table,
                                 struct bitstream *stream);

/* libération de la mémoire allouée pour la création de la table de huffman */
extern void free_huffman_table(struct huff_table *table);




#endif
