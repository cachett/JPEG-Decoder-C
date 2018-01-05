
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "jpeg_reader.h"


/* Initialisation de l'image ppm ou pgm et écriture de l'entête textuel */

extern char *initialise_picture(char *filename, uint16_t size_x, uint16_t size_y, uint8_t nb_components){
  const char *suffixe = (nb_components == 1) ? ".pgm" : ".ppm";
  // on enleve .jpeg ou .jpg
  uint8_t nombre_caractere = strlen(filename);
  uint8_t nbr_char_a_enlever = 0;
  //le nom de fichier est .jpeg
  if (filename[nombre_caractere - 2] == 0x65){
    nbr_char_a_enlever= 5; // on enleve donc 5 caracteres
  }
  else{
    nbr_char_a_enlever = 4; //le nom de fichier est .jpg, on enleve 4 caracteres
  }
  //+5 car on rajoute .pgm ou .ppm et la caractère de fin '\0'
  char *new_filename = calloc(nombre_caractere - nbr_char_a_enlever + 5, sizeof(char));

  //réecriture de filename
  strncpy(new_filename, filename, (nombre_caractere - nbr_char_a_enlever));
  strcat(new_filename, suffixe);
  FILE *f = fopen(new_filename,"w");
  //si l'image est en noir et blanc le magic number vaut P5, sinon P6
  char *magic_number = (nb_components == 1) ? "P5" : "P6";
  fprintf(f,"%c%c\n%i %i\n255\n", magic_number[0], magic_number[1], size_x, size_y);
  fclose(f);
  return new_filename;

}


/* chargement a la bonne place dans le tableau "tab_image" des pixels décodés.
Ainsi, les pixels dans tab_image sont dans le même ordre que pour l'image réelle */

extern void set_current_pixels(uint32_t *tab_decode, uint32_t **tab_image, uint32_t index,
                              uint16_t size_x, uint16_t size_y, uint8_t coef_mcu_x, uint8_t coef_mcu_y)
{
    uint16_t nb_mcu_x = (size_x % (coef_mcu_x << 3) == 0) ? size_x/(coef_mcu_x << 3) : size_x/(coef_mcu_x << 3) + 1;
    uint16_t nb_mcu_y = (size_y % (coef_mcu_y << 3) == 0) ? size_y/(coef_mcu_y << 3) : size_y/(coef_mcu_y << 3) + 1;
    uint32_t nb_mcu = nb_mcu_x * nb_mcu_y;
    //pour se placer à la bonne "ordonnée" du mcu
    uint32_t coordonnee_mcu_x = index % (nb_mcu_x);
    uint32_t coordonnee_mcu_y = index / (nb_mcu_x);
    //origine du début du bloc
    uint32_t origine_ligne = ((((nb_mcu_x - 1) *  coef_mcu_x*coef_mcu_y << 3)  +
    coef_mcu_y *((coef_mcu_x << 3) - ((nb_mcu_x*coef_mcu_x << 3) - size_x))) << 3)* coordonnee_mcu_y ; //origine du début de la ligne de bloc
    uint32_t origine_colonne = 0;
    uint8_t max_i = coef_mcu_y << 3; //max_i représente le nombre de ligne a afficher
    uint8_t max_j = coef_mcu_x << 3; //max_j représente le nombre d'élement à afficher par ligne

    //cas dernier bloc qui peut être tronqué en bas et sur la droite
    if (index == nb_mcu - 1){
      max_i = ((coef_mcu_y << 3) - ((nb_mcu_y*coef_mcu_y << 3) - size_y));
      max_j = ((coef_mcu_x << 3) - ((nb_mcu_x*coef_mcu_x << 3) - size_x));
    }

    //cas colonne droite
    else if (index % nb_mcu_x == nb_mcu_x - 1){
      max_j = ((coef_mcu_x << 3) - ((nb_mcu_x * coef_mcu_x << 3) - size_x));
    }

    //cas derniere ligne
    else if (index >= (nb_mcu - nb_mcu_x)){
      max_i = ((coef_mcu_y << 3) - ((nb_mcu_y*coef_mcu_y << 3) - size_y));
    }

    for (uint8_t i = 0; i < max_i; i++){
      origine_colonne = (coordonnee_mcu_x  * coef_mcu_x << 3 ) + i * (size_x - max_i);
      for (uint8_t j = 0; j < max_j ; j++){
        uint32_t k = tab_decode[(i*coef_mcu_x << 3) + j];
        (*tab_image)[origine_colonne + origine_ligne + i*max_i + j] = k;
      }
    }
}


/* écriture des élements du tableau en binaire dans le fichier ppm */

extern void write_picture_ppm(char *filename, uint32_t *tab_image, uint16_t size_x, uint16_t size_y){
  FILE *f = fopen(filename, "ab");
  for (uint64_t i = 0; i < (size_x*size_y);i++){ //3 octets par pixel à écrire
    //ecriture du rouge
    uint8_t k = (tab_image[i] >> 16) & 0x000000FF;
    fwrite(&k, 1, 1, f);
    //ecriture du vert
    k = (tab_image[i] >> 8) & 0x000000FF;
    fwrite(&k, 1, 1, f);
    //ecriture du bleu
    k = tab_image[i] & 0x000000FF;
    fwrite(&k, 1, 1, f);
    }
  fclose(f);
}


/* écriture des élements du tableau en binaire dans le fichier pgm */

extern void write_picture_pgm(char *filename, uint32_t *tab_image, uint16_t size_x, uint16_t size_y){
  FILE *f = fopen(filename, "ab");
  for (uint64_t i = 0; i < (size_x*size_y); i++){ //ecriture d'un seul octet car l'image est en noir et blanc
    uint8_t k = tab_image[i];
    fwrite(&k, 1, 1, f);
  }
  fclose(f);
}
