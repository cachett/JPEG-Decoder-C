# Repertoires du projet

BIN_DIR = bin
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
OBJPROF_DIR = obj-prof

# Options de compilation/édition des liens

CC = clang
LD = clang
INC = -I$(INC_DIR)

CFLAGS += $(INC) -Wall -std=c99 -O2 -g  -Wextra 
LDFLAGS =  -lm  

# Liste des fichiers objet

# les notres...
# et les votres!
OBJ_FILES = $(OBJ_DIR)/main.o $(OBJ_DIR)/utilitaire.o $(OBJ_DIR)/decrompression.o $(OBJ_DIR)/quantification_inverse.o $(OBJ_DIR)/reverse_zig_zag.o
OBJ_FILES += $(OBJ_DIR)/idct.o $(OBJ_DIR)/reconstitution_ppm.o $(OBJ_DIR)/ycbcr_to_rgb.o $(OBJ_DIR)/upsampling.o $(OBJ_DIR)/jpeg_reader.o $(OBJ_DIR)/bitstream.o $(OBJ_DIR)/huffman.o

# cible par défaut

TARGET = $(BIN_DIR)/jpg2ppm

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(LD) $(LDFLAGS) $(OBJ_FILES) -o $(TARGET)

$(OBJ_DIR)/utilitaire.o: $(SRC_DIR)/utilitaire.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/utilitaire.c -o $(OBJ_DIR)/utilitaire.o
$(OBJ_DIR)/decrompression.o: $(SRC_DIR)/decrompression.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/decrompression.c -o $(OBJ_DIR)/decrompression.o
$(OBJ_DIR)/quantification_inverse.o: $(SRC_DIR)/quantification_inverse.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/quantification_inverse.c -o $(OBJ_DIR)/quantification_inverse.o
$(OBJ_DIR)/reverse_zig_zag.o: $(SRC_DIR)/reverse_zig_zag.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/reverse_zig_zag.c -o $(OBJ_DIR)/reverse_zig_zag.o
$(OBJ_DIR)/idct.o: $(SRC_DIR)/idct.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/idct.c -o $(OBJ_DIR)/idct.o
$(OBJ_DIR)/reconstitution_ppm.o: $(SRC_DIR)/reconstitution_ppm.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/reconstitution_ppm.c -o $(OBJ_DIR)/reconstitution_ppm.o
$(OBJ_DIR)/ycbcr_to_rgb.o: $(SRC_DIR)/ycbcr_to_rgb.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/ycbcr_to_rgb.c -o $(OBJ_DIR)/ycbcr_to_rgb.o
$(OBJ_DIR)/upsampling.o: $(SRC_DIR)/upsampling.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/upsampling.c -o $(OBJ_DIR)/upsampling.o
$(OBJ_DIR)/jpeg_reader.o: $(SRC_DIR)/jpeg_reader.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/jpeg_reader.c -o $(OBJ_DIR)/jpeg_reader.o
$(OBJ_DIR)/bitstream.o: $(SRC_DIR)/bitstream.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/bitstream.c -o $(OBJ_DIR)/bitstream.o
$(OBJ_DIR)/huffman.o: $(SRC_DIR)/huffman.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/huffman.c -o $(OBJ_DIR)/huffman.o
$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c $(INC_DIR)/jpeg_reader.h $(INC_DIR)/bitstream.h $(INC_DIR)/huffman.h $(INC_DIR)/utilitaire.h $(INC_DIR)/reverse_zig_zag.h $(INC_DIR)/idct.h $(INC_DIR)/reconstitution_ppm.h $(INC_DIR)/decrompression.h $(INC_DIR)/ycbcr_to_rgb.h $(INC_DIR)/upsampling.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/main.c -o $(OBJ_DIR)/main.o



.PHONY: clean

clean:
	rm -f $(TARGET) $(OBJ_FILES)
