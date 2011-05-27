#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "libguardaimagen.h"

#pragma warning ( disable: 4996 )

#define INICIALIZACION_SEMILLA		0		// Se inicializa con un '1' en la primera fila, en la columna central
#define INICIALIZACION_ALEATORIA	1		// Se inicializa con una distribución aleatoria de '0' y '1' en la primera fila
#define INICIALIZACION_SIMILAR		2		// Se inicializa con la primera fila similar a otra pero cambiado sólo el valor central negado
#define INICIALIZACION_FIJA			2		// Se inicializa con la primera fila proporcionada

#define REGLA						54		// regla a aplicar por defecto
#define CELDAS						1000	// número de celdas del ACE por defecto
#define PASOS						500		// número de pasos de evolución por defecto

#define TODAS_LAS_REGLAS			-1		// valor de regla que indica que hay que calcular todas las reglas

#define MIN_PASOS					1		// como mínimo 1 paso de evolución
#define MAX_PASOS					5000	// como máximo 5000 pasos de evolución

#define MIN_CELDAS					2		// como mínimo 2 celdas en el ACE
#define MAX_CELDAS					10000	// como máximo 10000 celdas en el ACE

#define N_MIN						3		// En evoluciones por número de celdas, valor mínimo
#define N_MAX						16		// En evoluciones por número de celdas, valor máximo

/*
 * Nombre: aleatorio
 *
 * Descripción: Devuelve un entero aleatorio entre 'a' y 'b' (ambos incluidos y de forma equiprobable)
 *
 * a: Valor inicial
 * b: Valor final
 *
 * La función supone que (a < b) y (RAND_MAX > 0)
 *
 */
int aleatorio(int a, int b)
{
	double denominador = b - a + 1;		// Número de posibles valores a devolver
	double numerador = RAND_MAX + 1;	// Número de posibles valores de la función 'rand'
	double resultado;					// Guardamos el índice del valor a devolver que toca [0 .. b - a]
	
	// Calculamos el índice del valor a devolver dentro del rango [a .. b]
	resultado = ((double)rand() * denominador) / numerador;

	// Desplazamos el índice para integrarnos en el rango [a .. b]
	return a + (int)resultado;
}

/*
 * Nombre: regresion
 *
 * Descripción: Calcula la recta de regresión correspondiente a los puntos proporcionados como parámetros.
 *				Genera la pendiente 'my', la ordenada de origen 'y0' y el coeficiente de correlación 'r'.
 *
 * puntosx: Vector con las coordenadas X de los puntos.
 * puntosy: Vector con las coordenadas Y de los puntos.
 * npuntos: Número de puntos.
 * my: Pendiente de la recta de regresión Y = my * X + y0.
 * y0: Ordenada de origen de la recta de regresión Y = my * X + y0.
 * r: Coeficiente de correlación r^2 = mx * my.
 *
 * Devuelve, por referencia, los valores que definen la recta de regresión correspondiente a los puntos proporcionados en los parámetros de entrada.
 * Devuelve un booleano indicando si el proceso se ha completado correctamente o no.
 * Se supone que los vectores 'puntosx' y 'puntosy' contiene 'npuntos' valores.
 *
 */
bool regresion(const double* puntosx, const double* puntosy, int npuntos, double& my, double& y0, double& r)
{
	double mx = 0.0;		// Necesario para calcular 'r'
	double sumx = 0.0;		// Sumatorio de los valores de 'puntosx'
	double sumy = 0.0;		// Sumatorio de los valores de 'puntosy'
	double sumxy = 0.0;		// Sumatorio de los valores de 'puntosx'*'puntosy'
	double sumx2 = 0.0;		// Sumatorio de los valores de 'puntosx'^2
	double sumy2 = 0.0;		// Sumatorio de los valores de 'puntosy'^2
	double denominadormx;	// Guardaremos el denominador para 'mx'
	double denominadormy;	// Guardaremos el denominador para 'my'
	double numeradorm;		// Guardaremos el numerador tanto para 'mx' como para 'my'
	double N = npuntos;		// Guardamos el número de puntos como un double

	if (npuntos <= 0)
		return false;

	// Calculamos los sumatorios
	for (int i = 0; i < npuntos; i++) {
		sumx += puntosx[i];
		sumy += puntosy[i];
		sumxy += (puntosx[i] * puntosy[i]);
		sumx2 += (puntosx[i] * puntosx[i]);
		sumy2 += (puntosy[i] * puntosy[i]);
	}

	// Calculamos el denominador para 'my' (si es cero devolvemos error)
	denominadormy = N * sumx2 - sumx * sumx;
	if (denominadormy == 0.0)
		return false;

	// Calculamos el denominador para 'mx' (si es cero devolvemos error)
	denominadormx = N * sumy2 - sumy * sumy;
	if (denominadormx == 0.0)
		return false;

	// Calculamos el numerador
	numeradorm = N * sumxy - sumx * sumy;

	// Calculamos 'mx', 'my' y 'y0'
	my = numeradorm / denominadormy;
	mx = numeradorm / denominadormx;
	y0 = (sumy - my * sumx) / N;

	// Si no podemos calcular 'r' devolvemos error
	if (mx * my < 0.0)
		return false;

	// Calculamos 'r'
	r = sqrt(mx * my);

	return true;
}

/*
 * Nombre: exponenteHamming
 *
 * Descripción: Calcula el exponente de Hamming que ajusta la evolución de la distancia de Hamming entre dos ACE 
 *              a una ley de potencias Ht = t^a (siendo 'a' el exponente de Hamming que devolvemos en el parámetro de entrada).
 *              Dicho exponente corresponde con la pendiente de la recta de regresión de los logaritmos de los puntos.
 *
 * distanciasHamming: Vector con la evolución de las distancias de Hamming entre dos ACEs.
 * pasos: Número de pasos de que consta la evolución del ACE.
 * eh: Variable en la que devolvemos el exponente de Hamming calculado.
 *
 * Devuelve cierto si se ha podido calcular el exponente de Hamming y falso en caso contrario.
 *
 */
bool exponenteHamming(const int* distanciasHamming, int pasos, double& eh)
{
	double y0;								// Ordenada de la recta de regresión (x = 0)
	double r;								// Coeficiente de correlación r^2 = mx * my de la recta de regresión
	double* puntosx = new double [pasos];	// Coordenadas X de los puntos a partir de los que se calculará la regresión
	double* puntosy = new double [pasos];	// Coordenadas Y de los puntos a partir de los que se calculará la regresión

	// Inicializamos los puntos a partir de las distancias de Hamming aplicando logaritmos
	for (int i = 0; i < pasos; i++) {

		// Si la distancia de Hamming es 0 las evoluciones han convergido y no tiene sentido calcular la evolución
		// Las siguientes distancias serán 0 al aplicarse la misma regla al mismo estado de ambos ACEs (determinista)
		if (distanciasHamming[i] == 0) {
			delete[] puntosx;
			delete[] puntosy;
			return false;
		}

		// Aplicamos logaritmo a ambas coordenadas de los puntos.
		puntosx[i] = log((double)(i + 1));
		puntosy[i] = log((double)distanciasHamming[i]);
	}

	// Se calcula la recta de regresión sobre los puntos transformados
	// en 'eh' tendremos el exponente de Hamming, que es la pendiente de
	// la recta de regresión Y = eh * X + y0.
	// 'r' es el coeficiente de correlación que no se usa aquí (y0 tampoco se usa).
	bool resultado = regresion(puntosx, puntosy, pasos, eh, y0, r);

	// Liberamos memoria
	delete[] puntosx;
	delete[] puntosy;

	return resultado;
}

/*
 * Nombre: liberarACE
 *
 * Descripción: Liberar la memoria asignada a 'ACE'. Se requiere saber cuantos 'pasos' de evolución contenía dicho
 *              ACE para poder hacer la liberación correctamente ya que hay que liberar cada fila (así se asignó la memoria)
 *
 * ACE: Autómata Celular Elemental cuya memoria hay que liberar.
 * pasos: Número de pasos de que consta la evolución del ACE.
 *
 */
void liberarACE(int** ACE, int pasos)
{
	// Liberamos cada fila o paso de evolución
	for (int i = 0; i < pasos + 1; i++)
		delete[] ACE[i];

	// Liberamos el vector de punteros a las filas
	delete[] ACE;
}

/*
 * Nombre: inicializarACE
 *
 * Descripción: Asigna la memoria necesaria para guardar los 'pasos' evolución de un ACE con 'celdas'.
 *				Inicializa el primer paso de la evolución del ACE según el criterio especificado.
 *
 * ACE: Autómata Celular Elemental al que hay que asignar la memoria necesaria e 
 *      inicializar la priemera fila con el criterio deseado ('inicialización' y 'base').
 * pasos: Número de pasos de que constará la evolución del ACE.
 * celdas: Número de celdas que tendrá el ACE.
 * inicializacion: Tipo de inicialización de la primera fila (paso 0).
 * base: Si la inicialización es INICIALIZACION_SIMILAR, se inicializa la primera fila con este vector
 *       modificando únicamente el valor central negándolo (0 <-> 1). Se supone que el vector tiene la misma dimensión
 *       que el ACE ('celdas' + 2). Si la inicialización es INICIALIZACION_SIMILAR, se inicializa la primera fila con este vector.
 *
 * El ACE tendrá 'pasos' + 1 filas (la primera es el paso 0 o inicialización) y 'celdas' + 2 columnas (las dos extras son 
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
	// Inicializamos la primera fila con una distribución aleatoria de 0 y 1
	else if (inicializacion == INICIALIZACION_ALEATORIA) {
		for (int c = 1; c < celdas + 1; c++)
			(*ACE)[0][c] = aleatorio(0, 1);

		// actualizamos las condiciones periódicas de contorno de la primera fila
		(*ACE)[0][0] = (*ACE)[0][celdas];
		(*ACE)[0][celdas + 1] = (*ACE)[0][1];
	}
	// Inicializamos la primera fila con una copia del vector 'base' cambiando el valor
	// de la celda central. Se supone que base tiene la dimensión adecuada y las condiciones
	// periodicas de contorno correctas.
	else if (inicializacion == INICIALIZACION_SIMILAR) {
		for (int c = 0; c < celdas + 2; c++)
			(*ACE)[0][c] = base[c];

		// Se invierte el valor de la celda central de la primera fila
		(*ACE)[0][celdas / 2 + 1] = ((*ACE)[0][celdas / 2 + 1] == 1) ? 0 : 1;
	}
	// Inicializamos la primera fila con una copia del vector 'base'.
	// Se supone que base tiene la dimensión adecuada y las condiciones periodicas de contorno correctas.
	else if (inicializacion == INICIALIZACION_FIJA) {
		for (int c = 0; c < celdas + 2; c++)
			(*ACE)[0][c] = base[c];
	}
}

/*
 * Nombre: liberarAtractores
 *
 * Descripción: Libera la memoria asignada a la estructura que almacena las veces que se visita cada estado en cada paso.
 *              Libera la memoria asignada a la estructura que almacena el porcentaje de estados diferentes visitados en cada paso.
 *              Libera la memoria asignada a la estructura que almacena el porcentaje de visitas a cada estado posible.
 *
 * probabilidades: Para cada paso, guarda cuantas veces se visitó cada estado posible.
 * visitados: Para cada paso, guarda el porcentaje de estados diferentes visitados (multiplicado por 100).
 * estados: Para cada estado, guarda el porcentaje de visitas del mismo respecto al total (multiplicado por 100).
 * pasos: Número de pasos de que constan las simulaciones.
 *
 */
void liberarAtractores(int** probabilidades, int* visitados, int* estados, int pasos)
{
	// Liberamos cada paso que consta de un vector con las veces en que se visitó cada estado
	for (int i = 0; i < pasos + 1; i++)
		delete[] probabilidades[i];

	// Liberamos el vector de punteros
	delete[] probabilidades;

	// Liberamos el vector de porcentaje de estados visitados para cada paso
	delete[] visitados;

	// Liberamos el vector de porcentaje de visitas a cada estado 
	delete[] estados;
}

/*
 * Nombre: inicializarAtractores
 *
 * Descripción: Asigna la memoria necesaria para guardar el número de visitas a cada estado en cada paso ('probabilidades')
 *				a partir de las simulaciones partiendo de una serie de estados iniciales que abarca todo el espacio de fases.
 *				También asigna memoria para guardar el porcentaje de estados diferentes visitados en cada paso a partir de la misma simulación,
 *				es decir, en cada paso contará el número de estados diferentes visitados, dividirá por el número de estados posibles.
 *
 * probabilidades: Para cada paso, guardará cuantas veces se visitó cada estado posible.
 * visitados: Para cada paso, guardaremos la relación entre el número de estados diferentes visitados y el número de estados posibles [0..10000]
 *            ya que guardaremos un entero representando un % con dos decimales de precisión (multiplicado por 100)
 * estados: Para cada estado guardaremos la probabilidad de ser visitado en cualquier paso durante cualquier simulación [0..10000]
 *          ya que guardaremos un entero representando un % con dos decimales de precisión (multiplicado por 100)
 * pasos: Número de pasos de que constará las evoluciones de los ACEs.
 * estadosPosibles: Número de estados posibles de los ACEs (2^celdas).
 *
 */
void inicializarAtractores(int*** probabilidades, int** visitados, int** estados, int pasos, int estadosPosibles)
{
	*probabilidades = new int* [pasos + 1];
	for (int p = 0; p < pasos + 1; p++)
	{
		(*probabilidades)[p] = new int [estadosPosibles];
		memset((*probabilidades)[p], 0, estadosPosibles * sizeof(int));
	}

	// Memoria para la relación entre estados visitados y estados posibles en cada paso
	*visitados = new int [pasos + 1];
	memset(*visitados, 0, (pasos + 1) * sizeof(int));

	// Memoria para la probabilidad de cada estado de ser visitado en algún paso y para alguna simulación
	*estados = new int [estadosPosibles];
	memset(*estados, 0, estadosPosibles * sizeof(int));
}

/*
 * Nombre: generarACE
 *
 * Descripción: Genera la evolución del autómata celular elemental ACE con un número de celdas
 *				'celdas' durante un número de pasos 'pasos' aplicándo la regla 'regla'.
 *
 * ACE: Autómata Celular Elemental sobre el que se calculará la evolución. Se supone que la
 *      primera fila del autómata ya está inicializada con su estado inicial.
 * regla: Entero con la regla que se aplicará para hacer evolucionar el ACE de entrada.
 * pasos: Número de pasos de que consta la evolución del ACE.
 * celdas: Número de celdas que tiene el ACE.
 *
 * Devuelve en la variable ACE la evolución del autómata a partir de su estado inicial
 * aplicando la regla indicada. Se supone que la variable ACE está incializada correctamente, es decir,
 * que las dimensiones son correctas y su estado incial (primera fila) también.
 * Devueve un puntero al vector de estados que ha visitado a cada paso, sólo para tamaños de ACE menores que 32 celdas y
 * sin contar el estado inicial
 *
 */
long* generarACE(int** ACE, int regla, int pasos, int celdas)
{
	int vecindad;	// Guardamos la vecindad de la celda a calcular [0-7]
	long* estados;	// Vamos calculando los estados en los que quedan cada uno de los pasos

	estados = new long [pasos];
	for (int i = 1; i < pasos + 1; i++)
	{
		estados[i - 1] = 0; // Inicializamos el valor del estado para el paso 'i'

		for (int j = 1; j < celdas + 1; j++)
		{
			// La vencidad de la celda (i, j) la componen {(i-1, j-1), (i-1, j), (i-1, j+1)}
			vecindad = (ACE[i - 1][j + 1] | ACE[i - 1][j] << 1 | ACE[i - 1][j - 1] << 2);

			// Comprovamos que valor [0-1] corresponde a dicha vencidad según la regla
			ACE[i][j] = (regla >> vecindad) & 1;

			// Actualizamos el valor del estado si el número de celdas del ACE es menor que 32
			if (celdas < 32)
				estados[i - 1] += (ACE[i][j] * (long)pow(2.0, celdas - j));
		}
/*
		// Actualizamos las probabilidades de caer en el 'estado' en el paso 'i'
		if (probabilidades)
			probabilidades[i][estado]++;

		// Actualizamos el número de estados diferentes visitados en el paso 'i'
		if (visitados)
			if (probabilidades[i][estado] == 1)
				visitados[i]++;

		// Actualizamos el número de visitas a dicho estado
		if (estados)
			estados[estado]++;
*/
		// actualizamos las condiciones periódicas de contorno
		ACE[i][0] = ACE[i][celdas];
		ACE[i][celdas + 1] = ACE[i][1];
	}

	return estados;
}

/*
 * Nombre: generarHamming
 *
 * Descripción: Genera información sobre la evolución de la distancia de Hamming entre el
 *				ACE que se proporciona como parámetro y otro que evoluciona de un estado inicial
 *				en el que únicamente difiere el valor de la posición central.
 *
 * ACE: Simulación del Autómata Celular Elemental sobre el que se calculará la distancia de Hamming
 *		creando uno muy similiar que partirá desde el estado inicial de este cambiando únicamente
 *      la celda central del estado inicial del mismo (primera fila).
 * regla: Entero con la regla que se aplicó al ACE de entrada y hay que aplicar para 
 *		  hacer evolucionar el nuevo ACE en el tiempo.
 * pasos: Número de pasos de que consta el ACE de entrada y que tendrá la simulación del nuevo ACE.
 * celdas: Número de celdas que tiene el ACE y con que constará el nuevo ACE.
 *
 * Devuelve una lista de enteros con la evolución de las distancias de Hamming entre el ACE proporcionado 
 * y el ACE que evoluciona desde un estado inicial casi idéntico.
 *
 */
int* generarHamming(int** ACE, int regla, int pasos, int celdas)
{
	int** ACE1;		// Nueva simulación
	int* hamming;	// Distancias de Hamming de cada estado (fila) entre las evoluciones de los ACE
	
	// Inicializamos el ACE1, esto es, asignamos memoria y inicializamos la primera
	// fila (estado inicial) con el estado inicial de ACE sustituyendo la celda central
	// por la inversa de tal manera que dicho estado inicial sólo difiere en un valor, el del centro.
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
		// Calculamos la distancia de Hamming para el paso 'i' (número de diferencias)
		for (int j = 1; j < celdas + 1; j++)
		{
			hamming[i] += (ACE[i][j] == ACE1[i][j] ? 0 : 1);
		}
	}

	// Liberamos el espacio asignado dinámicamente a ACE1
	liberarACE(ACE1, pasos);

	return hamming;
}

/*
 * Nombre: parsearReglas
 *
 * Descripción: Obtiene las reglas proporcionadas en 'reglastxt'
 *				estas reglas son uno o varios números entre 0 y 255
 *
 * reglas: Vector de enteros en la que guardaremos las reglas obtenidas.
 * nreglas: Entero que contiene el número de reglas finalmente obtenidas.
 * reglastxt: Texto en el que buscamos las reglas. Puede contener uno o más números separados por comas.
 *
 * Devuelve en el vector 'reglas' los enteros entre 0 y 255 encontrados en 'reglastxt' y en 'nreglas'
 * tenemos el número de dichas reglas encontradas. Si no se encuentra ninguna regla se devuelve la regla 'REGLA'.
 * Como máximo se aceptan 256 reglas en la lista (todas las posibles con reglas de 8 bits).
 *
 */
bool parsearReglas(int* reglas, int &nreglas, const char* reglastxt)
{
	int nr;					// Almacenamos cada nueva regla encontrada
	char nreglatxt[32];		// Almacenamos cada cadena separada por comas encontrada

	// Mientras se encuentren comas (',') se va analizando la cadena 'reglastxt'
	nreglas = 0;
	while (strstr(reglastxt, ",") != NULL && nreglas < 255) {
		// En 'nreglatxt' almacenamos el primer texto antes de la primera coma.
		strncpy(nreglatxt, reglastxt, strstr(reglastxt, ",") - reglastxt);
		nreglatxt[strstr(reglastxt, ",") - reglastxt] = 0;

		// Se analiza intenta convertir la cadena a un entero 
		nr = (strlen(nreglatxt) > 0) ? atoi(nreglatxt) : -1;

		// Si no es una regla válida se descarta, si es válida se añade a la lista
		if (nr >= 0 || nr <= 255 || errno == 0) {
			reglas[nreglas] = nr;
			nreglas++;
		}
		else
			printf("Parámetro incorrecto, se esperaba una regla entre 0 y 255... Se descarta\n");

		// Avanzamos a la siguiente regla, tras la siguiente coma.
		reglastxt = strstr(reglastxt, ",") + 1;
	}

	// Se analiza la última cadena sin comas
	nr = (strlen(reglastxt) > 0) ? atoi(reglastxt) : -1;

	// Si no es una regla válida se descarta, si es válida se añade a la lista
	if (nr >= 0 || nr <= 255 || errno == 0) {
		reglas[nreglas] = nr;
		nreglas++;
	}
	else
		printf("Parámetro incorrecto, se esperaba una regla entre 0 y 255... Se descarta\n");

	// Si finalmente no hemos encontrado ninguna regla, pondemos la regla por defecto 'REGLA'
	if (nreglas == 0) {
		reglas[nreglas] = REGLA;
		nreglas++;
	}

	return true;
}

/*
 * Nombre: generarEstadoInicial
 *
 * Descripción: Genera un vector con el estado inicial 'estado' de un ACE 
 *				con el número de celdas 'celdas'
 *
 * estado: Representa el estado inicial del ACE (hay que pasarlo a binario).
 * celdas: Número de celdas del ACE.
 *
 */
int* generarEstadoInicial(int estado, int celdas)
{
	int* base = new int [celdas + 2];

	// Inciializamos cada posición con su 'bit' correspondiente en 'estado'
	for (int j = celdas - 1; j >= 0; j--)
		base[celdas - j] = (estado >> j) & 1;

	// Actualizamos las condiciones periódicas de contorno
	base[0] = base[celdas];
	base[celdas + 1] = base[1];

	return base;
}

double entropia(int* probabilidades, int celdas)
{
	double estadosPosibles = pow(2.0, celdas);
	double suma = 0.0;
	double pe;
	long estados = (long)estadosPosibles;
	for (long e = 0; e < estados; e++) {
		if (probabilidades[e] == 0)
			continue;
		pe = (double)probabilidades[e] / estadosPosibles;
		suma += ((pe * log(pe)) / log(2.0));
	}
	return -suma / (double)celdas;
}

/*
 * Nombre: ACE (Autómata Celular Elemental)
 * Autor: Ismael Flores Campoy
 * Descripción: Genera información a propósito de la evolución de autómatas celulares elementales
 * Sintaxis: ACE <opcion1>:<valor1> <opcion2>:<valor2> ...
 *
 * Opción					| Valores (separados por comas)		| Valor por defecto
 * -------------------------------------------------------------------------------------------------------
 * inicializacion			| aleatoria, semilla				| semilla
 * guardar					| si								| no
 * hamming					| si								| no
 * atractor					| si								| no
 * irreversibilidad			| si								| no
 * regla					| [0, 255], todas					| REGLA (54)
 * pasos					| [1, 5000]							| PASOS (500)
 * celdas					| [2, 10000]						| CELDAS (1000)
 *
 * Argumento					| Significado
 * ------------------------------------------------------------------------------------------------------------
 * inicializacion:aleatoria		| La primera fila del ACE contiene una sucesión aleatoria de '0' y '1'
 * inicializacion:semilla		| La primera fila del ACE contiene todo '0' menos un '1' en la posición central
 * guardar:si					| Se guarda el ACE (o ACEs) en un fichero
 * hamming:si					| Se calcula la evolución de la distancia de hamming en el tiempo
 *								| entre el ACE calculado y otro que difiere únicamente en el valor central 
 *								| de la primera fila. Cada evolución calculada se guarda en un fichero.
 * atractor:si					| Se calculan los atractores, probabilidades de llegar a cada estado en cada paso,
 *								| estados visitados en cada paso, visitas a un estado en cualquier paso desde cualquier estado inicial y
 *								| la evolución de la entropía en cada paso
 * irreversibilidad:si			| Se calcula la evolución del porcentaje de estados no visitados final en función del número de celdas y
 *								| la evolución de la entropía final en función del número de celdas
 * regla:todas					| Se calculan los ACEs (y se guardan en ficheros) de todas las reglas [0, 255]
 * regla:4						| Se calcula el ACE (y se guarda en ficheros) de la regla 4
 * regla:4,90,126				| Se calcula el ACE (y se guarda en ficheros) de laS reglas 4, 90 y 126
 * pasos:300					| Se calculan 300 pasos de la evolución del ACE
 * celdas:700					| El ACE lo conforman 700 posiciones
 * 
 * Ejemplos:
 *
 * ACE regla:126,90
 * ACE hamming:si inicializacion:aleatoria
 * ACE regla:todas guardar:si
 * ACE regla:4 hamming:si pasos:200 celdas:200
 *
 */
int main(int argc, char** argv)
{
	int celdas = CELDAS;							// Celdas del ACE (por defecto CELDAS)
	int pasos = PASOS;								// Pasos de evolución a simular (por defecto PASOS)
	int inicializacion = INICIALIZACION_SEMILLA;	// Por defecto la inicialización es por semilla ACE[0][CELDAS /2 + 1]=1
	int** ACE;										// Donde guardamos el estado del autómata [PASOS + 1][CELDAS + 2]
	char strInicializacion[32];						// Guardamos el tipo de inicialización para generar el nombre del fichero
	bool guardar = false;							// Guardamos si hay que guardar el ACE en un fichero o no
	bool hamming = false;							// Guardamos si hay que calcular la evolución de la distancia de hamming
	bool atractor = false;							// Guardamos si hay que calcular los atractores o no
	bool irreversibilidad = false;					// Guardamos si hay que calcular evoluciones según el número de celdas o no
	int reglas[256];								// Guardamos las reglas a aplicar
	int nreglas;									// Guardamos el número de reglas a aplicar

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
		else if (strstr(argv[a], "hamming:") == argv[a]) {
			// Si encontramos un argumento 'hamming:' analizamos que valor tiene.
			if (strstr(argv[a], ":si") != NULL)
				hamming = true;
		}
		else if (strstr(argv[a], "atractor:") == argv[a]) {
			// Si encontramos un argumento 'atractor:' analizamos que valor tiene.
			if (strstr(argv[a], ":si") != NULL)
				atractor = true;
		}
		else if (strstr(argv[a], "irreversibilidad:") == argv[a]) {
			// Si encontramos un argumento 'irreversibilidad:' analizamos que valor tiene.
			if (strstr(argv[a], ":si") != NULL)
				irreversibilidad = true;
		}
		else if (strstr(argv[a], "guardar:") == argv[a]) {
			// Si encontramos un argumento 'guardar:' analizamos que valor tiene.
			if (strstr(argv[a], ":si") != NULL)
				guardar = true;
		}
		else if (strstr(argv[a], "regla:") == argv[a]) {
			// Si encontramos un argumento 'regla:' analizamos que valor tiene.
			if (strstr(argv[a], ":todas") != NULL) {
				nreglas = 256;
				for (int i = 0; i < 256; i++)
					reglas[i] = i;
			}
			else
				parsearReglas(reglas, nreglas, argv[a] + strlen("regla:"));
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

	char nombreFichero[256];
	for (int nr = 0; nr < nreglas; nr++) { 
		// definimos la condición inicial de nuestro ACE y asignamos la memoria necesaria dinámicamente
		inicializarACE(&ACE, pasos, celdas, inicializacion);

		generarACE(ACE, reglas[nr], pasos, celdas);

		if (guardar) {
			sprintf(nombreFichero, "ACE_R%03d_C%05d_P%05d_%s.pgm", reglas[nr], celdas, pasos, strInicializacion);
			guardaPGMiACE(nombreFichero, pasos + 1, celdas + 2, ACE, 1, 0);
		}

		// Calculamos la distancia de Hamming y el exponente de Hamming
		if (hamming) {
			int* distanciasHamming = generarHamming(ACE, reglas[nr], pasos, celdas);

			sprintf(nombreFichero, "HAMMING_R%03d_C%05d_P%05d.dat", reglas[nr], celdas, pasos);
			guardaPLOT(nombreFichero, distanciasHamming, pasos + 1);

			double eh;
			bool esPosible = exponenteHamming(distanciasHamming, pasos + 1, eh);

			sprintf(nombreFichero, "EHAMMING_R%03d_C%05d_P%05d.dat", reglas[nr], celdas, pasos);
			guardarExponenteHamming(nombreFichero, eh, esPosible);

			delete[] distanciasHamming;
		}

		liberarACE(ACE, pasos);

		// Calculamos el atractor
		if (atractor) {
			int estadosPosibles = (int)pow(2.0, celdas);

			int** probabilidades;
			int* visitadosPaso;
			int* estadoVisitado;
			inicializarAtractores(&probabilidades, &visitadosPaso, &estadoVisitado, pasos, estadosPosibles);

			for (int estado = 0; estado < estadosPosibles; estado++) {
				int* base = generarEstadoInicial(estado, celdas);

				inicializarACE(&ACE, pasos, celdas, INICIALIZACION_FIJA, base);
				delete[] base;

				probabilidades[0][estado]++;
				visitadosPaso[0]++;
				estadoVisitado[estado]++;
				long* estados = generarACE(ACE, reglas[nr], pasos, celdas);

				// probabilidades: Actualizamos las veces que cada estado es visitado en cada paso
				// visitadosPaso: Actualizamos el número de estados diferentes visitados en cada paso
				// estadoVisitado: Actualizamos el número de veces que un estado ha sido visitado
				//
				// Si 'probabilidades' tiene un puntero válido se supone que tiene las dimensiones correctas: (pasos + 1)*(2^celdas)
				// Si 'visitados' tiene un puntero válido se supone que tiene las dimensiones correctas (pasos + 1)
				for (int p = 0; p < pasos; p++) {
					// Actualizamos las probabilidades de caer en el 'estado' en el paso 'p'
					probabilidades[p + 1][estados[p]]++;

					// Actualizamos el número de estados diferentes visitados en el paso 'p'
					if (probabilidades[p + 1][estados[p]] == 1)
						visitadosPaso[p + 1]++;

					// Actualizamos el número de visitas a dicho estado
					estadoVisitado[estados[p]]++;
				}
				delete[] estados;

				liberarACE(ACE, pasos);
			}

			sprintf(nombreFichero, "ATRACTOR_R%03d_C%05d_P%05d.dat", reglas[nr], celdas, pasos);
			guardarAtractorPLOT(nombreFichero, probabilidades, pasos + 1, estadosPosibles);

//			for (int p = 0; p < pasos + 1; p++)
//				visitados[p] = (visitados[p] * 100* 100) / estadosPosibles;
			sprintf(nombreFichero, "ATRACTOR_VISITADO_PASO_R%03d_C%05d_P%05d.dat", reglas[nr], celdas, pasos);
			guardaPLOT(nombreFichero, visitadosPaso, pasos + 1);

//			for (int e = 0; e < estadosPosibles; e++)
//				estados[e] = (estados[e] * 100 * 100000) / ((pasos + 1) * estadosPosibles);
			sprintf(nombreFichero, "ATRACTOR_ESTADO_VISITADO_R%03d_C%05d_P%05d.dat", reglas[nr], celdas, pasos);
			guardaPLOT(nombreFichero, estadoVisitado, estadosPosibles);

			// Evolución de la entropia en el tiempo
			int* entropias = new int [pasos + 1];
			for (int p = 0; p < pasos + 1; p++)
				entropias[p] = (int)(entropia(probabilidades[p], celdas) * 1000.0);
			sprintf(nombreFichero, "ENTROPIA_R%03d_C%05d_P%05d.dat", reglas[nr], celdas, pasos);
			guardaPLOT(nombreFichero, entropias, pasos + 1);
			delete[] entropias;

			liberarAtractores(probabilidades, visitadosPaso, estadoVisitado, pasos);
		}

		// Irreversibilidad (evolución para número de celdas)
		if (irreversibilidad) {
			int* noVisitados = new int [N_MAX - N_MIN + 1];
			int* entropias = new int [N_MAX - N_MIN + 1];
			for (int N = N_MIN; N <= N_MAX; N++) {
				int estadosPosibles = (int)pow(2.0, N);

				int visitadosPaso = 0;
				int* probabilidades;
				probabilidades = new int [estadosPosibles];
				memset(probabilidades, 0, estadosPosibles * sizeof(int));

				for (int estado = 0; estado < estadosPosibles; estado++) {
					int* base = generarEstadoInicial(estado, N);
					inicializarACE(&ACE, pasos, N, INICIALIZACION_FIJA, base);
					delete[] base;

					long* estados = generarACE(ACE, reglas[nr], pasos, N);

					// probabilidades: Actualizamos las veces que cada estado es visitado en el último paso
					// visitadosPaso: Actualizamos el número de estados diferentes visitados en el último paso
					// estadoVisitado: Actualizamos el número de veces que un estado ha sido visitado en el último paso
					// Actualizamos las probabilidades de caer en el 'estado' en el paso 'p'
					probabilidades[estados[pasos - 1]]++;

					// Actualizamos el número de estados diferentes visitados en el paso 'p'
					if (probabilidades[estados[pasos - 1]] == 1)
						visitadosPaso++;

					delete[] estados;
					liberarACE(ACE, pasos);
				}

				noVisitados[N - N_MIN] = (int)((1000.0 * (double)(estadosPosibles - visitadosPaso)) / (double)estadosPosibles);
				entropias[N - N_MIN] = (int)(entropia(probabilidades, N) * 1000.0);

				delete[] probabilidades;
			}

			sprintf(nombreFichero, "NOVISITADOS_R%03d.dat", reglas[nr]);
			guardaPLOT(nombreFichero, noVisitados,  N_MAX - N_MIN + 1, N_MIN);

			sprintf(nombreFichero, "ENTROPIA_R%03d.dat", reglas[nr]);
			guardaPLOT(nombreFichero, entropias, N_MAX - N_MIN + 1, N_MIN);

			delete[] noVisitados;
			delete[] entropias;
		}
	}
}
