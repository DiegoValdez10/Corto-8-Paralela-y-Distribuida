
#include <stdio.h>      
#include <stdlib.h>     
#include <time.h>       
#include <omp.h>        

#define HEIGHT 10        // se define la altura de la matriz (número de filas)
#define WIDTH 10        // se define el ancho de la matriz (número de columnas)

// Función para llenar la matriz con valores aleatorios de brillo
void llenar_matriz(int foto[HEIGHT][WIDTH]) {
    srand(time(NULL)); // Inicializa la semilla para números aleatorios usando la hora actual
    for (int i = 0; i < HEIGHT; i++) // Recorre cada fila
        for (int j = 0; j < WIDTH; j++) // Recorre cada columna
            foto[i][j] = rand() % 1000; // Asigna un valor aleatorio entre 0 y 999 a cada píxel
}

// imprime la matriz de brillo en pantalla
void imprimir_matriz(int foto[HEIGHT][WIDTH]) {
    for (int i = 0; i < HEIGHT; i++) { // Recorre cada fila
        for (int j = 0; j < WIDTH; j++) // Recorre cada columna
            printf("%4d ", foto[i][j]); // Imprime el valor del píxel con formato
        printf("\n"); // Salto de línea
    }
}

// Parte Serial: busca el valor máximo y sus coordenadas recorriendo toda la matriz
double max_brillo_serial(int foto[HEIGHT][WIDTH]) {
    clock_t start = clock(); // Inicia el cronómetro
    int max_val = foto[0][0]; // Inicializa el máximo con el primer valor de la matriz
    int max_i = 0, max_j = 0; // Inicializa las coordenadas del máximo
    for (int i = 0; i < HEIGHT; i++) { // Recorre cada fila
        for (int j = 0; j < WIDTH; j++) { // Recorre cada columna
            if (foto[i][j] > max_val) { // Si el valor actual es mayor que el máximo
                max_val = foto[i][j];   // Actualiza el valor máximo
                max_i = i;              // Guarda la fila del nuevo máximo
                max_j = j;              // Guarda la columna del nuevo máximo
            }
        }
    }
    clock_t end = clock(); 
    double tiempo = (double)(end - start) / CLOCKS_PER_SEC; // Calcula el tiempo en segundos
    printf("Serial:\n");
    printf("Maximo valor: %d\n", max_val); // Imprime el valor máximo encontrado
    printf("Coordenadas: (%d, %d)\n", max_i, max_j); // Imprime las coordenadas del máximo
    printf("Tiempo serial: %f segundos\n", tiempo);
    return tiempo;
}

// Parte Paralela: Busca el valor máximo usando varios hilos (OpenMP)
double max_brillo_paralelo(int foto[HEIGHT][WIDTH], int *num_threads) {
    double start = omp_get_wtime(); // Inicia el cronómetro de OpenMP
    int max_val = foto[0][0]; // Inicializa el máximo global
    int max_i = 0, max_j = 0; // Inicializa las coordenadas globales

    #pragma omp parallel
    {
        int local_max = foto[0][0]; // Cada hilo tiene su propio máximo local
        int local_i = 0, local_j = 0; // Y sus propias coordenadas locales

        #pragma omp for collapse(2) // Divide el trabajo entre los hilos
        for (int i = 0; i < HEIGHT; i++) { // Recorre cada fila
            for (int j = 0; j < WIDTH; j++) { // Recorre cada columna
                if (foto[i][j] > local_max) { // Si el valor actual es mayor que el máximo local
                    local_max = foto[i][j];   // Actualiza el máximo local
                    local_i = i;              // Guarda la fila del nuevo máximo local
                    local_j = j;              // Guarda la columna del nuevo máximo local
                }
            }
        }

        #pragma omp critical // Sección crítica para actualizar el máximo 
        {
            if (local_max > max_val) { // Si el máximo local es mayor que el máximo global
                max_val = local_max;   // Actualiza el máximo global
                max_i = local_i;       // Actualiza la fila del máximo global
                max_j = local_j;       // Actualiza la columna del máximo global
            }
        }

        #pragma omp master
        {
            *num_threads = omp_get_num_threads(); // Obtiene el número de hilos usados
        }
    }

    double end = omp_get_wtime(); // Detiene el cronómetro
    double tiempo = end - start; // Calcula el tiempo en segundos
    printf("Paralelo:\n");
    printf("Maximo valor: %d\n", max_val);
    printf("Coordenadas: (%d, %d)\n", max_i, max_j);
    printf("Tiempo paralelo: %f segundos\n", tiempo);
    return tiempo;
}

// Función principal del programa
int main() {
    int foto[HEIGHT][WIDTH]; // Declara la matriz de brillo
    llenar_matriz(foto);     // Llena la matriz con valores aleatorios
    printf("Matriz de brillo:\n");
    imprimir_matriz(foto);   // Imprime la matriz

    double t_serial = max_brillo_serial(foto);   // Llama a la función serial y mide el tiempo
    int num_threads = 1;
    double t_paralelo = max_brillo_paralelo(foto, &num_threads); // Llama a la función paralela y mide el tiempo

    // Calcular speedup y eficiencia
    double speedup = t_serial / t_paralelo; // Calcula el speedup
    double eficiencia = speedup / num_threads;
    printf("\n--- METRICAS ---\n");
    printf("Speedup: %f\n", speedup);
    printf("Eficiencia: %f\n", eficiencia);
    printf("Numero de hilos: %d\n", num_threads);
    printf("Tamaño de la matriz: %dx%d\n", HEIGHT, WIDTH);

    return 0;
}