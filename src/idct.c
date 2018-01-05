/* Module permettant de faire le idct inverse */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "idct.h"
#define PI 3.14159265
#define COS_1 0.98078528 //cos(PI/16)
#define COS_2 0.83146961 //cos(3*PI/16)
#define COS_3 0.38268343 //cos(3*PI/8)
#define SIN_1 0.19509032 //sin(PI/16)
#define SIN_2 0.55557023 //sin(3*PI/16)
#define SIN_3 0.92387953 //sin(3*PI/8)
#define SQRT_2 1.41421356 //sqrt(2)


/* realise l'idct grâce à l'algorithme de Loeffler */

extern uint8_t *idct_rapide(int16_t *tab){
  uint8_t *new_tab = malloc(sizeof(uint8_t) << 6); //tableau qui contiendra le résultat final
  float *tab_1d = malloc(sizeof(float)<<3); //tableau temporaire pour l'idct en 1d
  float *tab_temp = malloc(sizeof(float)<<6);
  //on effectue idct sur les lignes
  for (uint8_t k= 0; k<8; k++){
    for (uint8_t j= 0; j<8; j++){ //remplis le tableau 1d avec une ligne
      tab_1d[j] = ((float) tab[(k<<3) + j]);
    }
    idct_rapide_1d(tab_1d);
    for (uint8_t j = 0; j < 8; j++){//résultat stocké dans un tableau temp de float
      tab_temp[(k<<3) + j] = tab_1d[j];
    }
  }
  //on effectue idct sur les colonnes
  for (uint8_t k= 0; k<8; k++){
    for (uint8_t j= 0; j<8; j++){ //remplis le tableau 1d avec une colonne
      tab_1d[j] =  tab_temp[(j<<3) + k];
    }
    idct_rapide_1d(tab_1d);
    // clamping entre 0 et 255
    for (uint8_t i = 0; i<8; i++){
     tab_1d[i] += 128 ;
      if (tab_1d[i] < 0){
        tab_1d[i] = 0;
      }else if (tab_1d[i] > 255){
        tab_1d[i] = 255;}
    }
    //résultat sotcké dans un nouveau tableau de taille 64
    for (uint8_t j = 0; j<8; j++){
      new_tab[(j<<3) + k] = (uint8_t) (tab_1d[j] + 0.49);
    }
  }
  free(tab_1d);
  free(tab_temp);
  return new_tab;
}

void idct_rapide_1d(float *tab_1d){
  reorganisation(&tab_1d);
  stage1(&tab_1d);
  stage2(&tab_1d);
  stage3(&tab_1d);
  stage4(&tab_1d);
}

void stage1(float **tab_1d){
  for (uint8_t k=0; k<8; k++){
    (*tab_1d)[k] = (*tab_1d)[k] * 2 * SQRT_2;
  }
  (*tab_1d)[5] = (*tab_1d)[5] / SQRT_2;
  (*tab_1d)[6] = (*tab_1d)[6] / SQRT_2;
  butterfly(tab_1d, 7, 4);
}

void stage2(float **tab_1d){
  butterfly(tab_1d, 0, 1);
  rotor(tab_1d, 2, 3, sqrt(2), COS_3, SIN_3);
  butterfly(tab_1d, 4, 6);
  butterfly(tab_1d, 7, 5);
}

void stage3(float **tab_1d){
  butterfly(tab_1d, 0, 3);
  butterfly(tab_1d, 1, 2);
  rotor(tab_1d, 4, 7, 1, COS_2, SIN_2);
  rotor(tab_1d, 5, 6, 1, COS_1, SIN_1);
}

void stage4(float **tab_1d){
  butterfly(tab_1d, 0, 7);
  butterfly(tab_1d, 1, 6);
  butterfly(tab_1d, 2, 5);
  butterfly(tab_1d, 3, 4);
}

/* échange la valeur de deux case d'un tableau */
void swap(float **tab_1d, uint8_t indice1, uint8_t indice2){
  float memoire = (*tab_1d)[indice1];
  (*tab_1d)[indice1] = (*tab_1d)[indice2];
  (*tab_1d)[indice2] = memoire;
}

/*reorganise le tableau pour commencer le stage1 */
void reorganisation(float **tab_1d){
  swap(tab_1d, 1, 4);
  swap(tab_1d, 4, 7);
  swap(tab_1d, 3, 6);
  swap(tab_1d, 6, 5);
}


void rotor(float **tab_1d, uint8_t indice1, uint8_t indice2, float coef_k, float cosinus, float sinus){
  float tmp = ((*tab_1d)[indice1]*cosinus-(*tab_1d)[indice2]*sinus)/coef_k;
  (*tab_1d)[indice2] = ((*tab_1d)[indice2]*cosinus+(*tab_1d)[indice1]*sinus)/coef_k;
  (*tab_1d)[indice1] = tmp;
}

void butterfly(float **tab_1d, uint8_t indice1, uint8_t indice2){
  float tmp = ((*tab_1d)[indice1] + (*tab_1d)[indice2])/2;
  (*tab_1d)[indice2] = ((*tab_1d)[indice1] - (*tab_1d)[indice2])/2;
  (*tab_1d)[indice1] = tmp;

}
