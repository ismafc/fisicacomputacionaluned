#include <time.h>
#include "libguardaimagen.h"
#include "libACE.h"

#pragma warning ( disable: 4996 )

#define REGLA						54		// regla a aplicar por defecto
#define PASOS						20		// n�mero de pasos de evoluci�n por defecto

#define TODAS_LAS_REGLAS			-1		// valor de regla que indica que hay que calcular todas las reglas
#define MAX_REGLAS					256		// N�mero m�ximo de reglas a calcular

#define MIN_PASOS					1		// como m�nimo 1 paso de evoluci�n
#define MAX_PASOS					5000	// como m�ximo 5000 pasos de evoluci�n

#define N_MIN						3		// En evoluciones por n�mero de celdas, valor m�nimo
#define N_MAX						20		// En evoluciones por n�mero de celdas, valor m�ximo

/*
 * Nombre: ENTROPIA (Estudio de la evoluci�n de la entropia)
 * Autor: Ismael Flores Campoy
 * Descripci�n: Genera informaci�n a prop�sito de la evoluci�n de aut�matas celulares elementales
 * Sintaxis: ACE <opcion1>:<valor1> <opcion2>:<valor2> ...
 *
 * Opci�n					| Valores (separados por comas)		| Valor por defecto
 * -------------------------------------------------------------------------------------------------------
 * regla					| [0, 255], todas					| REGLA (54)
 * pasos					| [1, 5000]							| PASOS (500)
 * celdas					| [2, 10000]						| CELDAS (1000)
 *
 * Argumento					| Significado
 * ------------------------------------------------------------------------------------------------------------
 * regla:todas					| Se calculan los ACEs (y se guardan en ficheros) de todas las reglas [0, 255]
 * regla:4						| Se calcula el ACE (y se guarda en ficheros) de la regla 4
 * regla:4,90,126				| Se calcula el ACE (y se guarda en ficheros) de laS reglas 4, 90 y 126
 * pasos:300					| Se calculan 300 pasos de la evoluci�n del ACE
 * celdas:700					| El ACE lo conforman 700 posiciones
 * 
 * Ejemplos:
 *
 * ENTROPIA regla:126,90
 * ENTROPIA regla:todas
 * ENTROPIA regla:4 pasos:200 celdas:200
 *
 */
int main(int argc, char** argv)
{
	int pasos = PASOS;								// Pasos de evoluci�n a simular (por defecto PASOS)
	int** ACE;										// Donde guardamos el estado del aut�mata [PASOS + 1][CELDAS + 2]
	int reglas[256];								// Guardamos las reglas a aplicar
	int nreglas;									// Guardamos el n�mero de reglas a aplicar
	char nombreFichero[256];						// Guardamos los nombres de los ficheros a crear

	// Inicializamos la semilla de los n�meros aleatorios
	srand((unsigned int)time(NULL));

	// Por defecto aplicaremos la regla REGLA si como argumento no indicamos otra cosa
	reglas[0] = REGLA;
	nreglas = 1;

	// Procesado de los par�metros de entrada (si existen)
	for (int a = 1; a < argc; a++) {
		if (strstr(argv[a], "reglas:") == argv[a]) {
			// Si encontramos un argumento 'regla:' analizamos que valor tiene.
			if (strstr(argv[a], ":todas") != NULL)
				nreglas = obtenerValores(reglas, MAX_REGLAS, "0-255");
			else
				nreglas = obtenerValores(reglas, MAX_REGLAS, argv[a] + strlen("reglas:"));
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

	double* noVisitados = new double [N_MAX - N_MIN + 1];
	double* entropias = new double [N_MAX - N_MIN + 1];
	for (int nr = 0; nr < nreglas; nr++) { 
		for (int N = N_MIN; N <= N_MAX; N++) {
			int estadosPosibles = (int)pow(2.0, N);

			int visitadosPaso = 0;
			int* probabilidades;
			probabilidades = new int [estadosPosibles];
			memset(probabilidades, 0, estadosPosibles * sizeof(int));
			int* base = new int [N + 2];

			asignarMemoriaACE(&ACE, pasos, N);

			for (int estado = 0; estado < estadosPosibles; estado++) {
				generarEstadoInicial(base, estado, N);
				inicializarACE(ACE, N, INICIALIZACION_FIJA, base);

				long* estados = generarACE(ACE, reglas[nr], pasos, N);

				// probabilidades: Actualizamos las veces que cada estado es visitado en el �ltimo paso
				// visitadosPaso: Actualizamos el n�mero de estados diferentes visitados en el �ltimo paso
				// estadoVisitado: Actualizamos el n�mero de veces que un estado ha sido visitado en el �ltimo paso
				// Actualizamos las probabilidades de caer en el 'estado' en el paso 'p'
				probabilidades[estados[pasos - 1]]++;

				// Actualizamos el n�mero de estados diferentes visitados en el paso 'p'
				if (probabilidades[estados[pasos - 1]] == 1)
					visitadosPaso++;

				delete[] estados;
			}

			liberarMemoriaACE(ACE, pasos);
			delete[] base;

			noVisitados[N - N_MIN] = (double)(estadosPosibles - visitadosPaso) / (double)estadosPosibles;
			entropias[N - N_MIN] = entropia(probabilidades, N);

			delete[] probabilidades;
		}

		sprintf(nombreFichero, "NOVISITADOS_R%03d_P%05d.dat", reglas[nr], pasos);
		guardaPLOT(nombreFichero, noVisitados,  N_MAX - N_MIN + 1, N_MIN, 5);

		sprintf(nombreFichero, "ENTROPIA_R%03d_P%05d.dat", reglas[nr], pasos);
		guardaPLOT(nombreFichero, entropias, N_MAX - N_MIN + 1, N_MIN);

	}
	delete[] noVisitados;
	delete[] entropias;
}
