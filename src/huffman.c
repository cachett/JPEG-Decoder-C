#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "huffman.h"



struct huff_table{
  int8_t valeur;
  struct huff_table *fils_gauche;
  struct huff_table *fils_droit;
  struct huff_table *pere;
};

/* fonction d'affichage de l'arbre construit pour effectuer du debeugage (inutilisée) */

void affiche_arbre(struct huff_table *table, char *path){
  char *zero = "0";
  char *un = "1";
  if (table->fils_gauche == NULL && table->fils_droit == NULL){
    printf("path : %s ", path);
    printf("valeur : %hhx \n", table -> valeur);
  }
  else if (table->fils_droit == NULL){
    strcat(path, zero);
    affiche_arbre(table->fils_gauche, path);
  }
  else{
    char *pathcopy = malloc(sizeof(char)<<6); //on alloue une taille suffisament grande
    strcpy(pathcopy, path);
    strcat(path, zero);
    affiche_arbre(table->fils_gauche, path);
    strcat(pathcopy, un);
    affiche_arbre(table->fils_droit, pathcopy);
    free(pathcopy);
  }
}

/* constuit la table de huffman (sous forme d'arbre) propre a l'image en cours de décodage,
et met a jour le nombre de byte lu pendant la création de l'arbre  */

extern struct huff_table *load_huffman_table(struct bitstream *stream, uint16_t *nb_byte_read){
  *nb_byte_read = 0;
  uint32_t byte = 0;
  uint32_t nombre_total_symbole = 0;
  struct huff_table *noeud_courant = calloc(1, sizeof(struct huff_table));
  if (noeud_courant == NULL){
    *nb_byte_read = -1;
    return NULL;
  }
  uint32_t *nombre_element_meme_longueur = malloc(sizeof(uint32_t) << 4);
  assert(nombre_element_meme_longueur != NULL && "Pb allocation mémoire\n");
  // initialisation du tableau contenant le nombre d'élément de même longueur
  for (uint8_t i = 0; i < 16; i++){
    read_bitstream(stream, 8, &byte, false);
    *nb_byte_read += 1;
    nombre_total_symbole += byte;
    nombre_element_meme_longueur[i] = byte;
    }
  if (nombre_total_symbole > 256){
    perror("ERREUR dans la section DHT, trop de symboles \n");
    exit(EXIT_FAILURE);
    }
  uint8_t profondeur = 0;
  for (uint8_t i = 0; i < 16; i++){
    uint8_t longueur_element = i + 1;
    uint8_t nb_element = nombre_element_meme_longueur[i];
    // la variable compteur compte les éléments qu'on a déja inscrit dans la huff_table pour une longueur donnée
    uint8_t compteur = 0;
    while (compteur < nb_element){
      while (profondeur != longueur_element){
        //en priorité on crée un fils gauche
        if (noeud_courant->fils_gauche == NULL){
          noeud_courant->fils_gauche = calloc(1, sizeof(struct huff_table));
          if (noeud_courant->fils_gauche == NULL){
            *nb_byte_read = -1;
            return NULL;
            }
          noeud_courant->fils_gauche->pere = noeud_courant;
          noeud_courant = noeud_courant->fils_gauche;
          profondeur++;
          }
        else if (noeud_courant->fils_droit == NULL){
          noeud_courant->fils_droit = calloc(1, sizeof(struct huff_table));
          if (noeud_courant->fils_droit == NULL){
            *nb_byte_read = -1;
            return NULL;
            }
          noeud_courant->fils_droit->pere = noeud_courant;
          noeud_courant = noeud_courant->fils_droit;
          profondeur++;
          }
        else{
          noeud_courant = noeud_courant->pere;
          profondeur--;
          }
      }
      //lecture de la valeur correspondante
      read_bitstream(stream, 8, &byte, false);
      noeud_courant->valeur = byte;
      noeud_courant = noeud_courant->pere;
      profondeur--;
      compteur++;
      *nb_byte_read += 1;
    }
  }
  // on veut retourner la racine de l'arbre
  while (noeud_courant->pere != NULL){
    noeud_courant = noeud_courant->pere;
  }
  nombre_total_symbole = (uint16_t) nombre_total_symbole;
  // On vérifie que le nombre total de symbole correspond bien au nombre de symbole que l'on a lu
  if (nombre_total_symbole != *nb_byte_read - 16){
    *nb_byte_read = -1;
    return NULL;
  }
  free(nombre_element_meme_longueur);
  return noeud_courant;
}


/* retourne la prochaine valeur atteinte en parcourant la table de Huffman table
selon les bits extraits du flux stream . */

extern int8_t next_huffman_value(struct huff_table *table, struct bitstream *stream){
  uint32_t byte = 0;
  uint8_t nb_bits_read = 0;
  //la valeur recherchée est forcément sur une feuille, qui est caractérisée
  //par l'abscence de fils gauche
  while (table->fils_gauche != NULL){
    read_bitstream(stream, 1, &byte, true);
    nb_bits_read +=1;
    if (byte == 0){
      table = table->fils_gauche;
    }else{
      table = table->fils_droit;
      }
  }
  return table->valeur;
}


/* libération de la mémoire allouée pour la création de la table de huffman */

extern void free_huffman_table(struct huff_table *table){
  if (table == NULL){
    return;
  }
  if (table->fils_gauche == NULL){
    free(table);
  }else if (table->fils_droit == NULL){
    free_huffman_table(table->fils_gauche);
    free(table);
  }else{
    free_huffman_table(table->fils_gauche);
    free_huffman_table(table->fils_droit);
    free(table);
  }
}
