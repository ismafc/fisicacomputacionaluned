#include "libACE.h"

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

long* generarACE(int** ACE, int regla, int pasos, int celdas)
{
	int vecindad;	// Guardamos la vecindad de la celda a calcular [0-7]
	long* estados;	// Vamos calculando los estados por los que pasa en cada uno de los pasos

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

		// actualizamos las condiciones periódicas de contorno
		ACE[i][0] = ACE[i][celdas];
		ACE[i][celdas + 1] = ACE[i][1];
	}

	return estados;
}

void asignarMemoriaACE(int*** ACE, int pasos, int celdas)
{
	// Asignamos la memoria para los pasos (lista de punteros a cada fila -vector-)
	*ACE = new int* [pasos + 1];

	// Para cada paso, asignamos la memoria para las celdas e inicializamos sus valores a 0
	for (int p = 0; p < pasos + 1; p++)
	{
		(*ACE)[p] = new int [celdas + 2];
		memset((*ACE)[p], 0, (celdas + 2) * sizeof(int));
	}
}

void liberarMemoriaACE(int** ACE, int pasos)
{
	// Liberamos cada fila o paso de evolución
	for (int i = 0; i < pasos + 1; i++)
		delete[] ACE[i];

	// Liberamos el vector de punteros a las filas
	delete[] ACE;
}

void inicializarACE(int** ACE, int celdas, int inicializacion, const int* base)
{
	// Inicializamos la primera fila con un 1 en la celda central
	if (inicializacion == INICIALIZACION_SEMILLA)
		ACE[0][celdas / 2 + 1] = 1;
	// Inicializamos la primera fila con una distribución aleatoria de 0 y 1
	else if (inicializacion == INICIALIZACION_ALEATORIA) {
		for (int c = 1; c < celdas + 1; c++)
			ACE[0][c] = aleatorio(0, 1);

		// actualizamos las condiciones periódicas de contorno de la primera fila
		ACE[0][0] = ACE[0][celdas];
		ACE[0][celdas + 1] = ACE[0][1];
	}
	// Inicializamos la primera fila con una copia del vector 'base' cambiando el valor
	// de la celda central. Se supone que base tiene la dimensión adecuada y las condiciones
	// periodicas de contorno correctas.
	else if (inicializacion == INICIALIZACION_SIMILAR) {
		for (int c = 0; c < celdas + 2; c++)
			ACE[0][c] = base[c];

		// Se invierte el valor de la celda central de la primera fila
		ACE[0][celdas / 2 + 1] = (ACE[0][celdas / 2 + 1] == 1) ? 0 : 1;
	}
	// Inicializamos la primera fila con una copia del vector 'base'.
	// Se supone que base tiene la dimensión adecuada y las condiciones periodicas de contorno correctas.
	else if (inicializacion == INICIALIZACION_FIJA) {
		for (int c = 0; c < celdas + 2; c++)
			ACE[0][c] = base[c];
	}
}
