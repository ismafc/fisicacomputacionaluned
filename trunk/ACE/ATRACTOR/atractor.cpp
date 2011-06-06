#include <time.h>
#include "libguardaimagen.h"
#include "libACE.h"

#pragma warning ( disable: 4996 )

#define REGLA						54		// regla a aplicar por defecto
#define CELDAS						10		// n�mero de celdas del ACE por defecto
#define PASOS						20		// n�mero de pasos de evoluci�n por defecto

#define TODAS_LAS_REGLAS			-1		// valor de regla que indica que hay que calcular todas las reglas
#define MAX_REGLAS					256		// N�mero m�ximo de reglas a calcular

#define MIN_PASOS					1		// como m�nimo 1 paso de evoluci�n
#define MAX_PASOS					5000	// como m�ximo 5000 pasos de evoluci�n

#define MIN_CELDAS					2		// como m�nimo 2 celdas en el ACE
#define MAX_CELDAS					10000	// como m�ximo 10000 celdas en el ACE

/*
 * Nombre: ATRACTOR (Estudio de los atractores de Aut�matas Celulares Elementales)
 * Autor: Ismael Flores Campoy
 * Descripci�n: Genera informaci�n a prop�sito de los atractores extra�os en los ACEs.
 *				Para cada paso, guardar� cuantas veces se visit� cada estado posible.
 *				Para cada paso, obtendremos la relaci�n entre el n�mero de estados diferentes visitados y el n�mero de estados posibles [0..10000]
 *              ya que guardaremos un entero representando un % con dos decimales de precisi�n (multiplicado por 100)
 *				Para cada estado, guardaremos la probabilidad de ser visitado en cualquier paso durante cualquier simulaci�n [0..10000]
 *				ya que guardaremos un entero representando un % con dos decimales de precisi�n (multiplicado por 100)
 *				Para cada paso cuardaremos la entropia
 * Sintaxis: ATRACTOR <opcion1>:<valor1> <opcion2>:<valor2> ...
 *
 * Opci�n					| Valores (separados por comas)		| Valor por defecto
 * -------------------------------------------------------------------------------------------------------
 * reglas					| [0, 255], todas					| REGLA (54)
 * pasos					| [1, 5000]							| PASOS (500)
 * celdas					| [2, 10000]						| CELDAS (1000)
 *
 * Argumento					| Significado
 * ------------------------------------------------------------------------------------------------------------
 * reglas:todas					| Se calculan los ACEs (y se guardan en ficheros) de todas las reglas [0, 255]
 * reglas:4						| Se calcula el ACE (y se guarda en ficheros) de la regla 4
 * reglas:4,90,126				| Se calcula el ACE (y se guarda en ficheros) de laS reglas 4, 90 y 126
 * pasos:300					| Se calculan 300 pasos de la evoluci�n del ACE
 * celdas:700					| El ACE lo conforman 700 posiciones
 * 
 * Ejemplos:
 *
 * ATRACTOR reglas:126,90
 * ATRACTOR reglas:4 pasos:20 celdas:10
 *
 */
int main(int argc, char** argv)
{
	int celdas = CELDAS;							// Celdas del ACE (por defecto CELDAS)
	int pasos = PASOS;								// Pasos de evoluci�n a simular (por defecto PASOS)
	int** ACE;										// Donde guardamos el estado del aut�mata [PASOS + 1][CELDAS + 2]
	int reglas[256];								// Guardamos las reglas a aplicar
	int nreglas;									// Guardamos el n�mero de reglas a aplicar
	char nombreFichero[256];						// Guardamos los nombres de los ficheros a crear
	int** probabilidades;							// Guardamos cuantas veces se visit� cada estado posible en cada paso.
	int* visitadosPaso;								// Guardamos el n�mero de estados diferentes visitados en cada paso.
	int* estadoVisitado;							// Guardamos las veces que ha sido visitado cada estado en cualquier paso de la simulaci�n.
	int estadosPosibles;							// N�mero de estados diferentes posibles en un ACE (2^celdas)

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

	// Asignamos la memoria necesaria para almacenar el ACE
	asignarMemoriaACE(&ACE, pasos, celdas);

	// Asignamos la memoria necesaria para almacenar un primer estado del ACE de 'celdas' celdas
	int* base = new int [celdas + 2];

	estadosPosibles = (int)pow(2.0, celdas);

	for (int nr = 0; nr < nreglas; nr++) {
		inicializarAtractores(&probabilidades, &visitadosPaso, &estadoVisitado, pasos, estadosPosibles);
		for (int estado = 0; estado < estadosPosibles; estado++) {
			generarEstadoInicial(base, estado, celdas);

			inicializarACE(ACE, celdas, INICIALIZACION_FIJA, base);

			probabilidades[0][estado]++;
			visitadosPaso[0]++;
			estadoVisitado[estado]++;
			long* estados = generarACE(ACE, reglas[nr], pasos, celdas);

			// probabilidades: Actualizamos las veces que cada estado es visitado en cada paso
			// visitadosPaso: Actualizamos el n�mero de estados diferentes visitados en cada paso
			// estadoVisitado: Actualizamos el n�mero de veces que un estado ha sido visitado
			//
			// Si 'probabilidades' tiene un puntero v�lido se supone que tiene las dimensiones correctas: (pasos + 1)*(2^celdas)
			// Si 'visitados' tiene un puntero v�lido se supone que tiene las dimensiones correctas (pasos + 1)
			for (int p = 0; p < pasos; p++) {
				// Actualizamos las probabilidades de caer en el 'estado' en el paso 'p'
				probabilidades[p + 1][estados[p]]++;

				// Actualizamos el n�mero de estados diferentes visitados en el paso 'p'
				if (probabilidades[p + 1][estados[p]] == 1)
					visitadosPaso[p + 1]++;

				// Actualizamos el n�mero de visitas a dicho estado
				estadoVisitado[estados[p]]++;
			}

			// Liberamos la memoria donde se guard� el estado de cada paso
			delete[] estados;

		}

		// Guardamos los resultados
		sprintf(nombreFichero, "ATRACTOR_R%03d_C%05d_P%05d.dat", reglas[nr], celdas, pasos);
		guardarAtractorPLOT(nombreFichero, probabilidades, pasos, estadosPosibles);

		sprintf(nombreFichero, "ATRACTOR_VISITADO_PASO_R%03d_C%05d_P%05d.dat", reglas[nr], celdas, pasos);
		guardaPLOT(nombreFichero, visitadosPaso, pasos + 1);

		sprintf(nombreFichero, "ATRACTOR_ESTADO_VISITADO_R%03d_C%05d_P%05d.dat", reglas[nr], celdas, pasos);
		guardaPLOT(nombreFichero, estadoVisitado, estadosPosibles);

		// Evoluci�n de la entropia en el tiempo
		double* entropias = new double [pasos + 1];
		for (int p = 0; p < pasos + 1; p++)
			entropias[p] = entropia(probabilidades[p], celdas);
		sprintf(nombreFichero, "ENTROPIA_R%03d_C%05d_P%05d.dat", reglas[nr], celdas, pasos);
		guardaPLOT(nombreFichero, entropias, pasos + 1);
		delete[] entropias;

		// Liberamos la memoria para los datos de los atractores
		liberarAtractores(probabilidades, visitadosPaso, estadoVisitado, pasos);
	}

	// Liberamos la memoria para la simulaci�n del ACE
	liberarMemoriaACE(ACE, pasos);

	// Liberamos la memoria necesaria para el primer estado del ACE de 'celdas' celdas
	delete[] base;
}
