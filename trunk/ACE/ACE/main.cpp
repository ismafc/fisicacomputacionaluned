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
#define INICIALIZACION_SIMILAR		2		// Se inicializa con la primera fila similar a otra pero cambiado s�lo el valor central negado

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

void inicializarACE(int*** ACE, int pasos, int celdas, int inicializacion = INICIALIZACION_SEMILLA, int* base = NULL)
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
	else if (inicializacion == INICIALIZACION_SIMILAR) {
		for (i = 0; i < celdas + 2; i++)
			(*ACE)[0][i] = base[i];
		(*ACE)[0][(celdas + 1) / 2] = ((*ACE)[0][(celdas + 1) / 2] == 1) ? 0 : 1;
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

int* generarHamming(int** ACE, int regla, int pasos, int celdas)
{
	int** ACE1;
	int* hamming;
	
	inicializarACE(&ACE1, pasos, celdas, INICIALIZACION_SIMILAR, ACE[0]);
	generarACE(ACE1, regla, pasos, celdas);

	hamming = new int [pasos + 1];
	memset(hamming, 0, (pasos + 1) * sizeof(int));
	hamming[0] = 1;
	for (int i = 1; i < pasos + 1; i++) 
	{
		for (int j = 1; j < celdas + 1; j++)
		{
			hamming[i] += (ACE[i][j] == ACE1[i][j] ? 0 : 1);
		}
	}
	return hamming;
}

/*
 * Nombre: ACE (Aut�mata Celular Elemental)
 * Autor: Ismael Flores Campoy
 * Fecha: 28/04/2011
 * Descripci�n: Genera informaci�n a prop�sito de la evoluci�n de aut�matas celulares elementales
 * Sintaxis: ACE <opcion1>:<valor1> <opcion2>:<valor2> ...
 *
 * Opci�n					| Valores (separados por comas)		| Valor por defecto
 * -------------------------------------------------------------------------------------------------------
 * inicializacion			| aleatoria, semilla				| semilla
 * hamming					| si								| no
 * regla					| [0, 255], todas					| REGLA (54)
 * pasos					| [1, 5000]							| PASOS (500)
 * celdas					| [2, 10000]						| CELDAS (1000)
 *
 * Argumento					| Significado
 * ------------------------------------------------------------------------------------------------------------
 * inicializacion:aleatoria		| La primera fila del ACE contiene una sucesi�n aleatoria de '0' y '1'
 * inicializacion:semilla		| La primera fila del ACE contiene todo '0' menos un '1' en la posici�n central
 * hamming:si					| Se calcula la evoluci�n de la distancia de hamming en el tiempo
 *								| entre el ACE calculado y otro que difiere �nicamente en el valor central 
 *								| de la primera fila. Cada evoluci�n calculada se guarda en un fichero.
 * regla:todas					| Se calculan los ACEs (y se guardan en ficheros) de todas las reglas [0, 255]
 * regla:4						| Se calcula el ACE (y se guarda en ficheros) de la regla 4
 * pasos:300					| Se calculan 300 pasos de la evoluci�n del ACE
 * celdas:700					| El ACE lo conforman 700 posiciones
 * 
 * Ejemplos:
 *
 * ACE regla:126
 * ACE hamming:si inicializacion:aleatoria
 * ACE regla:todas
 * ACE regla:4 hamming:si pasos:200 celdas:200
 *
 */
int main(int argc, char** argv)
{
	int regla = REGLA;								// Regla a aplicar (por defecto REGLA)
	int celdas = CELDAS;							// Celdas del ACE (por defecto CELDAS)
	int pasos = PASOS;								// Pasos de evoluci�n a simular (por defecto PASOS)
	int inicializacion = INICIALIZACION_SEMILLA;	// Por defecto la inicializaci�n es por semilla ACE[0][CELDAS/2]=1
	int** ACE;										// Donde guardamos el estado del aut�mata [T + 1][N + 2]
	char strInicializacion[32];						// Guardamos el tipo de inicializaci�n para generar el nombre del fichero
	bool hamming = false;							// Guardamos si hay que calcular la evoluci�n de la distancia de hamming

	// Inicializamos el texto de la inicializaci�n del ACE como "semilla" (se usar� para el nombre del fichero PGM en el que se guardan los resultados)
	strcpy(strInicializacion, "semilla");

	// Inicializamos la semilla de los n�meros aleatorios
	srand((unsigned int)time(NULL));

	// Procesado de los par�metros de entrada (si existen)
	for (int a = 2; a <= argc; a++) {
		if (strstr(argv[a -1], "inicializacion:") == argv[a - 1]) {
			// Si encontramos un argumento 'inicializaci�n:' analizamos que valor tiene.
			if (strstr(argv[a -1], ":aleatoria") != NULL) {
				strcpy(strInicializacion, "aleatoria");
				inicializacion = INICIALIZACION_ALEATORIA;
			}
			else if (strstr(argv[a -1], ":semilla") != NULL) {
				strcpy(strInicializacion, "semilla");
				inicializacion = INICIALIZACION_SEMILLA;
			}
		}
		else if (strstr(argv[a -1], "hamming:") == argv[a - 1]) {
			// Si encontramos un argumento 'hamming:' analizamos que valor tiene.
			if (strstr(argv[a -1], ":si") != NULL)
				hamming = true;
		}
		else if (strstr(argv[a -1], "regla:") == argv[a - 1]) {
			// Si encontramos un argumento 'regla:' analizamos que valor tiene.
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
			// Si encontramos un argumento 'celdas:' analizamos que valor tiene.
			celdas = atoi(argv[a - 1] + strlen("celdas:"));
			if (celdas <= MIN_CELDAS || celdas >= MAX_CELDAS || errno != 0) {
				celdas = CELDAS;
				printf("Par�metro incorrecto, se esperaba un n�mero de celdas entre %d y %d... Se asumen %d celdas\n", MIN_CELDAS, MAX_CELDAS, celdas);
			}
		}
		else if (strstr(argv[a -1], "pasos:") == argv[a - 1]) {
			// Si encontramos un argumento 'pasos:' analizamos que valor tiene.
			pasos = atoi(argv[a - 1] + strlen("pasos:"));
			if (pasos <= MIN_PASOS || pasos >= MAX_PASOS || errno != 0) {
				pasos = PASOS;
				printf("Par�metro incorrecto, se esperaba un n�mero de pasos entre %d y %d... Se asumen %d pasos\n", MIN_PASOS, MAX_PASOS, pasos);
			}
		}
	}

	char nombreFichero[256];
	int reglaInicial = (regla == TODAS_LAS_REGLAS) ? MIN_REGLA : regla;
	int reglaFinal = (regla == TODAS_LAS_REGLAS) ? MAX_REGLA : regla;
	for (int r = reglaInicial; r <= reglaFinal; r++) { 
		// definimos la condici�n inicial de nuestro ACE y asignamos la memoria necesaria din�micamente
		inicializarACE(&ACE, pasos, celdas, inicializacion);

		generarACE(ACE, r, pasos, celdas);

		sprintf(nombreFichero, "ACE_R%03d_%s.pgm", r, strInicializacion);
		guardaPGMiACE(nombreFichero, pasos + 1, celdas + 2, ACE, 1, 0);

		if (hamming) {
			int* distanciasHamming = generarHamming(ACE, r, pasos, celdas);

			sprintf(nombreFichero, "HAMMING_R%03d.dat", r);
			guardaPLOT(nombreFichero, distanciasHamming, pasos + 1);			
		}
	}
}
