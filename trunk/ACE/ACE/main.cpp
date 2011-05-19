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

/*
 * Nombre: regresion
 *
 * Descripci�n: Calcula la recta de regresi�n correspondiente a los puntos proporcionados como par�metros.
 *				Genera la pendiente 'my', la ordenada de origen 'y0' y el coeficiente de correlaci�n 'r'.
 *
 * puntosx: Vector con las coordenadas X de los puntos.
 * puntosy: Vector con las coordenadas Y de los puntos.
 * npuntos: N�mero de puntos.
 * my: Pendiente de la recta de regresi�n Y = my * X + y0.
 * y0: Ordenada de origen de la recta de regresi�n Y = my * X + y0.
 * r: Coeficiente de correlaci�n r^2 = mx * my.
 *
 * Devuelve, por referencia, los valores que definen la recta de regresi�n correspondiente a los puntos proporcionados en los par�metros de entrada.
 * Devuelve un booleano indicando si el proceso se ha completado correctamente o no.
 * Se supone que los vectores 'puntosx' y 'puntosy' contiene 'npuntos' valores.
 *
 */
bool regresion(const double* puntosx, const double* puntosy, int npuntos, double& my, double& y0, double& r)
{
	double mx = 0.0;
	double sumx = 0.0;
	double sumy = 0.0;
	double sumxy = 0.0;
	double sumx2 = 0.0;
	double sumy2 = 0.0;
	double denominadormx;
	double denominadormy;
	double numeradorm;

	if (npuntos <= 0)
		return false;

	for (int i = 0; i < npuntos; i++) {
		sumx += puntosx[i];
		sumy += puntosy[i];
		sumxy += (puntosx[i] * puntosy[i]);
		sumx2 += (puntosx[i] * puntosx[i]);
		sumy2 += (puntosy[i] * puntosy[i]);
	}

	double N = npuntos;
	denominadormy = N * sumx2 - sumx * sumx;
	if (denominadormy == 0.0)
		return false;

	denominadormx = N * sumy2 - sumy * sumy;
	if (denominadormx == 0.0)
		return false;

	numeradorm = N * sumxy - sumx * sumy;

	my = numeradorm / denominadormy;
	mx = numeradorm / denominadormx;
	y0 = (sumy - my * sumx) / N;

	if (mx * my < 0.0)
		return false;

	r = sqrt(mx * my);

	return true;
}

/*
 * Nombre: exponenteHamming
 *
 * Descripci�n: Calcula el exponente de Hamming que ajusta la evoluci�n de la distancia de Hamming entre dos ACE 
 *              a una ley de potencias Ht = t^a (siendo 'a' el exponente de Hamming que devolvemos en el par�metro de entrada).
 *              Dicho exponente corresponde con la pendiente de la recta de regresi�n de los logaritmos de los puntos.
 *
 * distanciasHamming: Vector con la evoluci�n de las distancias de Hamming entre dos ACEs.
 * pasos: N�mero de pasos de que consta la evoluci�n del ACE.
 * eh: Variable en la que devolvemos el exponente de Hamming calculado.
 *
 * Devuelve cierto si se ha podido calcular el exponente de Hamming y falso en caso contrario.
 *
 */
bool exponenteHamming(const int* distanciasHamming, int pasos, double& eh)
{
	double y0;								// Ordenada de la recta de regresi�n (x = 0)
	double r;								// Coeficiente de correlaci�n r^2 = mx * my de la recta de regresi�n
	double* puntosx = new double [pasos];	// Coordenadas X de los puntos a partir de los que se calcular� la regresi�n
	double* puntosy = new double [pasos];	// Coordenadas Y de los puntos a partir de los que se calcular� la regresi�n

	// Inicializamos los puntos a partir de las distancias de Hamming aplicando logaritmos
	for (int i = 0; i < pasos; i++) {

		// Si la distancia de Hamming es 0 las evoluciones han convergido y no tiene sentido calcular la evoluci�n
		// Las siguientes distancias ser�n 0 al aplicarse la misma regla al mismo estado de ambos ACEs (determinista)
		if (distanciasHamming[i] == 0) {
			delete[] puntosx;
			delete[] puntosy;
			return false;
		}

		// Aplicamos logaritmo a ambas coordenadas de los puntos.
		puntosx[i] = log((double)(i + 1));
		puntosy[i] = log((double)distanciasHamming[i]);
	}

	// Se calcula la recta de regresi�n sobre los puntos transformados
	// en 'eh' tendremos el exponente de Hamming, que es la pendiente de
	// la recta de regresi�n Y = eh * X + y0.
	// 'r' es el coeficiente de correlaci�n que no se usa aqu� (y0 tampoco se usa).
	bool resultado = regresion(puntosx, puntosy, pasos, eh, y0, r);

	// Liberamos memoria
	delete[] puntosx;
	delete[] puntosy;

	return resultado;
}

/*
 * Nombre: liberarACE
 *
 * Descripci�n: Liberar la memoria asignada a 'ACE'. Se requiere saber cuantos 'pasos' de evoluci�n conten�a dicho
 *              ACE para poder hacer la liberaci�n correctamente ya que hay que liberar cada fila (as� se asign� la memoria)
 *
 * ACE: Aut�mata Celular Elemental cuya memoria hay que liberar.
 * pasos: N�mero de pasos de que consta la evoluci�n del ACE.
 *
 */
void liberarACE(int** ACE, int pasos)
{
	// Liberamos cada fila o paso de evoluci�n
	for (int i = 0; i < pasos + 1; i++)
		delete[] ACE[i];

	// Liberamos la lista de punteros a las filas
	delete[] ACE;
}

/*
 * Nombre: inicializarACE
 *
 * Descripci�n: Asigna la memoria necesaria para guardar lo 'pasos' evoluci�n de un ACE con 'celdas'.
 *				Inicializa el primer paso de la evoluci�n del ACE seg�n el criterio especificado.
 *
 * ACE: Aut�mata Celular Elemental al que hay que asignar la memoria necesaria e 
 *      inicializar la priemera fila con el criterio deseado ('inicializaci�n' y 'base').
 * pasos: N�mero de pasos de que constar� la evoluci�n del ACE.
 * celdas: N�mero de celdas que tendr� el ACE.
 * inicializacion: Tipo de inicializaci�n de la primera fila (paso 0).
 * base: Si la inicializaci�n es INICIALIZACION_SIMILAR, se inicializa la primera fila con este vector
 *       modificando �nicamente el valor central neg�ndolo (0 <-> 1). Se supone que el vector tiene la misma dimensi�n
 *       que el ACE ('celdas' + 2).
 *
 * El ACE tendr� 'pasos' + 1 filas (la primera es el paso 0 o inicializaci�n) y 'celdas' + 2 columnas (las dos extras son 
 * para establecer las condiciones de contorno).
 *
 */
void inicializarACE(int*** ACE, int pasos, int celdas, int inicializacion = INICIALIZACION_SEMILLA, const int* base = NULL)
{
	// Asignamos la memoria para los pasos (lista de punteros a cada fila -vector-)
	*ACE = new int* [pasos + 1];

	// Para cada paso, asignamos la memoria para las celdas e inicializamos sus valores a 0
	for (int p = 0; p < pasos + 1; p++)
	{
		(*ACE)[p] = new int [celdas + 2];
		memset((*ACE)[p], 0, (celdas + 2) * sizeof(int));
	}

	// Inicializamos la primera fila con un 1 en la celda central
	if (inicializacion == INICIALIZACION_SEMILLA)
		(*ACE)[0][celdas / 2 + 1] = 1;
	// Inicializamos la primera fila con una distribuci�n aleatoria de 0 y 1
	else if (inicializacion == INICIALIZACION_ALEATORIA) {
		for (int c = 1; c < celdas + 1; c++)
			(*ACE)[0][c] = aleatorio(0, 1);

		// actualizamos las condiciones peri�dicas de contorno de la primera fila
		(*ACE)[0][0] = (*ACE)[0][celdas];
		(*ACE)[0][celdas + 1] = (*ACE)[0][1];
	}
	// Inicializamos la primera fila con una copia del vector 'base' cambiando el valor
	// de la celda central. Se supone que base tiene la dimensi�n adecuada y las condiciones
	// periodicas de contorno correctas.
	else if (inicializacion == INICIALIZACION_SIMILAR) {
		for (int c = 0; c < celdas + 2; c++)
			(*ACE)[0][c] = base[c];

		// Se invierte el valor de la celda central de la primera fila
		(*ACE)[0][celdas / 2 + 1] = ((*ACE)[0][celdas / 2 + 1] == 1) ? 0 : 1;
	}
}

/*
 * Nombre: generarACE
 *
 * Descripci�n: Genera la evoluci�n del aut�mata celular elemental ACE con un n�mero de celdas
 *				'celdas' durante un n�mero de pasos 'pasos' aplic�ndo la regla 'regla'.
 *
 * ACE: Aut�mata Celular Elemental sobre el que se calcular� la evoluci�n. Se supone que la
 *      primera fila del aut�mata ya est� inicializada con su estado inicial.
 * regla: Entero con la regla que se aplicar� para hacer evolucionar el ACE de entrada.
 * pasos: N�mero de pasos de que consta la evoluci�n del ACE.
 * celdas: N�mero de celdas que tiene el ACE.
 *
 * Devuelve en la variable ACE la evoluci�n del aut�mata a partir de su estado inicial
 * aplicando la regla indicada. Se supone que la variable ACE est� incializada correctamente, es decir,
 * que las dimensiones son correctas y su estado incial (primera fila) tambi�n.
 *
 */
void generarACE(int** ACE, int regla, int pasos, int celdas)
{
	int vecindad;	// Guardamos la vecindad de la celda a calcular [0-7]

	for (int i = 1; i < pasos + 1; i++)
	{
		for (int j = 1; j < celdas + 1; j++)
		{
			// La vencidad de la celda (i, j) la componen {(i-1, j-1), (i-1, j), (i-1, j+1)}
			vecindad = (ACE[i - 1][j + 1] | ACE[i - 1][j] << 1 | ACE[i - 1][j - 1] << 2);

			// Comprovamos que valor [0-1] corresponde a dicha vencidad seg�n la regla
			ACE[i][j] = (regla >> vecindad) & 1;
		}

		// actualizamos las condiciones peri�dicas de contorno
		ACE[i][0] = ACE[i][celdas];
		ACE[i][celdas + 1] = ACE[i][1];
	}
}

/*
 * Nombre: generarHamming
 *
 * Descripci�n: Genera informaci�n sobre la evoluci�n de la distancia de Hamming entre el
 *				ACE que se proporciona como par�metro y otro que evoluciona de un estado inicial
 *				en el que �nicamente difiere el valor de la posici�n central.
 *
 * ACE: Simulaci�n del Aut�mata Celular Elemental sobre el que se calcular� la distancia de Hamming
 *		creando uno muy similiar que partir� desde el estado inicial de este cambiando �nicamente
 *      la celda central del estado inicial del mismo (primera fila).
 * regla: Entero con la regla que se aplic� al ACE de entrada y hay que aplicar para 
 *		  hacer evolucionar el nuevo ACE en el tiempo.
 * pasos: N�mero de pasos de que consta el ACE de entrada y que tendr� la simulaci�n del nuevo ACE.
 * celdas: N�mero de celdas que tiene el ACE y con que constar� el nuevo ACE.
 *
 * Devuelve una lista de enteros con la evoluci�n de las distancias de Hamming entre el ACE proporcionado 
 * y el ACE que evoluciona desde un estado inicial casi id�ntico.
 *
 */
int* generarHamming(int** ACE, int regla, int pasos, int celdas)
{
	int** ACE1;		// Nueva simulaci�n
	int* hamming;	// Distancias de Hamming de cada estado (fila) entre las evoluciones de los ACE
	
	// Inicializamos el ACE1, esto es, asignamos memoria y inicializamos la primera
	// fila (estado inicial) con el estado inicial de ACE sustituyendo la celda central
	// por la inversa de tal manera que dicho estado inicial s�lo difiere en un valor, el del centro.
	inicializarACE(&ACE1, pasos, celdas, INICIALIZACION_SIMILAR, ACE[0]);

	// Simulamos el nuevo ACE1 con la regla dada
	generarACE(ACE1, regla, pasos, celdas);

	// Asignamos memoria para generar las distancias de Hamming entre ambos ACEs
	hamming = new int [pasos + 1];

	// Ponemos todos los valores de las distancias de Hamming a 0
	memset(hamming, 0, (pasos + 1) * sizeof(int));

	// La distancia de Hamming entre los estados iniciales sabemos que es 1 (el valor central de la primera fila)
	hamming[0] = 1;

	for (int i = 1; i < pasos + 1; i++) 
	{
		// Calculamos la distancia de Hamming para el paso 'i' (n�mero de diferencias)
		for (int j = 1; j < celdas + 1; j++)
		{
			hamming[i] += (ACE[i][j] == ACE1[i][j] ? 0 : 1);
		}
	}

	// Liberamos el espacio asignado din�micamente a ACE1
	liberarACE(ACE1, pasos);

	return hamming;
}

/*
 * Nombre: ACE (Aut�mata Celular Elemental)
 * Autor: Ismael Flores Campoy
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
	int inicializacion = INICIALIZACION_SEMILLA;	// Por defecto la inicializaci�n es por semilla ACE[0][CELDAS /2 + 1]=1
	int** ACE;										// Donde guardamos el estado del aut�mata [PASOS + 1][CELDAS + 2]
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

			double eh;
			if (!exponenteHamming(distanciasHamming, pasos + 1, eh))
				printf("No se pudo calcular el exponente de Hamming\n");
			else
				printf("El exponente de Hamming es %d\n", eh);

			delete[] distanciasHamming;
		}

		liberarACE(ACE, pasos);
	}
}
