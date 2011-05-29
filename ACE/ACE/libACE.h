#ifndef _LIBACE_H_
#define _LIBACE_H_

#include <memory.h>
#include <stdlib.h>
#include <math.h>

#define INICIALIZACION_SEMILLA		0		// Se inicializa con un '1' en la primera fila, en la columna central
#define INICIALIZACION_ALEATORIA	1		// Se inicializa con una distribución aleatoria de '0' y '1' en la primera fila
#define INICIALIZACION_SIMILAR		2		// Se inicializa con la primera fila similar a otra pero cambiado sólo el valor central negado
#define INICIALIZACION_FIJA			2		// Se inicializa con la primera fila proporcionada

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

#endif