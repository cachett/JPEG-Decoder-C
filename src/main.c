#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "huffman.h"
#include "utilitaire.h"
#include "quantification_inverse.h"
#include "reverse_zig_zag.h"
#include "idct.h"
#include "reconstitution_ppm.h"
#include "ycbcr_to_rgb.h"
#include "decrompression.h"
#include "upsampling.h"

bool verbose = false;
bool blabla = false;

void decode_jpg(char *argv)
{
    /* On recupere le nom du fichier JPEG sur la ligne de commande. */
    char *filename = argv;
    /* On cree un jpeg_desc(ripteur) qui permettra de lire ce fichier. */
    struct jpeg_desc *jdesc = read_jpeg(filename);
    /* On recupere le flux des donnees brutes a partir du descripteur. */
    struct bitstream *stream = get_bitstream(jdesc);

    /* Initialisation de l'image */
    uint8_t nb_components = get_nb_components(jdesc);
    uint16_t size_x = get_image_size(jdesc, DIR_H);
    uint16_t size_y = get_image_size(jdesc, DIR_V);
    //new_filename est le nom du fichier image sans le .jpeg ou .jpg
    char *new_filename = initialise_picture(filename, size_x, size_y, nb_components);
    /* Initialisation du .blabla si le mode -blabla est activé*/
    FILE *blabla_fichier = NULL;
    if (blabla){
      const char *suffixe2 = ".blabla";
      char *filename_blabla = malloc((strlen(filename)+8)*sizeof(char));
      strcpy(filename_blabla, filename);
      filename_blabla = strcat(filename_blabla, suffixe2);
      blabla_fichier = fopen(filename_blabla, "w");
    }
    if (verbose){
      affiche_info_entete(jdesc);
    }
    /* Initialisation des paramètres */
    uint32_t *tab_image = malloc(size_x*size_y*sizeof(uint32_t)); //tableau dont les élèments sont à afficher sur le fichier ppm ou pgm
    assert(tab_image != NULL && "Problème allocation mémoire\n");
    int16_t *tab_initial = NULL; //tableau de départ utilisé jusqu'à idct
    uint8_t *tab_idct = NULL; //tableau aprés idct
    uint8_t **tabYCbCr = NULL; //tableau de pointeur vers les blocs des trois composantes
    uint32_t *tab_decode = NULL; //tableau de pixel sous forme RGB
    int16_t *predicateur = calloc(3, sizeof(int16_t));
    assert(predicateur != NULL && "Problème allocation mémoire\n");
    uint8_t *tab_upsampling = NULL; //tableau aprés l'opération d'upsampling sur les différentes composantes
    uint8_t *sampling_factor = initialise_tab_sampling_factor(jdesc); //contient les facteurs de sous-echantillonage des composantes
    uint32_t nb_mcu = calcul_nb_mcu(size_x, size_y, sampling_factor);
    uint8_t *identifiant_composantes = initialise_tab_id(jdesc, nb_components); //contient les bonnes valeurs des identifiants définis dans le Frame

    //pour chaque MCU...
    uint32_t i = 0;
    while (i < nb_mcu){
      if (blabla) {fprintf(blabla_fichier,"\n ***MCU : %u  \n", i);}
      //pour chaque composante du mcu...
      for (uint8_t frame_comp_index = 0; frame_comp_index < nb_components; frame_comp_index++){
        uint8_t nb_bloc_current_comp = sampling_factor[2*frame_comp_index] * sampling_factor[2*frame_comp_index + 1];
        //pour gerer l'ordre des composantes
        uint8_t identifiant_comp_scan = get_scan_component_id(jdesc, frame_comp_index);
        uint8_t memoire = frame_comp_index;
        frame_comp_index = maj_frame_comp_index(identifiant_comp_scan, &frame_comp_index, identifiant_composantes);
        if (blabla){
          fprintf(blabla_fichier, "* Component %s\n", (frame_comp_index == 0) ? "Y" : (frame_comp_index == 1) ? "Cb" : "Cr");
        }

        //pour chaque bloc du MCU, on effectue le décodage
        for (uint8_t k = 0 ; k < nb_bloc_current_comp ; k++){
          tab_initial = reverse_huffman(jdesc, stream, &predicateur, frame_comp_index);
          if (blabla){
            fprintf(blabla_fichier,"Bloc : %u\nAprès huffman : \n", k);
            affiche_tableau(blabla_fichier,tab_initial);
          }
          tab_initial = quantification_inverse(jdesc, tab_initial, frame_comp_index);
          if (blabla){
            fprintf(blabla_fichier,"Après quantification inverse : \n");
            affiche_tableau(blabla_fichier,tab_initial);
          }
          tab_initial = reverse_zig_zag(tab_initial);
          if (blabla){
            fprintf(blabla_fichier,"Après ZZ : \n");
            affiche_tableau(blabla_fichier, tab_initial);
          }
          tab_idct = idct_rapide(tab_initial);
          if (blabla){
            fprintf(blabla_fichier,"Après idct :\n");
            affiche_tableau_idct(blabla_fichier,tab_idct);
          }
          free(tab_initial);
          if (frame_comp_index == 0){ //si component Y
            reconstitue_mcu(&tab_upsampling, tab_idct, k, sampling_factor[0], sampling_factor[1]);
            if (sampling_factor[0]*sampling_factor[1] > 1){
              free(tab_idct);
              }
            }
          }
        if (frame_comp_index > 0){ // si component Cb ou Cr --> un seul bloc, on ne rentre pas dans le boucle précèdente
           upsampling(tab_idct, sampling_factor[0], sampling_factor[1], &tab_upsampling);
           if (sampling_factor[0]*sampling_factor[1] > 1){
             free(tab_idct);
             }
           }

        build_tabYCbCr(tab_upsampling, nb_components, frame_comp_index, &tabYCbCr); //construit un tableau de pointeur vers les blocs des composantes
        // image en noir et blanc
        if (nb_components == 1){
          tab_decode = ycbcr_to_rgb(tabYCbCr, sampling_factor[0], sampling_factor[1]); //composante Y entierement décodée
          free(tabYCbCr[0]);
          //mise au bon endroit des élements de tab_decode dans le tableau tab_image
          set_current_pixels(tab_decode, &tab_image, i, size_x, size_y, sampling_factor[0], sampling_factor[1]);
          free(tab_decode);
          }
        // image en couleur
        else if (tabYCbCr[2] != NULL){
         //bloc complet (toutes les composantes sont chargées dans tabYCbCr)
          if (tabYCbCr[1] != NULL){ //pour gérer le cas YCrCb : On remplirait d'abord la case numéro 2 avant la case 1
             tab_decode = ycbcr_to_rgb(tabYCbCr, sampling_factor[0], sampling_factor[1]);
             set_current_pixels(tab_decode, &tab_image, i, size_x, size_y, sampling_factor[0], sampling_factor[1]);
             free(tabYCbCr[0]);
             free(tabYCbCr[1]);
             free(tabYCbCr);
             //memoire == frame_comp_index signifie que l'ordre est bien Y/Cb/Cr
             if (sampling_factor[0]*sampling_factor[1] == 1 && memoire == frame_comp_index){
               free(tab_idct);
             }
             tabYCbCr = NULL; //et on remet le tableau a NULL pour recommencer avec une nouvelle MCU
             free(tab_decode);
          }else{
             tab_decode = ycbcr_to_rgb(tabYCbCr, sampling_factor[0], sampling_factor[1]);
             set_current_pixels(tab_decode, &tab_image, i, size_x, size_y, sampling_factor[0], sampling_factor[1]);
           }
         }
        frame_comp_index = memoire; //pour le bon fonctionnement de la boucle
        }
      i++;
    }
    //écriture des élements de tab_image dans le fichier ppm (pour une image en couleur) ou pgm
    if (nb_components == 1) {
      write_picture_pgm(new_filename, tab_image, size_x, size_y);
    }else{
      write_picture_ppm(new_filename, tab_image, size_x, size_y);
      }
    // nb_components>1 pour gérer le cas de taille MCU exotiques en noir et blanc
    if (sampling_factor[0]*sampling_factor[1] > 1 && nb_components>1){
      free(tab_upsampling);
      }
    free(tab_image);
    free(predicateur);
    free(new_filename);
    free(identifiant_composantes);
    if (blabla){fclose(blabla_fichier);}
    free(tabYCbCr);
    free(sampling_factor);
    close_jpeg(jdesc);
}

int main(int argc, char **argv){
  uint8_t i = 1;
  uint8_t nombre_photo_decode = argc -1;
  if (argc < 2) {
    /* Si il n'y pas au moins DEUX arguments en ligne de commandes, on boude. */
    fprintf(stderr, "Usage: %s [-v] [-blabla] fichier1.jpeg fichier2.jpeg etc\n", argv[0]);
    return EXIT_FAILURE;
  }

  if (strcmp(argv[1], "-v")== 0 || strcmp(argv[1], "-blabla") == 0){
    i = 2;
    if (strcmp(argv[1], "-v")== 0){
      printf("Mode verbose activé ! \n");
      verbose = true;
      nombre_photo_decode--;
    }
    else{
      printf("Création d'un blabla en cours! \n");
      blabla = true;
      nombre_photo_decode--;
    }
  }
  if (verbose || blabla){
    if (argv[2] != NULL && strcmp(argv[2], "-blabla") == 0){
      printf("Création d'un blabla en cours! \n");
      nombre_photo_decode--;
      i = 3;
      blabla = true;
    }
    if (argv[2] != NULL && strcmp(argv[2], "-v") == 0){
      printf("Mode verbose activé! \n");
      nombre_photo_decode--;
      i = 3;
      verbose = true;
    }
  }
  printf("Nombre de pĥoto à décoder : %u \n", nombre_photo_decode );
  if (nombre_photo_decode <= 0){
    printf("Usage: %s [-v] [-blabla] fichier.jpeg\n", argv[0]);
    return EXIT_FAILURE;
  }
  while (i < argc){
    decode_jpg(argv[i]);
    printf("%s décodée !\n", argv[i]);
    i++;
    }
  printf("\nMerci d'avoir utilisé notre décodeur !\n");
  return EXIT_SUCCESS;
}
