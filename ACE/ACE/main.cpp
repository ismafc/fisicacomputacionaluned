#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "libguardaimagen.h"

#pragma warning ( disable: 4996 )

#define INICIALIZACION_SEMILLA		0		// Se inicializa con un '1' en la primera fila, en la columna central
#define INICIALIZACION_ALEATORIA	1		// Se inicializa con una distribuci�n aleatoria de '0' y '1' en la primera fila

#define REGLA						54		// regla a aplicar por defecto
#define CELDAS						1000	// n�mero de celdas del ACE por defecto
#define PASOS						500		// n�mero de pasos de evoluci�n por defecto

#define TODAS_LAS_REGLAS			-1		// valor de regla que indica que hay que calcular todas las reglas

#define MIN_PASOS					1		// como m�nimo 1 paso de evoluci�n
#define MAX_PASOS					5000	// como m�ximo 5000 pasos de evoluci�n

#define MIN_CELDAS					2		// como m�nimo 2 celdas en el ACE
#define MAX_CELDAS					10000	// como m�ximo 10000 celdas en el ACE

#define MIN_REGLA					0		// primera regla
#define MAX_REGLA					255		// �ltima regla

int aleatorio(int a, int b) 
{
	double numerador = b - a;
	double denominador = RAND_MAX;
	double resultado = ((double)rand() * numerador) / denominador;

	if (resultado - floor(resultado) < ceil(resultado) - resultado)
		resultado = floor(resultado);
	else
		resultado = ceil(resultado);

	return a + (int)resultado;
}

void inicializarACE(int*** ACE, int pasos, int celdas, int inicializacion = INICIALIZACION_SEMILLA)
{
	int i;

	*ACE = new int* [pasos + 1];
	for (i = 0; i < pasos + 1; i++)
	{
		(*ACE)[i] = new int [celdas + 2];
		memset((*ACE)[i], 0, (celdas + 2) * sizeof(int));
	}

	if (inicializacion == INICIALIZACION_SEMILLA)
		(*ACE)[0][(celdas + 1) / 2] = 1;
	else if (inicializacion == INICIALIZACION_ALEATORIA) {
		for (i = 0; i < celdas + 2; i++)
			(*ACE)[0][i] = aleatorio(0, 1);
	}
}

void generarACE(int** ACE, int regla, int pasos, int celdas)
{
	int vecindad; 
	for (int i = 1; i < pasos + 1; i++)
	{
		for (int j = 1; j < celdas + 1; j++)
		{
			vecindad = (ACE[i - 1][j + 1] | ACE[i - 1][j] << 1 | ACE[i - 1][j - 1] << 2);
			ACE[i][j] = (regla >> vecindad) & 1;
		}
		/* actualizamos las condiciones peri�dicas de contorno */
		ACE[i][0] = ACE[i][celdas];
		ACE[i][celdas + 1] = ACE[i][1];
	}
}

int main(int argc, char** argv)
{
	int regla = REGLA;								// regla a aplicar (por defecto REGLA)
	int celdas = CELDAS;							// celdas del ACE (por defecto CELDAS)
	int pasos = PASOS;								// pasos de evoluci�n a simular (por defecto PASOS)
	int inicializacion = INICIALIZACION_SEMILLA;	// por defecto la inicializaci�n es por semilla ACE[0][CELDAS/2]=1
	int** ACE;										// donde guardamos el estado del aut�mata [T + 1][N + 2]
	char strInicializacion[32];						// guardamso el tipo de inicializaci�n para generar el nombre del fichero

	// Inicializamos el texto de la inicializaci�n del ACE como "semilla"
	strcpy(strInicializacion, "semilla");

	// Inicializamos la semilla de los n�meros aleatorios
	srand((unsigned int)time(NULL));

	// Procesado de los par�metros de entrada (si existen)
	for (int a = 2; a <= argc; a++) {
		if (strstr(argv[a -1], "inicializacion:") == argv[a - 1]) {
			if (strstr(argv[a -1], ":aleatoria") != NULL) {
				strcpy(strInicializacion, "aleatoria");
				inicializacion = INICIALIZACION_ALEATORIA;
			}
			else if (strstr(argv[a -1], ":semilla") != NULL) {
				strcpy(strInicializacion, "semilla");
				inicializacion = INICIALIZACION_SEMILLA;
			}
		}
		else if (strstr(argv[a -1], "regla:") == argv[a - 1]) {
			if (strstr(argv[a -1], ":todas") != NULL) 
				regla = -1;
			else {
				regla = atoi(argv[a - 1] + strlen("regla:"));
				if (regla < 0 || regla > 255 || errno != 0) {
					regla = REGLA;
					printf("Par�metro incorrecto, se esperaba una regla entre 0 y 255... Se asume %d\n", regla);
				}
			}
		}
		else if (strstr(argv[a -1], "celdas:") == argv[a - 1]) {
			celdas = atoi(argv[a - 1] + strlen("celdas:"));
			if (celdas <= MIN_CELDAS || celdas >= MAX_CELDAS || errno != 0) {
				celdas = CELDAS;
				printf("Par�metro incorrecto, se esperaba un n�mero de celdas entre %d y %d... Se asumen %d celdas\n", MIN_CELDAS, MAX_CELDAS, celdas);
			}
		}
		else if (strstr(argv[a -1], "pasos:") == argv[a - 1]) {
			pasos = atoi(argv[a - 1] + strlen("pasos:"));
			if (pasos <= MIN_PASOS || pasos >= MAX_PASOS || errno != 0) {
				pasos = PASOS;
				printf("Par�metro incorrecto, se esperaba un n�mero de pasos entre %d y %d... Se asumen %d pasos\n", MIN_PASOS, MAX_PASOS, pasos);
			}
		}
	}

	if (regla == TODAS_LAS_REGLAS) {
		for (int r = MIN_REGLA; r <= MAX_REGLA; r++) { 
			/* definimos la condici�n inicial de nuestro ACE y asignamos la memoria necesaria din�micamente */
			inicializarACE(&ACE, pasos, celdas, inicializacion);

			generarACE(ACE, r, pasos, celdas);
	
			char nombreFichero[256];
			sprintf(nombreFichero, "ACE_R%03d_%s.pgm", r, strInicializacion);
			guardaPGMi(nombreFichero, pasos + 1, celdas + 2, ACE, 1, 0);
		}
	}
	else {
		/* definimos la condici�n inicial de nuestro ACE y asignamos la memoria necesaria din�micamente */
		inicializarACE(&ACE, pasos, celdas, inicializacion);

		generarACE(ACE, regla, pasos, celdas);
	
		char nombreFichero[256];
		sprintf(nombreFichero, "ACE_R%03d_%s.pgm", regla, strInicializacion);
		guardaPGMi(nombreFichero, pasos + 1, celdas + 2, ACE, 1, 0);
	}
}
