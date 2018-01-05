#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "upsampling.h"
#include "utilitaire.h"


/* retourne un tableau à 2 (image en noir et blanc) ou 6 (image en couleur) élements.
contenant les facteurs de sous-echantillonage hxv des composantes */

uint8_t *initialise_tab_sampling_factor(struct jpeg_desc *jdesc){
  uint8_t nb_components = get_nb_components(jdesc);
  uint8_t *tab_sampling_factor;
  if (nb_components == 1){
    tab_sampling_factor = malloc(sizeof(uint8_t) << 2);
    assert(tab_sampling_factor!= NULL && "Pb allocation mémoire !\n");
  }else{
    tab_sampling_factor = malloc(6*sizeof(uint8_t));
    assert(tab_sampling_factor != NULL && "Pb allocation mémoire !\n");
    tab_sampling_factor[2] = get_frame_component_sampling_factor(jdesc, DIR_H, COMP_Cb);
    tab_sampling_factor[3] = get_frame_component_sampling_factor(jdesc, DIR_V, COMP_Cb);
    tab_sampling_factor[4] = get_frame_component_sampling_factor(jdesc, DIR_H, COMP_Cr);
    tab_sampling_factor[5] = get_frame_component_sampling_factor(jdesc, DIR_V, COMP_Cr);
    if (tab_sampling_factor[2]*tab_sampling_factor[3] != 1){
      perror("ERREUR : cas plusieurs blocs Cb au sein d'un MCU non traité par le programme \n");
      exit(EXIT_FAILURE);
      }
    if (tab_sampling_factor[4]*tab_sampling_factor[5] != 1){
      perror("ERREUR : cas plusieurs blocs Cr au sein d'un MCU non traité par le programme \n");
      exit(EXIT_FAILURE);
      }
    }
  tab_sampling_factor[0] = get_frame_component_sampling_factor(jdesc, DIR_H, COMP_Y);
  tab_sampling_factor[1] = get_frame_component_sampling_factor(jdesc, DIR_V, COMP_Y);
  if (!((tab_sampling_factor[0] == 1 || tab_sampling_factor[0] == 2) && (tab_sampling_factor[1] == 1 || tab_sampling_factor[1] == 2))){
    perror("ERREUR : le sous échantillonage de l'image n'est pas traité dans le programme \n");
    exit(EXIT_FAILURE);
    }
  return tab_sampling_factor;
}


/*La fonction upsampling permet de mettre à jour les valeurs de Cb et Cr en fonction du facteur de sous echantillonage
associé à coef_mcu_x et coef_mcu_y */

void upsampling(uint8_t *tab_idct, uint8_t coef_mcu_x, uint8_t coef_mcu_y, uint8_t **tab_upsampling){
  //sous-echantillonage horizontal
  if (coef_mcu_x == 2 && coef_mcu_y == 2){
    uint8_t *tab_to_free = NULL;
    //on récupère tab_to_free uniquement pour ne pas perdre son pointeur et pouvoir le free par la suite
    tab_to_free = upsampling_horizontal(tab_idct, tab_upsampling);
    upsampling_vertical(*tab_upsampling, tab_upsampling, 2);
    free(tab_to_free);
  //sous-échantillonage horizontal
  }else if (coef_mcu_x == 2){
     upsampling_horizontal(tab_idct, tab_upsampling);
  //sous-echantillonage vertical
  }else if (coef_mcu_y == 2){
     upsampling_vertical(tab_idct, tab_upsampling, 1);
  //pas de sous-échantillonage
  }else{
      (*tab_upsampling) = tab_idct;
  }
}

/*la fonction "dédouble" toutes les valeurs du tableau (on passe de [1,2] à [1, 1, 2, 2]) */

uint8_t *upsampling_horizontal(uint8_t *tab_idct, uint8_t **tab_upsampling){
  uint8_t *tableau_temporaire = malloc(sizeof(uint8_t) << 7);
  assert(tableau_temporaire != NULL && "Pb allocation mémoire !\n");
  for (uint8_t i = 0; i < 64; i++){
    tableau_temporaire[2*i] = tab_idct[i];
    tableau_temporaire[2*i + 1] = tab_idct[i];
  }
  *tab_upsampling = tableau_temporaire;
  return tableau_temporaire;
}

/* la fonction répète des séquences d'octets. La variable facteur peut à priori valoir 1 ou 2
facteur = 1 signifie le cas sous-echantillonage vertical ou horizontal donc répétition tous les 8 octets
facteur = 2 signifie le cas sous-échantillonage vertical et horizontal donc répétition tous les 16 octets */

void upsampling_vertical(uint8_t *tab_idct, uint8_t **tab_upsampling, uint8_t facteur){
  uint8_t multiplicateur = 0;
  uint8_t compteur = 1;
  uint8_t *tableau_temporaire = malloc(facteur * sizeof(uint8_t) << 7);
  assert(tableau_temporaire != NULL && "Pb allocation mémoire !\n");
  for (uint8_t j=0; j < facteur << 6; j++){
    tableau_temporaire[j + multiplicateur*(facteur << 3)] = tab_idct[j];
    tableau_temporaire[j + multiplicateur*(facteur << 3) + (facteur << 3)] = tab_idct[j];
    if (compteur % (facteur << 3) == 0 && compteur != 0){
      multiplicateur++;
      }
    compteur++;
    }
  *tab_upsampling = tableau_temporaire;
}


/* reconstitue le MCU dans le cas d'une taille différente de 8x8*/

void reconstitue_mcu(uint8_t **tab_concatene, uint8_t *tab_idct, uint8_t k, uint8_t coef_mcu_x, uint8_t coef_mcu_y){
  //coef_mcu_x et coef_mcu_y correspondent aux dimensions de la MCU en nombre de bloc
  if ((coef_mcu_x != 1 || coef_mcu_y != 1) && *tab_concatene == NULL){
    *tab_concatene = malloc((coef_mcu_x * coef_mcu_y << 6) * sizeof(uint8_t));
  }
  // cas uniquement vertical, on concatene les Y à la suite
  if (coef_mcu_y == 2 && coef_mcu_x != 2){
    for (uint8_t j = k<<6; j < ((k<<6) + 64); j++){
      (*tab_concatene)[j] = tab_idct[j - (k<<6)];
    }
  }
  //cas uniquement horizontal, on intercalle 8 élements d'un bloc
  //avec 8 èlements du suivant
  else if (coef_mcu_x == 2 && coef_mcu_y != 2){
    intercalle_bloc(tab_concatene,0, 64, tab_idct, k, 0);
   }

  //cas vertical ET horizontal
  else if (coef_mcu_x == 2 && coef_mcu_y == 2){
    if (k < 2){
      //on passe dans le else if, cas horizontal
      reconstitue_mcu(tab_concatene, tab_idct, k, coef_mcu_x, 3);
      }
    else if (k == 2){
      intercalle_bloc(tab_concatene, k<<6, ((k<<6) + 64),tab_idct, 0, k);
     }
    else{
      //pour le dernier bloc, on intercalle les elements du bloc actuel
      //avec le précedent
      uint8_t compteur = 1;
      uint8_t indice = 0;
      uint16_t j = ((k-1)<<6) + 8; //on recommence aprés les 8 bits du mcu précédent
      while (j < (k+1)<<6){
        (*tab_concatene)[j] = tab_idct[indice];
        if (compteur%8 == 0 && compteur != 1){
          j+=8;
        }
       j++;
       compteur++;
       indice++;
      }
    }
  }
  // pas d'échantillonage
  else{
    (*tab_concatene) = tab_idct;
  }
}


/* écrit 8 pixels d'un bloc dans tab_concatene, puis saute 8 élements et ainsi de suite,
jusqu'a l'écriture entiere du bloc. Debut et fin permettent de ne pas écraser les données
écrites s'il y a eu un échantillonage vertical auparavant */

void intercalle_bloc(uint8_t **tab_concatene, uint8_t debut, uint8_t fin, uint8_t *tab_idct, uint8_t multiplicateur_valeur, uint8_t k){
  uint8_t multiplicateur = multiplicateur_valeur;
  uint8_t compteur = 1;
  for (uint8_t j=debut; j < fin ; j++){
    (*tab_concatene)[j+ (multiplicateur<<3)] = tab_idct[j - (k<<6)];
    if (compteur%8 == 0 && compteur!=0){
      multiplicateur++;
    }
    compteur++;
  }
}

/* retourne un tableau à 1 (image noir et blanc) ou 3 (image en couleur) élements,
contenant les indices (ic) des composantes définis dans le frame Header */

uint8_t *initialise_tab_id(struct jpeg_desc *jdesc, uint8_t nb_components){
  uint8_t *tab_id;
  if (nb_components == 3){
    tab_id = malloc(3* sizeof(uint8_t));
    tab_id[1] = get_frame_component_id(jdesc, 1);
    tab_id[2] = get_frame_component_id(jdesc, 2);
  }
  else{
    tab_id = malloc(sizeof(uint8_t));
  }
  tab_id[0] = get_frame_component_id(jdesc, 0);
  return tab_id;
}
