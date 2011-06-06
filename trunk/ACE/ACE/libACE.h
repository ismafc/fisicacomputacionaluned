#ifndef _LIBACE_H_
#define _LIBACE_H_

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>

#define INICIALIZACION_SEMILLA		0		// Se inicializa con un '1' en la primera fila, en la columna central
#define INICIALIZACION_ALEATORIA	1		// Se inicializa con una distribución aleatoria de '0' y '1' en la primera fila
#define INICIALIZACION_SIMILAR		2		// Se inicializa con la primera fila similar a otra pero cambiado sólo el valor central negado
#define INICIALIZACION_FIJA			3		// Se inicializa con la primera fila proporcionada

/*
 * Nombre: aleatorio
 *
 * Descripción: Devuelve un entero aleatorio entre 'a' y 'b' (ambos incluidos y de forma equiprobable)
 *
 * a: Valor inicial
 * b: Valor final
 *
 * La función presupone que (a < b) y (RAND_MAX > 0)
 *
 */
int aleatorio(int a, int b);

/*
 * Nombre: asignarMemoriaACE
 *
 * Descripción: Asigna la memoria necesaria para guardar los 'pasos' evolución de un ACE con 'celdas'.
 *
 * ACE: Autómata Celular Elemental al que hay que asignar la memoria necesaria e 
 *      inicializar la priemera fila con el criterio deseado ('inicialización' y 'base').
 * pasos: Número de pasos de que constará la evolución del ACE.
 * celdas: Número de celdas que tendrá el ACE.
 *
 * El ACE tendrá 'pasos' + 1 filas (la primera es el paso 0 o inicialización) y 'celdas' + 2 columnas (las dos extras son 
 * para establecer las condiciones de contorno).
 *
 */
void asignarMemoriaACE(int*** ACE, int pasos, int celdas);

/*
 * Nombre: liberarMemoriaACE
 *
 * Descripción: Liberar la memoria asignada a 'ACE'. Se requiere saber cuantos 'pasos' de evolución contenía dicho
 *              ACE para poder hacer la liberación correctamente ya que hay que liberar cada fila (así se asignó la memoria)
 *
 * ACE: Autómata Celular Elemental cuya memoria hay que liberar.
 * pasos: Número de pasos de que consta la evolución del ACE.
 *
 */
void liberarMemoriaACE(int** ACE, int pasos);

/*
 * Nombre: inicializarACE
 *
 * Descripción: Inicializa el primer paso de la evolución del ACE según el criterio especificado.
 *
 * ACE: Autómata Celular Elemental del que hay que inicializar la primera fila 
 *      con el criterio deseado ('inicialización' y 'base').
 * celdas: Número de celdas que tendrá el ACE.
 * inicializacion: Tipo de inicialización de la primera fila (paso 0).
 * base: Si la inicialización es INICIALIZACION_SIMILAR, se inicializa la primera fila con este vector
 *       modificando únicamente el valor central negándolo (0 <-> 1). Se supone que el vector tiene la misma dimensión
 *       que el ACE ('celdas' + 2). Si la inicialización es INICIALIZACION_SIMILAR, se inicializa la primera fila con este vector.
 *
 * El ACE debe tener 'celdas' + 2 columnas (las dos extras son para establecer las condiciones de contorno) y 
 * almenos una fila (paso 0).
 *
 */
void inicializarACE(int** ACE, int celdas, int inicializacion = INICIALIZACION_SEMILLA, const int* base = NULL);

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
long* generarACE(int** ACE, int regla, int pasos, int celdas);

/*
 * Nombre: obtenerValores
 *
 * Descripción: Obtiene los valores proporcionados en 'reglastxt'
 *				estos valores son uno o varios números enteros separados por comas.
 *
 * valores: Vector de enteros en la que guardaremos los valores obtenidos.
 * maxvalores: Entero que contiene el número máximo de valores que pueden almacenar 'valores'.
 * valorestxt: Texto en el que buscamos los valores. Puede contener uno o más números enteros separados por comas.
 *
 * Devuelve en el vector 'valores' los enteros entre 'minvalor' y 'maxvalor' encontrados en 'valorestxt' y 
 * devuelve el número de dichos valores encontrados. Si no se encuentra ningún valor se devuelve 0.
 * Como máximo se aceptan 'maxvalores' valores.
 *
 */
int obtenerValores(int* valores, int maxvalores, const char* valorestxt, int minvalor = 0, int maxvalor = 255);

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
bool regresion(const double* puntosx, const double* puntosy, int npuntos, double& my, double& y0, double& r);

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
bool exponenteHamming(const int* distanciasHamming, int pasos, double& eh);

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
int* generarHamming(int** ACE, int regla, int pasos, int celdas);

/*
 * Nombre: inicializarAtractores
 *
 * Descripción: Asigna la memoria necesaria para guardar el número de visitas a cada estado en cada paso ('probabilidades')
 *				a partir de las simulaciones partiendo de una serie de estados iniciales que abarca todo el espacio de fases.
 *				También asigna memoria para guardar el porcentaje de estados diferentes visitados en cada paso a partir de la misma simulación,
 *				es decir, en cada paso contará el número de estados diferentes visitados, dividirá por el número de estados posibles.
 *
 * probabilidades: Para cada paso, guardará cuantas veces se visitó cada estado posible en dicho paso.
 * visitados: Para cada paso, guardaremos el número de estados diferentes visitados.
 * estados: Para cada estado guardaremos las veces que ha sido visitado en cualquier paso de la simulación.
 * pasos: Número de pasos de que constará las evoluciones de los ACEs.
 * estadosPosibles: Número de estados posibles de los ACEs (2^celdas).
 *
 */
void inicializarAtractores(int*** probabilidades, int** visitados, int** estados, int pasos, int estadosPosibles);

/*
 * Nombre: liberarAtractores
 *
 * Descripción: Libera la memoria asignada a la estructura que almacena las veces que se visita cada estado en cada paso.
 *              Libera la memoria asignada a la estructura que almacena el porcentaje de estados diferentes visitados en cada paso.
 *              Libera la memoria asignada a la estructura que almacena el porcentaje de visitas a cada estado posible.
 *
 * probabilidades: Para cada paso, guarda cuantas veces se visitó cada estado posible.
 * visitados: Para cada paso, guarda el porcentaje de estados diferentes visitados.
 * estados: Para cada estado, guarda el porcentaje de visitas del mismo respecto al total.
 * pasos: Número de pasos de que constan las simulaciones.
 *
 */
void liberarAtractores(int** probabilidades, int* visitados, int* estados, int pasos);

/*
 * Nombre: generarEstadoInicial
 *
 * Descripción: Genera un vector con el estado inicial 'estado' de un ACE 
 *				con el número de celdas 'celdas'. El vector se devuelve en 'base'
 *              que debe tener la dimensión adecuada.
 *
 * base: Vector en el que ponemos el estado inicial (ceros y unos)
 * estado: Representa el estado inicial del ACE (hay que pasarlo a binario).
 * celdas: Número de celdas del ACE.
 *
 */
void generarEstadoInicial(int* base, int estado, int celdas);

/*
 * Nombre: entropia
 *
 * Descripción: Calcula la entropia (medida del desorden) de un paso concreto de un ACE. 
 *              Para ello recibimos el número de visitas de cada estado en dicho paso y 
 *				el número de celdas del ACE.
 *
 * probabilidades: Vector con tantas posiciones como estados posibles (2^celdas). En cada posición
 *                 tenemos el número de visitas al estado dado en el paso concreto de la evolución del ACE.
 * celdas: Número de celdas del ACE.
 *
 * Devuelve la entropía de dicho paso de evolución del ACE. 
 * Es una valor de 0 (mínima entropía) a 1 (máxima entropía o desorden absoluto).
 *
 */
double entropia(int* probabilidades, int celdas);

#endif