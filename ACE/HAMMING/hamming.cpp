#include <time.h>
#include "libguardaimagen.h"
#include "libACE.h"

#pragma warning ( disable: 4996 )

#define REGLA						54		// regla a aplicar por defecto
#define CELDAS						1000	// número de celdas del ACE por defecto
#define PASOS						500		// número de pasos de evolución por defecto

#define TODAS_LAS_REGLAS			-1		// valor de regla que indica que hay que calcular todas las reglas
#define MAX_REGLAS					256		// Número máximo de reglas a calcular

#define MIN_PASOS					1		// como mínimo 1 paso de evolución
#define MAX_PASOS					5000	// como máximo 5000 pasos de evolución

#define MIN_CELDAS					2		// como mínimo 2 celdas en el ACE
#define MAX_CELDAS					10000	// como máximo 10000 celdas en el ACE

/*
 * Nombre: HAMMING (Estudio distancia de Hamming de Autómatas Celulares Elementales)
 * Autor: Ismael Flores Campoy
 * Descripción: Genera información a propósito de la distancia de Hamming.
 *              Se calcula la evolución de la distancia de hamming en el tiempo entre el ACE calculado 
 *				y otro que difiere únicamente en el valor central de la primera fila.
 *              Cada evolución calculada se guarda en un fichero.
 * Sintaxis: HAMMING <opcion1>:<valor1> <opcion2>:<valor2> ...
 *
 * Opción					| Valores (separados por comas)		| Valor por defecto
 * -------------------------------------------------------------------------------------------------------
 * inicializacion			| aleatoria, semilla				| semilla
 * reglas					| [0, 255], todas					| REGLA (54)
 * pasos					| [1, 5000]							| PASOS (500)
 * celdas					| [2, 10000]						| CELDAS (1000)
 *
 * Argumento					| Significado
 * ------------------------------------------------------------------------------------------------------------
 * inicializacion:aleatoria		| La primera fila del ACE contiene una sucesión aleatoria de '0' y '1'
 * inicializacion:semilla		| La primera fila del ACE contiene todo '0' menos un '1' en la posición central
 * reglas:todas					| Se calculan los ACEs (y se guardan en ficheros) de todas las reglas [0, 255]
 * reglas:4						| Se calcula el ACE (y se guarda en ficheros) de la regla 4
 * reglas:4,90,126				| Se calcula el ACE (y se guarda en ficheros) de laS reglas 4, 90 y 126
 * pasos:300					| Se calculan 300 pasos de la evolución del ACE
 * celdas:700					| El ACE lo conforman 700 posiciones
 * 
 * Ejemplos:
 *
 * HAMMING reglas:126,90
 * HAMMING inicializacion:aleatoria
 * HAMMING reglas:todas celdas:500
 * HAMMING reglas:4 pasos:200 celdas:200
 *
 */
int main(int argc, char** argv)
{
	int celdas = CELDAS;							// Celdas del ACE (por defecto CELDAS)
	int pasos = PASOS;								// Pasos de evolución a simular (por defecto PASOS)
	int inicializacion = INICIALIZACION_SEMILLA;	// Por defecto la inicialización es por semilla ACE[0][CELDAS /2 + 1]=1
	int** ACE;										// Donde guardamos el estado del autómata [PASOS + 1][CELDAS + 2]
	char strInicializacion[32];						// Guardamos el tipo de inicialización para generar el nombre del fichero
	int reglas[256];								// Guardamos las reglas a aplicar
	int nreglas;									// Guardamos el número de reglas a aplicar
	char nombreFichero[256];						// Guardamos los nombres de los ficheros a crear
	int* distanciasHamming;							// Vector en el que guardamos las distancias de hamming para cada paso
	double eh;										// Guardamos el exponente de hamming calculado por el programa

	// Inicializamos el texto de la inicialización del ACE como "semilla" (se usará para el nombre del fichero PGM en el que se guardan los resultados)
	strcpy(strInicializacion, "semilla");

	// Inicializamos la semilla de los números aleatorios
	srand((unsigned int)time(NULL));

	// Por defecto aplicaremos la regla REGLA si como argumento no indicamos otra cosa
	reglas[0] = REGLA;
	nreglas = 1;

	// Procesado de los parámetros de entrada (si existen)
	for (int a = 1; a < argc; a++) {
		if (strstr(argv[a], "inicializacion:") == argv[a]) {
			// Si encontramos un argumento 'inicialización:' analizamos que valor tiene.
			if (strstr(argv[a], ":aleatoria") != NULL) {
				strcpy(strInicializacion, "aleatoria");
				inicializacion = INICIALIZACION_ALEATORIA;
			}
			else if (strstr(argv[a], ":semilla") != NULL) {
				strcpy(strInicializacion, "semilla");
				inicializacion = INICIALIZACION_SEMILLA;
			}
		}
		else if (strstr(argv[a], "reglas:") == argv[a]) {
			// Si encontramos un argumento 'regla:' analizamos que valor tiene.
			if (strstr(argv[a], ":todas") != NULL)
				nreglas = obtenerValores(reglas, MAX_REGLAS, "0-255");
			else
				nreglas = obtenerValores(reglas, MAX_REGLAS, argv[a] + strlen("reglas:"));
		}
		else if (strstr(argv[a], "celdas:") == argv[a]) {
			// Si encontramos un argumento 'celdas:' analizamos que valor tiene.
			celdas = atoi(argv[a] + strlen("celdas:"));
			if (celdas < MIN_CELDAS || celdas > MAX_CELDAS || errno != 0) {
				celdas = CELDAS;
				printf("Parámetro incorrecto, se esperaba un número de celdas entre %d y %d... Se asumen %d celdas\n", MIN_CELDAS, MAX_CELDAS, celdas);
			}
		}
		else if (strstr(argv[a], "pasos:") == argv[a]) {
			// Si encontramos un argumento 'pasos:' analizamos que valor tiene.
			pasos = atoi(argv[a] + strlen("pasos:"));
			if (pasos < MIN_PASOS || pasos > MAX_PASOS || errno != 0) {
				pasos = PASOS;
				printf("Parámetro incorrecto, se esperaba un número de pasos entre %d y %d... Se asumen %d pasos\n", MIN_PASOS, MAX_PASOS, pasos);
			}
		}
	}

	// Asignamos la memoria necesaria dinámicamente
	asignarMemoriaACE(&ACE, pasos, celdas);

	// Definimos la condición inicial de nuestro ACE 
	inicializarACE(ACE, celdas, inicializacion);

	// Para cada regla generamos el ACE y la información sobre la distancia de Hamming entre este y otro
	// que únicamente se diferencia del mismo en el valor central de la primera fila (paso 0)
	for (int nr = 0; nr < nreglas; nr++) {

		// Generamos la evolución de nuestro ACE base
		generarACE(ACE, reglas[nr], pasos, celdas);

		// Generamos la información sobre la evolución de las distancias de Hamming
		distanciasHamming = generarHamming(ACE, reglas[nr], pasos, celdas);

		// Guardamos la información en un fichero
		sprintf(nombreFichero, "HAMMING_R%03d_C%05d_P%05d.dat", reglas[nr], celdas, pasos);
		guardaPLOT(nombreFichero, distanciasHamming, pasos + 1);

		// Calculamos el exponente de Hamming mediante la regresión de los puntos de las distancias
		// y mostramos el resultado ppor consola
		if (exponenteHamming(distanciasHamming, pasos + 1, eh))
			printf("El exponente de hamming (R%03d,C%05d,P%05d) es %.5f\n", reglas[nr], celdas, pasos, eh);
		else
			printf("No se pudo calcular el exponente de hamming (R%03d,C%05d,P%05d)\n", reglas[nr], celdas, pasos, eh);

		delete[] distanciasHamming;
	}

	// Liberamos la memoria
	liberarMemoriaACE(ACE, pasos);
}
