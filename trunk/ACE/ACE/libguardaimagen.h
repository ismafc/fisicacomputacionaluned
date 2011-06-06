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

/* 
	Guarda en el archivo de nombre dado una imagen PGM de dimensiones (celdas + 2) X (pasos + 1)
	que contiene en sus píxeles valores reales entre pixel_min y pixel_max. 
	Representa el ACE almacenado en pixels en forma de doble vector ([paso][celda])
*/
void guardaPGMiACE (char* nombre, int pasos, int celdas, int **pixels, int pixel_min, int pixel_max);

/* 
	Guarda en el archivo de nombre dado los puntos de una gráfica almacenados en 'y'.
	En 'valores' tenemos el número de puntos que hay en 'y' (lista de enteros).
	Los valores de la coordenada x se obtienen incrementalmente a partir de 'xini' (por defecto 1)
	El formato del fichero es compatible con el comando plot de gnuplot (dos columnas, la primera el valor X y la segunda el valor Y)
*/
void guardaPLOT (char* nombre, const int *y, int valores, int xini = 1);

/* 
	Guarda en el archivo de nombre dado los puntos de una gráfica almacenados en 'y'.
	En 'valores' tenemos el número de puntos que hay en 'y' (lista de reales)
	Los valores de la coordenada x se obtienen incrementalmente a partir de 'xini' (por defecto 1).
	Cada valor de 'y' se guarda con 'decimales' decimales (por defecto 3)
	El formato del fichero es compatible con el comando plot de gnuplot (dos columnas, la primera el valor X y la segunda el valor Y)
*/
void guardaPLOT (char* nombre, const double *y, int valores, int xini = 1, int decimales = 3);

/* 
	Guarda en el archivo de nombre dado los valores de las visitas a cada estado posible de un ACE en cada paso.
	En 'probabilidades' tenemos la estructura (doble vector [paso][estado]) que almacena las visitas.	
	En 'pasos' tenemos los pasos de la simulación. Por tanto la estructura tendra 'pasos + 1' vectores de estados.
	En 'estados' tenemos el número de estados simulados.
	El formato del fichero es compatible con el comando plot de gnuplot (dos columnas, la primera el valor X y la segunda el valor Y)
	Para representarlo con el comando plot de gnuplot: plot [0:estados] [-1:pasos + 2] “nombreFichero” pt 7 ps 0.2.
*/
void guardarAtractorPLOT(char* nombreFichero, int** probabilidades, int pasos, int estados);

#endif