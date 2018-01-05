
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "jpeg_reader.h"

/* affiche les élèments d'un tableau de type (int16_t) en héxadécimal */
void affiche_tableau(FILE *f, int16_t *tab){
  fprintf(f,"[");
  uint8_t i = 0;
  while (i < 63 ){
    fprintf(f,"%hx ", tab[i]);
    i++;
  }
  fprintf(f, "%hx]\n", tab[i]);

}

/* affiche les élèments d'un tableau de type (uint8_t) en héxadécimal*/
void affiche_tableau_idct(FILE *f, uint8_t *tab){
  fprintf(f,"[");
  uint8_t i = 0;
  while (i < 63 ){
    fprintf(f,"%x ", tab[i]);
    i++;
  }
  fprintf(f, "%x]\n", tab[i]);

}


/* retourne le nombre de MCU de l'image */
extern uint32_t calcul_nb_mcu(uint16_t size_x, uint16_t size_y, uint8_t *sampling_factor){
  uint32_t nb_mcu = 0;
  nb_mcu = (size_x % (8*sampling_factor[0]) == 0) ? size_x/(8*sampling_factor[0]) : size_x/(8*sampling_factor[0]) + 1;
  nb_mcu = (size_y % (8*sampling_factor[1]) == 0) ? nb_mcu * (size_y / (8*sampling_factor[1])) : nb_mcu * ((size_y / (8*sampling_factor[1])) + 1);
  return nb_mcu;
}

extern void affiche_info_entete(struct jpeg_desc *jdesc){
    printf("[SOI]	marker found\n");
    printf("[APP0] JFIF application verified\n");
    printf("[DQT] nb of quantization table: %u\n", get_nb_quantization_tables(jdesc));
    printf("[SOF0] image height: %u\n", get_image_size(jdesc, DIR_V));
    printf("       image weight: %u\n", get_image_size(jdesc, DIR_H));
    uint8_t nb_components = get_nb_components(jdesc);
    printf("       nb of components: %u\n", nb_components);
    printf("       component Y\n");
    printf("              id %u\n", get_frame_component_id(jdesc, COMP_Y));
    printf("              sampling factors (hxv) %ux%u\n",
    get_frame_component_sampling_factor(jdesc, DIR_H, COMP_Y), get_frame_component_sampling_factor(jdesc, DIR_V, COMP_Y));
    if (nb_components == 3){
      printf("       component Cb\n");
      printf("              id %u\n", get_frame_component_id(jdesc, COMP_Cb));
      printf("              sampling factors (hxv) %ux%u\n",
      get_frame_component_sampling_factor(jdesc, DIR_H, COMP_Cb), get_frame_component_sampling_factor(jdesc, DIR_V, COMP_Cb));
      printf("       component Cr\n");
      printf("              id %u\n", get_frame_component_id(jdesc, COMP_Cr));
      printf("              sampling factors (hxv) %ux%u\n",
      get_frame_component_sampling_factor(jdesc, DIR_H, COMP_Cr), get_frame_component_sampling_factor(jdesc, DIR_V, COMP_Cr));
    }
    printf("[DHT] nb of quantification_table DC: %u\n", get_nb_huffman_tables(jdesc, DC));
    printf("      nb of quantification_table AC: %u\n", get_nb_huffman_tables(jdesc, AC));
    printf("[SOS] infos non communiquée\n");
    printf("*** STOPPED SCAN, bitstream at the beginning of Scan raw compressed data ***\n");
}
