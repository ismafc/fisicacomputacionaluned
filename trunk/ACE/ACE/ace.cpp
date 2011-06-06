#include <time.h>
#include "libguardaimagen.h"
#include "libACE.h"

#pragma warning ( disable: 4996 )			// Evita los warnings del compilador de funciones obsoletas

#define REGLA						54		// regla a aplicar por defecto
#define CELDAS						1000	// n�mero de celdas del ACE por defecto
#define PASOS						500		// n�mero de pasos de evoluci�n por defecto

#define TODAS_LAS_REGLAS			-1		// valor de regla que indica que hay que calcular todas las reglas
#define MAX_REGLAS					256		// N�mero m�ximo de reglas a calcular

#define MIN_PASOS					1		// como m�nimo 1 paso de evoluci�n
#define MAX_PASOS					5000	// como m�ximo 5000 pasos de evoluci�n

#define MIN_CELDAS					2		// como m�nimo 2 celdas en el ACE
#define MAX_CELDAS					10000	// como m�ximo 10000 celdas en el ACE

/*
 * Nombre: ACE (Aut�mata Celular Elemental)
 * Autor: Ismael Flores Campoy
 * Descripci�n: Genera la simulaci�n de la evoluci�n de aut�matas celulares elementales
 * Sintaxis: ACE <opcion1>:<valor1> <opcion2>:<valor2> ...
 *
 * Opci�n					| Valores (separados por comas)		| Valor por defecto
 * -------------------------------------------------------------------------------------------------------
 * inicializacion			| aleatoria, semilla				| semilla
 * reglas					| [0, 255], todas					| REGLA (54)
 * pasos					| [1, 5000]							| PASOS (500)
 * celdas					| [2, 10000]						| CELDAS (1000)
 *
 * Argumento					| Significado
 * ------------------------------------------------------------------------------------------------------------
 * inicializacion:aleatoria		| La primera fila del ACE contiene una sucesi�n aleatoria de '0' y '1'
 * inicializacion:semilla		| La primera fila del ACE contiene todo '0' menos un '1' en la posici�n central
 * reglas:todas					| Se calculan los ACEs (y se guardan en ficheros) de todas las reglas [0, 255]
 * reglas:4						| Se calcula el ACE (y se guarda en ficheros) de la regla 4
 * reglas:4,90,126				| Se calcula el ACE (y se guarda en ficheros) de laS reglas 4, 90 y 126
 * pasos:300					| Se calculan 300 pasos de la evoluci�n del ACE
 * celdas:700					| El ACE lo conforman 700 posiciones
 * 
 * Ejemplos:
 *
 * ACE reglas:126,90
 * ACE inicializacion:aleatoria
 * ACE reglas:todas celdas:1000 pasos:500
 * ACE reglas:4,126 pasos:200 celdas:200
 *
 */
int main(int argc, char** argv)
{
	int celdas = CELDAS;							// Celdas del ACE (por defecto CELDAS)
	int pasos = PASOS;								// Pasos de evoluci�n a simular (por defecto PASOS)
	int inicializacion = INICIALIZACION_SEMILLA;	// Por defecto la inicializaci�n es por semilla ACE[0][CELDAS /2 + 1]=1
	int** ACE;										// Donde guardamos el estado del aut�mata [PASOS + 1][CELDAS + 2]
	char strInicializacion[32];						// Guardamos el tipo de inicializaci�n para generar el nombre del fichero
	int reglas[MAX_REGLAS];							// Guardamos las reglas a aplicar
	int nreglas;									// Guardamos el n�mero de reglas a aplicar
	char nombreFichero[256];						// Guardaremos los nombres de los ficheros a crear

	// Inicializamos el texto de la inicializaci�n del ACE como "semilla" (se usar� para el nombre del fichero PGM en el que se guardan los resultados)
	strcpy(strInicializacion, "semilla");

	// Inicializamos la semilla de los n�meros aleatorios
	srand((unsigned int)time(NULL));

	// Por defecto aplicaremos la regla 'REGLA' si como argumento no indicamos otra cosa
	reglas[0] = REGLA;
	nreglas = 1;

	// Procesado de los par�metros de entrada (si existen)
	for (int a = 1; a < argc; a++) {
		if (strstr(argv[a], "inicializacion:") == argv[a]) {
			// Si encontramos un argumento 'inicializaci�n:' analizamos que valor tiene.
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
				printf("Par�metro incorrecto, se esperaba un n�mero de celdas entre %d y %d... Se asumen %d celdas\n", MIN_CELDAS, MAX_CELDAS, celdas);
			}
		}
		else if (strstr(argv[a], "pasos:") == argv[a]) {
			// Si encontramos un argumento 'pasos:' analizamos que valor tiene.
			pasos = atoi(argv[a] + strlen("pasos:"));
			if (pasos < MIN_PASOS || pasos > MAX_PASOS || errno != 0) {
				pasos = PASOS;
				printf("Par�metro incorrecto, se esperaba un n�mero de pasos entre %d y %d... Se asumen %d pasos\n", MIN_PASOS, MAX_PASOS, pasos);
			}
		}
	}

	// Asignamos la memoria necesaria din�micamente
	asignarMemoriaACE(&ACE, pasos, celdas);

	// Definimos la condici�n inicial de nuestro ACE 
	inicializarACE(ACE, celdas, inicializacion);

	// Para cada regla generamos la evoluci�n del ACE y lo guardamos
	for (int nr = 0; nr < nreglas; nr++) { 

		// Generamos nuestro ACE 
		generarACE(ACE, reglas[nr], pasos, celdas);

		// Guardamos nuestro ACE 
		sprintf(nombreFichero, "ACE_R%03d_C%05d_P%05d_%s.pgm", reglas[nr], celdas, pasos, strInicializacion);
		guardaPGMiACE(nombreFichero, pasos, celdas, ACE, 1, 0);

	}

	// Liberamos la memoria
	liberarMemoriaACE(ACE, pasos);
}
