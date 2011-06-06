#include "libACE.h"

#pragma warning ( disable: 4996 )

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
	// Ponemos todos los valores a 0 como inicialización
	memset(ACE[0], 0, (celdas + 2) * sizeof(int));

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

int obtenerValores(int* valores, int maxvalores, const char* valorestxt, int minvalor, int maxvalor)
{
	int nv;					// Almacenamos cada nuevo valor encontrado
	char nvalortxt[32];		// Almacenamos cada cadena separada por comas encontrada
	int nvalores;			// Guardamos los valores encontrados en la cadena de texto 'reglastxt'

	// Mientras se encuentren comas (',') se va analizando la cadena 'reglastxt'
	nvalores = 0;
	while (strstr(valorestxt, ",") != NULL && nvalores < maxvalores) {
		// En 'nreglatxt' almacenamos el primer texto antes de la primera coma.
		strncpy(nvalortxt, valorestxt, strstr(valorestxt, ",") - valorestxt);
		nvalortxt[strstr(valorestxt, ",") - valorestxt] = 0;

		// Se analiza intenta convertir la cadena a un entero 
		nv = (strlen(nvalortxt) > 0) ? atoi(nvalortxt) : -1;

		// Si no es una regla válida se descarta, si es válida se añade a la lista
		if (nv >= minvalor || nv <= maxvalor || errno == 0) {
			valores[nvalores] = nv;
			nvalores++;
		}
		else
			printf("Parámetro incorrecto, se esperaba un valor entre %d y %d... Se descarta\n", minvalor, maxvalor);

		// Avanzamos a la siguiente regla, tras la siguiente coma.
		valorestxt = strstr(valorestxt, ",") + 1;
	}

	if (nvalores < maxvalores) {
		if (strstr(valorestxt, "-") != NULL) {
			// Se analiza el rango de reglas. Primero el valor inicial
			strncpy(nvalortxt, valorestxt, strstr(valorestxt, "-") - valorestxt);
			nvalortxt[strstr(valorestxt, "-") - valorestxt] = 0;
			int desde = atoi(nvalortxt);

			// Después el valor final
			valorestxt = strstr(valorestxt, "-") + 1;
			int hasta = atoi(valorestxt);
			for (int i = desde; i <= hasta && nvalores < maxvalores; i++) {
				if (i >= minvalor || i <= maxvalor) {
					valores[nvalores] = i;
					nvalores++;
				}
				else
					printf("Parámetro incorrecto, se esperaba un valor entre %d y %d... Se descarta\n", minvalor, maxvalor);
			}
		}
		else {
			// Se analiza la última cadena sin comas
			nv = (strlen(valorestxt) > 0) ? atoi(valorestxt) : -1;

			// Si no es una regla válida se descarta, si es válida se añade a la lista
			if (nv >= minvalor || nv <= maxvalor || errno == 0) {
				valores[nvalores] = nv;
				nvalores++;
			}
			else
				printf("Parámetro incorrecto, se esperaba un valor entre %d y %d... Se descarta\n", minvalor, maxvalor);
		}
	}

	return nvalores;
}

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

	// Calculamos el numerador
	numeradorm = N * sumxy - sumx * sumy;

	// Calculamos el denominador para 'my' (si es cero devolvemos error)
	denominadormy = N * sumx2 - sumx * sumx;
	if (denominadormy == 0.0 && numeradorm != 0.0)
		return false;

	// Calculamos el denominador para 'mx' (si es cero devolvemos error)
	denominadormx = N * sumy2 - sumy * sumy;
	if (denominadormx == 0.0 && numeradorm != 0.0)
		return false;

	// Calculamos 'mx', 'my' y 'y0'
	my = (numeradorm == 0.0) ? 0.0 : numeradorm / denominadormy;
	mx = (numeradorm == 0.0) ? 0.0 : numeradorm / denominadormx;
	y0 = (sumy - my * sumx) / N;

	// Si no podemos calcular 'r' devolvemos error
	if (mx * my < 0.0)
		return false;

	// Calculamos 'r'
	r = sqrt(mx * my);

	return true;
}

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

int* generarHamming(int** ACE, int regla, int pasos, int celdas)
{
	int** ACE1;		// Nueva simulación
	int* hamming;	// Distancias de Hamming de cada estado (fila) entre las evoluciones de los ACE
	
	// Inicializamos el ACE1, esto es, asignamos memoria y inicializamos la primera
	// fila (estado inicial) con el estado inicial de ACE sustituyendo la celda central
	// por la inversa de tal manera que dicho estado inicial sólo difiere en un valor, el del centro.
	asignarMemoriaACE(&ACE1, pasos, celdas);
	inicializarACE(ACE1, celdas, INICIALIZACION_SIMILAR, ACE[0]);

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
	liberarMemoriaACE(ACE1, pasos);

	return hamming;
}

void inicializarAtractores(int*** probabilidades, int** visitados, int** estados, int pasos, int estadosPosibles)
{
	// Memoria para almacenar las visitas a cada estado en cada paso
	*probabilidades = new int* [pasos + 1];
	for (int p = 0; p < pasos + 1; p++)
	{
		(*probabilidades)[p] = new int [estadosPosibles];
		memset((*probabilidades)[p], 0, estadosPosibles * sizeof(int));
	}

	// Memoria para almacenar el número de estados diferentes visitados en cada paso
	*visitados = new int [pasos + 1];
	memset(*visitados, 0, (pasos + 1) * sizeof(int));

	// Memoria para almacenar el número de visitas a cada estado en algún paso
	*estados = new int [estadosPosibles];
	memset(*estados, 0, estadosPosibles * sizeof(int));
}

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

void generarEstadoInicial(int* base, int estado, int celdas)
{
	// Incializamos cada posición con su 'bit' correspondiente en 'estado'
	for (int j = celdas - 1; j >= 0; j--)
		base[celdas - j] = (estado >> j) & 1;

	// Actualizamos las condiciones periódicas de contorno
	base[0] = base[celdas];
	base[celdas + 1] = base[1];
}

double entropia(int* probabilidades, int celdas)
{
	long estadosPosibles = (long)pow(2.0, celdas);	// Todos los estados posibles
	double suma = 0.0;								// Iremos guardando la suma
	double pe;										// Guardaremos la probabilidad de visitar un estado concreto

	// La fórmula de la entropía consiste en realizar el sumatorio de
	for (long e = 0; e < estadosPosibles; e++) {
		if (probabilidades[e] == 0)
			continue;
		pe = (double)probabilidades[e] / (double)estadosPosibles;
		// Sumamos la probabilidad de visitar el estado 'e' en el paso que nos ocupa multiplicada
		// por el logaritmo en base 2 de dicha probabilidad.
		suma += ((pe * log(pe)) / log(2.0));
	}

	// Finalmente devolvemos el sumatorio calculado multiplicado por -1/celdas
	return -suma / (double)celdas;
}
