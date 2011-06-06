#ifndef _LIBACE_H_
#define _LIBACE_H_

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>

#define INICIALIZACION_SEMILLA		0		// Se inicializa con un '1' en la primera fila, en la columna central
#define INICIALIZACION_ALEATORIA	1		// Se inicializa con una distribuci�n aleatoria de '0' y '1' en la primera fila
#define INICIALIZACION_SIMILAR		2		// Se inicializa con la primera fila similar a otra pero cambiado s�lo el valor central negado
#define INICIALIZACION_FIJA			3		// Se inicializa con la primera fila proporcionada

/*
 * Nombre: aleatorio
 *
 * Descripci�n: Devuelve un entero aleatorio entre 'a' y 'b' (ambos incluidos y de forma equiprobable)
 *
 * a: Valor inicial
 * b: Valor final
 *
 * La funci�n presupone que (a < b) y (RAND_MAX > 0)
 *
 */
int aleatorio(int a, int b);

/*
 * Nombre: asignarMemoriaACE
 *
 * Descripci�n: Asigna la memoria necesaria para guardar los 'pasos' evoluci�n de un ACE con 'celdas'.
 *
 * ACE: Aut�mata Celular Elemental al que hay que asignar la memoria necesaria e 
 *      inicializar la priemera fila con el criterio deseado ('inicializaci�n' y 'base').
 * pasos: N�mero de pasos de que constar� la evoluci�n del ACE.
 * celdas: N�mero de celdas que tendr� el ACE.
 *
 * El ACE tendr� 'pasos' + 1 filas (la primera es el paso 0 o inicializaci�n) y 'celdas' + 2 columnas (las dos extras son 
 * para establecer las condiciones de contorno).
 *
 */
void asignarMemoriaACE(int*** ACE, int pasos, int celdas);

/*
 * Nombre: liberarMemoriaACE
 *
 * Descripci�n: Liberar la memoria asignada a 'ACE'. Se requiere saber cuantos 'pasos' de evoluci�n conten�a dicho
 *              ACE para poder hacer la liberaci�n correctamente ya que hay que liberar cada fila (as� se asign� la memoria)
 *
 * ACE: Aut�mata Celular Elemental cuya memoria hay que liberar.
 * pasos: N�mero de pasos de que consta la evoluci�n del ACE.
 *
 */
void liberarMemoriaACE(int** ACE, int pasos);

/*
 * Nombre: inicializarACE
 *
 * Descripci�n: Inicializa el primer paso de la evoluci�n del ACE seg�n el criterio especificado.
 *
 * ACE: Aut�mata Celular Elemental del que hay que inicializar la primera fila 
 *      con el criterio deseado ('inicializaci�n' y 'base').
 * celdas: N�mero de celdas que tendr� el ACE.
 * inicializacion: Tipo de inicializaci�n de la primera fila (paso 0).
 * base: Si la inicializaci�n es INICIALIZACION_SIMILAR, se inicializa la primera fila con este vector
 *       modificando �nicamente el valor central neg�ndolo (0 <-> 1). Se supone que el vector tiene la misma dimensi�n
 *       que el ACE ('celdas' + 2). Si la inicializaci�n es INICIALIZACION_SIMILAR, se inicializa la primera fila con este vector.
 *
 * El ACE debe tener 'celdas' + 2 columnas (las dos extras son para establecer las condiciones de contorno) y 
 * almenos una fila (paso 0).
 *
 */
void inicializarACE(int** ACE, int celdas, int inicializacion = INICIALIZACION_SEMILLA, const int* base = NULL);

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
 * Devueve un puntero al vector de estados que ha visitado a cada paso, s�lo para tama�os de ACE menores que 32 celdas y
 * sin contar el estado inicial
 *
 */
long* generarACE(int** ACE, int regla, int pasos, int celdas);

/*
 * Nombre: obtenerValores
 *
 * Descripci�n: Obtiene los valores proporcionados en 'reglastxt'
 *				estos valores son uno o varios n�meros enteros separados por comas.
 *
 * valores: Vector de enteros en la que guardaremos los valores obtenidos.
 * maxvalores: Entero que contiene el n�mero m�ximo de valores que pueden almacenar 'valores'.
 * valorestxt: Texto en el que buscamos los valores. Puede contener uno o m�s n�meros enteros separados por comas.
 *
 * Devuelve en el vector 'valores' los enteros entre 'minvalor' y 'maxvalor' encontrados en 'valorestxt' y 
 * devuelve el n�mero de dichos valores encontrados. Si no se encuentra ning�n valor se devuelve 0.
 * Como m�ximo se aceptan 'maxvalores' valores.
 *
 */
int obtenerValores(int* valores, int maxvalores, const char* valorestxt, int minvalor = 0, int maxvalor = 255);

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
bool regresion(const double* puntosx, const double* puntosy, int npuntos, double& my, double& y0, double& r);

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
bool exponenteHamming(const int* distanciasHamming, int pasos, double& eh);

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
int* generarHamming(int** ACE, int regla, int pasos, int celdas);

/*
 * Nombre: inicializarAtractores
 *
 * Descripci�n: Asigna la memoria necesaria para guardar el n�mero de visitas a cada estado en cada paso ('probabilidades')
 *				a partir de las simulaciones partiendo de una serie de estados iniciales que abarca todo el espacio de fases.
 *				Tambi�n asigna memoria para guardar el porcentaje de estados diferentes visitados en cada paso a partir de la misma simulaci�n,
 *				es decir, en cada paso contar� el n�mero de estados diferentes visitados, dividir� por el n�mero de estados posibles.
 *
 * probabilidades: Para cada paso, guardar� cuantas veces se visit� cada estado posible en dicho paso.
 * visitados: Para cada paso, guardaremos el n�mero de estados diferentes visitados.
 * estados: Para cada estado guardaremos las veces que ha sido visitado en cualquier paso de la simulaci�n.
 * pasos: N�mero de pasos de que constar� las evoluciones de los ACEs.
 * estadosPosibles: N�mero de estados posibles de los ACEs (2^celdas).
 *
 */
void inicializarAtractores(int*** probabilidades, int** visitados, int** estados, int pasos, int estadosPosibles);

/*
 * Nombre: liberarAtractores
 *
 * Descripci�n: Libera la memoria asignada a la estructura que almacena las veces que se visita cada estado en cada paso.
 *              Libera la memoria asignada a la estructura que almacena el porcentaje de estados diferentes visitados en cada paso.
 *              Libera la memoria asignada a la estructura que almacena el porcentaje de visitas a cada estado posible.
 *
 * probabilidades: Para cada paso, guarda cuantas veces se visit� cada estado posible.
 * visitados: Para cada paso, guarda el porcentaje de estados diferentes visitados.
 * estados: Para cada estado, guarda el porcentaje de visitas del mismo respecto al total.
 * pasos: N�mero de pasos de que constan las simulaciones.
 *
 */
void liberarAtractores(int** probabilidades, int* visitados, int* estados, int pasos);

/*
 * Nombre: generarEstadoInicial
 *
 * Descripci�n: Genera un vector con el estado inicial 'estado' de un ACE 
 *				con el n�mero de celdas 'celdas'. El vector se devuelve en 'base'
 *              que debe tener la dimensi�n adecuada.
 *
 * base: Vector en el que ponemos el estado inicial (ceros y unos)
 * estado: Representa el estado inicial del ACE (hay que pasarlo a binario).
 * celdas: N�mero de celdas del ACE.
 *
 */
void generarEstadoInicial(int* base, int estado, int celdas);

/*
 * Nombre: entropia
 *
 * Descripci�n: Calcula la entropia (medida del desorden) de un paso concreto de un ACE. 
 *              Para ello recibimos el n�mero de visitas de cada estado en dicho paso y 
 *				el n�mero de celdas del ACE.
 *
 * probabilidades: Vector con tantas posiciones como estados posibles (2^celdas). En cada posici�n
 *                 tenemos el n�mero de visitas al estado dado en el paso concreto de la evoluci�n del ACE.
 * celdas: N�mero de celdas del ACE.
 *
 * Devuelve la entrop�a de dicho paso de evoluci�n del ACE. 
 * Es una valor de 0 (m�nima entrop�a) a 1 (m�xima entrop�a o desorden absoluto).
 *
 */
double entropia(int* probabilidades, int celdas);

#endif