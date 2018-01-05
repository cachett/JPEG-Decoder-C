#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "bitstream.h"


/* construction et initialisation d'un tableau de pointeur vers les blocs des trois composantes*/

extern void build_tabYCbCr(uint8_t *new_tab,uint8_t nb_components, uint8_t frame_comp_index, uint8_t ***tabYCbCr){
  // initilisation
  if (*tabYCbCr == NULL){
     *tabYCbCr = calloc(3, sizeof(uint8_t *));
  }
  // cas d'une image noir et blanc, seul le premier élèment du tableau n'est pas vide
  if (nb_components == 1){
    (*tabYCbCr)[0] = new_tab;
  }
  else if (nb_components == 3){
    // frame_comp_index est mis a jour grâce a maj_frame_comp_index
    // et représente la ième composante lu (Y, Cb ou Cr)
    (*tabYCbCr)[frame_comp_index] = new_tab;
  }
  else{
    fprintf(stderr, "Oulah, il y a %u composantes, c'est bizarre...", nb_components);
    exit(0);
  }
}


/* retourne un pointeur de type (uint32_t) vers un tableau de taille 64 contenant
les élèments entièrement décodés : la fonction réalise la
dernière étape du décodage, en mettant les pixels du bloc sous forme RGB*/

extern uint32_t *ycbcr_to_rgb(uint8_t **tab, uint8_t coef_mcu_x, uint8_t coef_mcu_y){
  /* sous le format RGB, les 8 bits de poids fort valent 0,
   les 8 suivants correspondent au rouge, puis au vert,
   et les 8 bits de poids faibles correspondent au bleu */
  uint16_t j = 0;
  uint32_t *tab_finale = calloc(coef_mcu_x * coef_mcu_y << 6, sizeof(uint32_t));
  assert(tab_finale != NULL && "Pb allocation mémoire !\n");
  if (tab[1] == NULL){
    // l'image est en noir et blanc, car il n'y a pas de composante Cb
    while (j < coef_mcu_x * coef_mcu_y << 6){
      uint32_t gris = tab[0][j];
      tab_finale[j] = gris;
      j++;
      }
    }
  else{
    // il y a les composantes Cb et Cr (l'image est en couleur), on utilise donc toutes
    // les cases du tableau passé en parametre
    while (j< coef_mcu_x * coef_mcu_y << 6){
       float red = ((float) tab[0][j]) + 1.402 * ((float)(tab[2][j] - 128));
       // les valeurs doivent être clampées entre  0 et 255
       red = (red < 0) ? 0 : red;
       red = (red > 255) ? 255 : red;
       float green = ((float) tab[0][j]) - 0.34414*((float) (tab[1][j] - 128)) - 0.71414*((float) (tab[2][j] - 128));
       green = (green < 0) ? 0 : green;
       green = (green > 255) ? 255 : green;
       float blue = ((float) tab[0][j]) + 1.772*((float) (tab[1][j] - 128));
       blue = (blue < 0) ? 0 : blue;
       blue = (blue > 255) ? 255 : blue;
       tab_finale[j] = (((uint32_t)red) << 16) | ( ((uint32_t) green) << 8) | ((uint32_t) blue);
       j++;
     }
  }
  return tab_finale;
}

/* libération de la mémoire allouée */
extern void free_tabYCrCb(uint8_t **tabYCbCr){
  free(tabYCbCr[0]);
  free(tabYCbCr[1]);
  free(tabYCbCr[2]);
  free(tabYCbCr);
}
