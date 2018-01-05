

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


/*retourne un tableau contenant les élèments obtenus après décompression
réorganiser sous forme "zig_zag"
*/

extern int16_t *reverse_zig_zag(int16_t *tab){
  uint16_t tab_new_indice[64] = {0, 1, 5, 6, 14, 15, 27, 28,
                                 2, 4, 7, 13, 16, 26, 29, 42,
                                 3, 8, 12, 17, 25, 30, 41, 43,
                                 9, 11, 18, 24, 31, 40, 44, 53,
                                 10, 19, 23, 32, 39, 45, 52, 54,
                                 20, 22, 33, 38, 46, 51, 55, 60,
                                 21, 34, 37, 47, 50, 56, 59, 61,
                                 35, 36, 48, 49, 57, 58, 62, 63};

    int16_t *new_tab = malloc(sizeof(int16_t)<<6);
    uint16_t i = 0;
    while (i < 64){
      new_tab[i] = tab[tab_new_indice[i]];
      i++;
    }
    free(tab);
    return new_tab;
}
