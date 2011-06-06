#include "libguardaimagen.h"

#pragma warning ( disable: 4996 )

void guardaPGMi(char* nombre, int anchura, int altura, int *pixels, int pixel_min, int pixel_max)
{
	int i, j, ij, p;
	FILE* imagen;
	imagen = fopen(nombre, "wb");
	fprintf(imagen, "P2");
	fprintf(imagen, "#guardaPGMi %s\n", nombre);
	fprintf(imagen, "%d %d\n", anchura, altura);
	fprintf(imagen, "255\n");
	ij = 0;
	for (i=0; i<altura; i++)
	{
		for (j=0; j<anchura; j++)
		{
			p= (255*(pixels[ ij ]-pixel_min)) / (pixel_max-pixel_min);
			if ( p<0 ) p=0;
			if ( p>255) p=255;
			fprintf (imagen, " %d", p);
			ij++;
		}
		fprintf (imagen, "\n");
	}
	fclose (imagen);
}

void guardaPGMd (char* nombre, int anchura, int altura, double *pixels, double pixel_min, double pixel_max)
{
	int i, j, ij, p;
	FILE* imagen;
	imagen = fopen(nombre, "wb");
	fprintf(imagen, "P2");
	fprintf(imagen, "#guardaPGMi %s\n", nombre);
	fprintf(imagen, "%d %d\n", anchura, altura);
	fprintf(imagen, "255\n");
	ij=0;
	for (i=0; i<altura; i++)
	{
		for (j = 0; j < anchura; j++)
		{
			p = (int)((255.0 * (pixels[ij] - pixel_min)) / (pixel_max - pixel_min));
			if ( p<0 ) p=0;
			if ( p>255) p=255;
			fprintf (imagen, " %d", p);
			ij++;
		}
		fprintf (imagen, "\n");
	}
	fclose (imagen);
}

void guardaPGMiACE(char* nombre, int pasos, int celdas, int **pixels, int pixel_min, int pixel_max)
{
	int altura = pasos + 1;
	int anchura = celdas + 2;
	int i, j, p;
	FILE* imagen;
	imagen = fopen(nombre, "wb");
	fprintf(imagen, "P2");
	fprintf(imagen, "#guardaPGMi %s\n", nombre);
	fprintf(imagen, "%d %d\n", anchura, altura);
	fprintf(imagen, "255\n");
	for (i = 0; i < altura; i++)
	{
		for (j = 0; j < anchura; j++)
		{
			p = (255 * (pixels[i][j] - pixel_min)) / (pixel_max - pixel_min);
			if (p < 0) 
				p = 0;
			else if (p > 255) 
				p = 255;
			fprintf(imagen, " %d", p);
		}
		fprintf (imagen, "\n");
	}
	fclose (imagen);
}

void guardaPLOT (char* nombre, const int *y, int valores, int xini)
{
	FILE* plot;
	plot = fopen(nombre, "wb");
	for (int i = 0; i < valores; i++) {
		fprintf (plot, "%d %d\n", i + xini, y[i]);
	}
	fclose (plot);
}

void guardaPLOT (char* nombre, const double *y, int valores, int xini, int decimales)
{
	char decs[32];
	sprintf(decs, "%%d %%.%df\n", decimales);
	FILE* plot;
	plot = fopen(nombre, "wb");
	for (int i = 0; i < valores; i++) {
		fprintf (plot, decs, i + xini, y[i]);
	}
	fclose (plot);
}

void guardarAtractorPLOT(char* nombreFichero, int** probabilidades, int pasos, int estados)
{
	FILE* plot;
	plot = fopen(nombreFichero, "wb");
	for (int p = 0; p < pasos; p++) {
		for (int e = 0; e < estados; e++) {
			if (probabilidades[p][e] != 0) 
				fprintf(plot, "%d %d\n", e, p);
		}
	}
	fclose (plot);
}
