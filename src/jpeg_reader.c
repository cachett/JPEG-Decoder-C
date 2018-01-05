#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "huffman.h"



struct jpeg_desc{
    struct huff_table ***huffman_table;
    uint8_t nb_table_dqt;
    uint8_t **quantification_table;
    uint16_t height;
    uint16_t weight;
    uint8_t nb_components;
    uint8_t *nb_huffman;
    struct bitstream *stream;
    struct sos_infos *sos_tab;
    struct sampling_infos *sampling_tab;
    char *filename;

};


struct sampling_infos{
    uint8_t ic;
    uint8_t sampling_H;
    uint8_t sampling_V;
    uint8_t iq;
};

struct sos_infos{
    uint8_t ic;
    uint8_t ih_DC;
    uint8_t ih_AC;
};

/* lit tous les entêtes de sections rencontrés et stocke les in-
formations requises dans la structure associée */

extern struct jpeg_desc *read_jpeg(const char *filename){
  struct huff_table ***huffman_table = calloc(2, sizeof(struct huff_table **));
  huffman_table[0] = calloc(4, sizeof(struct huff_table *));
  huffman_table[1] = calloc(4, sizeof(struct huff_table *));
  struct jpeg_desc *jdesc = malloc(sizeof(struct jpeg_desc));
  assert(jdesc != NULL && huffman_table[0] != NULL && huffman_table[1] != NULL &&
                                huffman_table != NULL && "Pb allocation mémoire\n");
  jdesc->filename = (char *) filename;
  struct bitstream *stream;
  stream = create_bitstream (filename);
  if(stream == NULL){
    printf("Erreur de chargement du fichier...\n");
    exit(0);
  }
  jdesc->stream = stream;
  uint32_t byte = 0;
  uint8_t nb_read = 0;
  uint32_t longeur_section = 0;
  uint32_t nb_components = 0;
  uint32_t longueur_section = 0;
  uint8_t **quantification_table = calloc(4, sizeof(uint8_t *));
  assert(quantification_table != NULL && "Pb allocation mémoire !\n");
  uint8_t nb_table_dqt = 0;
  uint32_t indice_quantification = 0;
  uint32_t indice_ac_dc = 0;
  uint32_t indice_table_huffman = 0;
  uint8_t *nb_huffman = calloc(2, sizeof(uint8_t));
  assert(nb_huffman != NULL && "Pb allocation mémoire !\n");
  uint16_t nb_byte_read = 0;
  uint8_t numero_sof;
  uint32_t longueur_section_huffman = 0;

  bool fin_header = false;
  while (!fin_header){
    nb_read = read_bitstream(stream, 8, &byte, false);
    if (nb_read != 8){
      printf("Oulah, on voulais lire 8 bits et on en a lu %u\n", nb_read);
      exit(0);
    }
    if (byte == 0xff ){
      read_bitstream(stream, 8, &byte, false);
      switch(byte){
        case 0xc1:
        case 0xc2:
        case 0xc3:
        case 0xc5:
        case 0xc6:
        case 0xc7:
        case 0xc9:
        case 0xca:
        case 0xcb:
        case 0xcd:
        case 0xce:
        case 0xcf:
        /* Ces types d'encodage ne sont pas traités */
          numero_sof = 0xf & byte;
          fprintf(stderr, "ERROR DCT mode SOF%u is not handled by this decoder (baseline DCT SOF0 only)\n", numero_sof);
          exit(0);
          break;
        case 0xd8:
          /*SOI (start of image)*/
          continue;
          break;
        case 0xe0:
        /* Vérification de l'encodage de la phrase "JFIF" */
          read_bitstream(stream, 16, &byte, false);
          read_bitstream(stream, 8, &byte, false);
          if ( byte - 'J' != 0){
            printf("Erreur encodage APPx (JFIF)\n");
            exit(0);}
          read_bitstream(stream, 8, &byte, false);
          if ( byte - 'F' != 0){
            printf("Erreur encodage APPx (JFIF)\n");
            exit(0);}
          read_bitstream(stream, 8, &byte, false);
          if ( byte - 'I' != 0){
            printf("Erreur encodage APPx (JFIF)\n");
            exit(0);}
          read_bitstream(stream, 8, &byte, false);
          if (byte - 'F' != 0){
            printf("Erreur encodage APPx (JFIF)\n");
            exit(0);}
          read_bitstream(stream, 8, &byte, false);
          if (byte - '\0' != 0){
            printf("Erreur encodage APPx (JFIF)\n");
            exit(0);}
          break;

        case 0xdb:
         /* DQT table de quantification */
           nb_read = read_bitstream(stream, 16, &longueur_section, false);
           if (longueur_section % 65 != 2){
             printf("C'est étrange la longueur de la section n'est pas multiple de 67 octets elle vaut %u \n", longueur_section);
           }
           uint8_t nb_table = longueur_section / 65;
           nb_table_dqt += nb_table;
           for (uint8_t i = 0; i < nb_table; i++){
             read_bitstream(stream, 4, &byte, false);
             read_bitstream(stream, 4, &indice_quantification, false);
             quantification_table[indice_quantification] = malloc(sizeof(uint8_t)<<6);
             assert(quantification_table[indice_quantification] != NULL && "Pb allocation mémoire !\n");
             for (uint8_t j = 0; j < 64; j++){
               read_bitstream(stream, 8, &byte, false);
               quantification_table[indice_quantification][j] = byte;
             }
           }
           break;

        case 0xc0:
        /* marqueur SOFx, facteur d'échantillonage */
          read_bitstream(stream, 24, &byte, false);
          read_bitstream(stream, 16, &byte, false);
          jdesc->height = byte;
          read_bitstream(stream, 16, &byte, false);
          jdesc->weight = byte;
          read_bitstream(stream, 8, &nb_components, false);
          jdesc->nb_components = nb_components;
          jdesc->sampling_tab = malloc(nb_components*sizeof(struct sampling_infos));
          assert(jdesc->sampling_tab != NULL && "Pb allocation mémoire !\n");
          for (uint8_t k = 0; k < nb_components; k++){
            read_bitstream(stream, 8, &byte, false);
            jdesc->sampling_tab[k].ic = byte;
            read_bitstream(stream, 4, &byte, false);
            jdesc->sampling_tab[k].sampling_H = byte;
            read_bitstream(stream, 4, &byte, false);
            jdesc->sampling_tab[k].sampling_V = byte;
            read_bitstream(stream, 8, &byte, false);
            jdesc->sampling_tab[k].iq = byte;
          }
          break;

        case 0xc4:
        /* Tables de huffman ! */
           read_bitstream(stream, 16, &longueur_section_huffman, false);
           read_bitstream(stream, 3, &byte, false);
           if (byte != 0){
             printf("Erreur : Le résultat devrait valoir 0 et il vaut : %u \n", byte);
           }
           read_bitstream(stream, 1, &indice_ac_dc, false);
           if (indice_ac_dc == 0){
             nb_huffman[0]++;
           }else{
             nb_huffman[1]++;
           }
           read_bitstream(stream, 4, &indice_table_huffman, false);
           struct huff_table *table_huffman = load_huffman_table(stream, &nb_byte_read);
           //Cas ou load_huffman_table s'est mal passé
           if (table_huffman == NULL){
             perror("ERREUR lors de la section DHT \n");
             exit(EXIT_FAILURE);
           }
           //Cas de plusieurs table de huffman au sein d'une section DHT
           if (longueur_section_huffman != nb_byte_read + 3){
             perror("ERREUR : Nous ne traitons pas le cas de plusieurs table de huffman au sein d'un marqueur DHT \n");
             exit(EXIT_FAILURE);
           }
           huffman_table[indice_ac_dc][indice_table_huffman] = table_huffman;
           break;

        case 0xda:
        /* SOS, association table huffman/composante */
          read_bitstream(stream, 16, &byte, false);
          read_bitstream(stream, 8, &nb_components, false);
          jdesc->sos_tab = malloc(nb_components*sizeof(struct sos_infos));
          assert(jdesc->sos_tab != NULL && "Pb allocation mémoire !\n");
          for (uint8_t k = 0; k < nb_components; k++){
            read_bitstream(stream, 8, &byte, false);
            jdesc->sos_tab[k].ic = byte;
            read_bitstream(stream, 4, &byte, false);
            jdesc->sos_tab[k].ih_DC = byte;
            read_bitstream(stream, 4, &byte, false);
            jdesc->sos_tab[k].ih_AC = byte;

          }
          read_bitstream(stream, 24, &byte, false); //osef des 3 bytes de fin
          fin_header = true; //fin du while, fin du header
          break;

        default:
          printf("Les infos de la section de marqueur %x ne sont pas traitées par ce décodeur\n", byte);
          continue;
      }
    }
  }
  jdesc -> nb_table_dqt = nb_table_dqt;
  jdesc -> quantification_table = quantification_table;
  jdesc -> huffman_table = huffman_table;
  jdesc -> nb_huffman = nb_huffman;
  return jdesc;
}



/* libère la mémoire de jdesc */

extern void close_jpeg(struct jpeg_desc *jpeg){
  for (uint8_t i = 0; i < 2 ; i++){
    for (uint8_t j = 0; j < 4; j++){
      free_huffman_table(jpeg->huffman_table[i][j]);
    }
    free(jpeg->huffman_table[i]);
  }
  free(jpeg-> huffman_table);
  for (uint8_t k = 0; k < 4; k++){
    free(jpeg->quantification_table[k]);
  }
  free(jpeg->quantification_table);
  free(jpeg->sos_tab);
  free(jpeg->sampling_tab);
  free(jpeg->nb_huffman);
  close_bitstream(jpeg->stream);
  free(jpeg);
}

extern char *get_filename(const struct jpeg_desc *jpeg){
  return jpeg->filename;
}

// access to stream, placed just at the beginning of the scan raw data
extern struct bitstream *get_bitstream(const struct jpeg_desc *jpeg){
  return jpeg->stream;
}

// from DQT
extern uint8_t get_nb_quantization_tables(const struct jpeg_desc *jpeg){
  return jpeg->nb_table_dqt;
}

extern uint8_t *get_quantization_table(const struct jpeg_desc *jpeg,
                                       uint8_t index){
  return jpeg->quantification_table[index];
}

// from DHT
extern uint8_t get_nb_huffman_tables(const struct jpeg_desc *jpeg,
                                     enum acdc acdc){

  return jpeg->nb_huffman[acdc];
}

extern struct huff_table *get_huffman_table(const struct jpeg_desc *jpeg,
                                            enum acdc acdc, uint8_t index){
  return jpeg->huffman_table[acdc][index];
}



// from Frame Header SOF0
extern uint16_t get_image_size(struct jpeg_desc *jpeg, enum direction dir){
  if (dir == DIR_V){
    return jpeg->height;
  }
  return jpeg->weight;
}

extern uint8_t get_nb_components(const struct jpeg_desc *jpeg){
  return jpeg->nb_components;
}

extern uint8_t get_frame_component_id(const struct jpeg_desc *jpeg,
                                      uint8_t frame_comp_index){
  return jpeg->sampling_tab[frame_comp_index].ic;
}

extern uint8_t get_frame_component_sampling_factor(const struct jpeg_desc *jpeg,
                                                   enum direction dir,
                                                   uint8_t frame_comp_index){
  if (dir == DIR_V){
    return jpeg->sampling_tab[frame_comp_index].sampling_V;
  }
  return jpeg->sampling_tab[frame_comp_index].sampling_H;
}

extern uint8_t get_frame_component_quant_index(const struct jpeg_desc *jpeg,
                                               uint8_t frame_comp_index){
  return jpeg->sampling_tab[frame_comp_index].iq;
}

// from Scan Header SOS
extern uint8_t get_scan_component_id(const struct jpeg_desc *jpeg,
                                     uint8_t scan_comp_index){

  return jpeg->sos_tab[scan_comp_index].ic;
}

extern uint8_t get_scan_component_huffman_index(const struct jpeg_desc *jpeg,
                                                enum acdc acdc,
                                                uint8_t scan_comp_index){
  if (acdc == DC){
    return jpeg->sos_tab[scan_comp_index].ih_DC;
  }
  return jpeg->sos_tab[scan_comp_index].ih_AC;
}
