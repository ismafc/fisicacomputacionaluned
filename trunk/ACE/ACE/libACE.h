#ifndef _LIBACE_H_
#define _LIBACE_H_

#include <memory.h>
#include <stdlib.h>
#include <math.h>

#define INICIALIZACION_SEMILLA		0		// Se inicializa con un '1' en la primera fila, en la columna central
#define INICIALIZACION_ALEATORIA	1		// Se inicializa con una distribuci�n aleatoria de '0' y '1' en la primera fila
#define INICIALIZACION_SIMILAR		2		// Se inicializa con la primera fila similar a otra pero cambiado s�lo el valor central negado
#define INICIALIZACION_FIJA			2		// Se inicializa con la primera fila proporcionada

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

#endif