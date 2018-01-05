#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "bitstream.h"
#include "assert.h"


struct bitstream{
  FILE *file;
  uint8_t buffer;
  uint8_t bit_in_buffer;
  bool prec_FF;
};

/* Ouvre le fichier et initialise la structure de bitstream */
extern struct bitstream *create_bitstream(const char *filename){
  FILE *f = fopen(filename, "rb");
  struct bitstream *bitstream = calloc(1, sizeof(struct bitstream));
  if (f == NULL || bitstream == NULL){
    fprintf(stderr, "Erreur lors de la création du bitstream\nImage non trouvée?!\n");
    exit(0);
  }
  bitstream->prec_FF = false;
  bitstream->file = f;
  return bitstream;
}

/* ferme le fichier et libère la mémoire */
extern void close_bitstream(struct bitstream *stream){
  uint8_t test = fclose(stream->file);
  if (test != 0){
    fprintf(stderr, "Erreur lors de la fermeture du fichier\n");
  }
  free(stream);
}


/*Légèrement modifié par rapport au sujet, on vérifie la bonne lecture du nb de bit dans la fonctione elle-même */
extern uint8_t read_bitstream(struct bitstream *stream, uint8_t nb_bits, uint32_t *dest, bool discard_byte_stuffing){
  uint8_t test_nb_bits = nb_bits;
  uint8_t nb_bits_lu = 0;
  uint8_t test_fread = 0;
  *dest = 0;
  /* Dans un premier temps, on check si il reste des bits dans le buffer */
  utilisation_du_buffer(stream, &nb_bits, dest, &nb_bits_lu);
  /* Sinon, on remplis le buffer et on le traite à nouveau */
  while (nb_bits > 0){
    test_fread = fread(&stream->buffer, 1, 1, stream->file);
    assert(test_fread == 1 && "Nombre bytes lus par fread faux\n");
    if (stream->prec_FF && discard_byte_stuffing && stream->buffer == 0){
      /* On discard le byte stuffing */
      test_fread = fread(&stream->buffer, 1, 1, stream->file);
      assert(test_fread == 1 && "Nombre bytes lus par fread faux\n");
    }
    stream->prec_FF = (stream->buffer == 0xFF)? true : false;
    stream->bit_in_buffer = 8;
    utilisation_du_buffer(stream, &nb_bits, dest, &nb_bits_lu);
  }
  if (nb_bits_lu != test_nb_bits){
    fprintf(stderr, "Ouulaah, on a voulu lire %u bits et on en a lu %u\n", nb_bits, nb_bits_lu);
    exit(0);
  }
  return nb_bits_lu;
}


/*Vide le buffer en insérant les bits dans la destination */
extern void utilisation_du_buffer(struct bitstream *stream, uint8_t *nb_bits, uint32_t *dest, uint8_t *nb_bits_lu){
  while (*nb_bits > 0 && stream->bit_in_buffer > 0){
    /*Sélection du derneir bit de poids fort dans le buffer */
    uint8_t tmp = 1;
    tmp = tmp << (stream->bit_in_buffer - 1);
    tmp = stream->buffer & tmp;
    stream->buffer -= tmp;
    tmp = tmp >>(stream->bit_in_buffer - 1);
    /* On le met dans dest et on incrémente/décremente les compteurs */
    *dest = (*dest << 1) + tmp;
    *nb_bits -= 1;
    stream->bit_in_buffer--;
    *nb_bits_lu += 1;
  }
}

/*La fonction skip_bitstream_until est implémentée mais non utilisée (dans jpeg_reader) car nous
n'avons pas jugé son apport suffisemment intéressant*/

extern void skip_bitstream_until(struct bitstream *stream, uint8_t byte){
  /*Place le pointeur de fichier juste avant la première occurrence du byte "byte"*/
  uint8_t test_fread = 0;
  do{
    test_fread = fread(&stream->buffer, 1, 1, stream->file);
    assert(test_fread == 1 && "Nombre bytes lus par fread faux\n");
  }while (stream->buffer != byte);
  stream->bit_in_buffer = 8;
}

/* Check si nous sommes à la fin du file */
extern bool end_of_bitstream(struct bitstream *stream){
  if (feof(stream->file)){
    return true;
  }
  return false;
}
