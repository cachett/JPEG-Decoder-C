#ifndef __IDCT_H__
#define __IDCT_H__






/* realise l'idct grâce à l'algorithme de Loeffler */
extern uint8_t *idct_rapide(int16_t *tab);

void idct_rapide_1d(float *tab_1d);

void stage1(float **tab_1d);

void stage2(float **tab_1d);

void stage3(float **tab_1d);

void stage4(float **tab_1d);

void swap(float **tab_1d, uint8_t indice1, uint8_t indice2);

void reorganisation(float **tab_1d);

void rotor(float **tab_1d, uint8_t indice1, uint8_t indice2, float coef_k, float cosinus, float sinus);

void butterfly(float **tab_1d, uint8_t indice1, uint8_t indice2);
#endif
