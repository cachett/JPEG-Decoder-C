This is a complete JPEG decoder developped in C, which take one or more .jpeg in argument and generate the correspondant .ppm picture.

Example of use: 
./bin/jpg2ppm [-v] [-blabla] images/name.jpg




                      Les différentes options de l'éxécutable 

l'option verbose “-v” affiche les caractéristiques de l'image JPEG à décoder.

L'option “-blabla” offre la trace de toutes les étapes de décodage dans un fichier filename.jp*g.blabla situé dans le dossier images.

Il est également possible de décoder plusieurs images en une ligne de commande : il suffit de donner le nom  des images à décoder à la suite les une des autres.

-Les images traitées 

Attention, le décodeur ne fonctionne que sur des images JPEG au format SOF0. Un message d'erreur sera affiché dans les autres cas.

Seul les dimensions de MCU en nombre de blocs suivantes sont possibles : 2x2, 2x1, 1x2. Nous n'avons pas gérer les cas de facteurs d'échantillonages plus “exotiques”. 


Exemple d'utilisation :
./bin/jpg2ppm [-v] [-blabla] images/nom.jp*g
