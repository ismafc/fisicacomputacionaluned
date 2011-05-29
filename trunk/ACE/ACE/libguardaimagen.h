#ifndef _LIBGUARDAIMAGEN_H_
#define _LIBGUARDAIMAGEN_H_

#include <stdio.h>

/* 
	Guarda en el archivo de nombre dado una imagen PGM de dimensiones anchura X altura 
	que contiene en sus píxeles valores enteros entre pixel_min y pixel_max 
*/
void guardaPGMi (char* nombre, int anchura, int altura, int *pixels, int pixel_min, int pixel_max);

/* 
	Guarda en el archivo de nombre dado una imagen PGM de dimensiones anchura X altura 
	que contiene en sus píxeles valores reales entre pixel_min y pixel_max
*/
void guardaPGMd (char* nombre, int anchura, int altura, double *pixels, double pixel_min, double pixel_max);

void guardaPGMiACE (char* nombre, int pasos, int celdas, int **pixels, int pixel_min, int pixel_max);

void guardaPLOT (char* nombre, const int *y, int valores, int xini = 1);

void guardarExponenteHamming(char* nombreFichero, double eh, bool esPosible);

void guardarAtractorPLOT(char* nombreFichero, int** probabilidades, int pasos, int estados);

#endif