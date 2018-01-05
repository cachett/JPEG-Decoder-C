#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

#include <stdint.h>
#include <stdbool.h>


struct bitstream;

/* Ouvre le fichier et initialise la structure de bitstream */
extern struct bitstream *create_bitstream(const char *filename);

/* ferme le fichier et libère la mémoire */
extern void close_bitstream(struct bitstream *stream);

/*Légèrement modifié par rapport au sujet, on vérifie la bonne lecture du nb de bit dans la fonctione elle-même */
extern uint8_t read_bitstream(struct bitstream *stream,
                              uint8_t nb_bits,
                              uint32_t *dest,
                              bool discard_byte_stuffing);

extern void read_bitstream_verified(struct bitstream *stream,
                              uint8_t nb_bits,
                              uint32_t *dest,
                              bool discard_byte_stuffing);

/*Vide le buffer en insérant les bits dans la destination */
extern void utilisation_du_buffer(struct bitstream *stream, uint8_t *nb_bits, uint32_t *dest, uint8_t *nb_bits_lu);

/* Check si nous sommes à la fin du file */
extern bool end_of_bitstream(struct bitstream *stream);

/*La fonction skip_bitstream_until est implémentée mais non utilisée (dans jpeg_reader) car nous
n'avons pas jugé son apport suffisemment intéressant*/
extern void skip_bitstream_until(struct bitstream *stream, uint8_t byte);



#endif
