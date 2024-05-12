/**
 * @defgroup   SAXPY saxpy
 *
 * @brief      This file implements an iterative saxpy operation
 * 
 * @param[in] <-p> {vector size} 
 * @param[in] <-s> {seed}
 * @param[in] <-n> {number of threads to create} 
 * @param[in] <-i> {maximum itertions} 
 *
 @author     Danny Munera, Carlos Castaño, Neiver Tapia, Sebastian Mora
 * @date       2024
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <pthread.h>

typedef struct{
	int id;
	int max_iters;
	int p;
	int n_threads;
	double* X; 
	double* Y; 
	double* Y_avgs;
	double a;
	double* partial_sums; // Nuevo campo para partial_sums
	}Argumentos;

pthread_barrier_t barrier;
int it = 0;

void* saxpy (void* arg){				
	Argumentos *args = (Argumentos *)arg;
	int id = args->id;
    int max_iters = args->max_iters;
    int p = args->p;
    int n_threads = args->n_threads;
    double a = args->a;
    double* X = args->X;
    double* Y = args->Y;
    double* Y_avgs = args->Y_avgs;
    int i;
    double* partial_sums = args->partial_sums; // Acceder a partial_sums

     // Calculando la porción del trabajo que le corresponde a este hilo
    int chunk_size = p / n_threads;
    int start = id * chunk_size;
    int end = (id == n_threads - 1) ? p : (id + 1) * chunk_size;

    while(it < max_iters) {
    	double partial_sum = 0.0; // Suma parcial local
        for (i = start; i < end; i++) {
            Y[i] = Y[i] + a * X[i];
            partial_sum += Y[i];
        }
        partial_sums[id] = partial_sum;
        
        // Barrera para sincronizar it
        pthread_barrier_wait(&barrier);
        if (id == 0) {
        	for (int i = 0; i < n_threads; i++) {
                Y_avgs[it] += partial_sums[i];
            }
            Y_avgs[it] /= p;
            it++;
        }
        // Barrera para sincronizar Y_avgs
        pthread_barrier_wait(&barrier);
    }

	pthread_exit(NULL);
}

int main(int argc, char* argv[]){
	// Variables to obtain command line parameters
	unsigned int seed = 1;
  	int p = 10000000;
  	int n_threads = 2;
  	int max_iters = 1000;
  	// Variables to perform SAXPY operation
	double* X;
	double a;
	double* Y;
	double* Y_avgs;
	int i;
	double* partial_sums; // Declarar ----------->
	// Variables to get execution time
	struct timeval t_start, t_end;
	double exec_time;

	// Getting input values
	int opt;
	while((opt = getopt(argc, argv, ":p:s:n:i:")) != -1){  
		switch(opt){  
			case 'p':  
			printf("vector size: %s\n", optarg);
			p = strtol(optarg, NULL, 10);
			assert(p > 0 && p <= 2147483647);
			break;  
			case 's':  
			printf("seed: %s\n", optarg);
			seed = strtol(optarg, NULL, 10);
			break;
			case 'n':  
			printf("threads number: %s\n", optarg);
			n_threads = strtol(optarg, NULL, 10);
			break;  
			case 'i':  
			printf("max. iterations: %s\n", optarg);
			max_iters = strtol(optarg, NULL, 10);
			break;  
			case ':':  
			printf("option -%c needs a value\n", optopt);  
			break;  
			case '?':  
			fprintf(stderr, "Usage: %s [-p <vector size>] [-s <seed>] [-n <threads number>] [-i <maximum itertions>]\n", argv[0]);
			exit(EXIT_FAILURE);
		}  
	}  
	srand(seed);

	printf("p = %d, seed = %d, n_threads = %d, max_iters = %d\n", \
	 p, seed, n_threads, max_iters);	

	// initializing data
	X = (double*) malloc(sizeof(double) * p);
	Y = (double*) malloc(sizeof(double) * p);
	Y_avgs = (double*) malloc(sizeof(double) * max_iters);
	

	for(i = 0; i < p; i++){
		X[i] = (double)rand() / RAND_MAX;
		Y[i] = (double)rand() / RAND_MAX;
	}
	for(i = 0; i < max_iters; i++){
		Y_avgs[i] = 0.0;
	}
	a = (double)rand() / RAND_MAX;

#ifdef DEBUG
	printf("vector X= [ ");
	for(i = 0; i < p-1; i++){
		printf("%f, ",X[i]);
	}
	printf("%f ]\n",X[p-1]);

	printf("vector Y= [ ");
	for(i = 0; i < p-1; i++){
		printf("%f, ", Y[i]);
	}
	printf("%f ]\n", Y[p-1]);

	printf("a= %f \n", a);	
#endif

	/*
	 *	Function to parallelize 
	 */

	if(n_threads > p){
		printf("\n Numero de hilos mayor al tamaño del vector. | hilos = %d | p = %d\n",n_threads, p);
        return 1;
	}
	partial_sums = (double*) malloc(sizeof(double) * n_threads); // ------>

	gettimeofday(&t_start, NULL);

	pthread_barrier_init(&barrier, NULL, n_threads);
	pthread_t threads[n_threads];
    Argumentos args[n_threads];
    for (i = 0; i < n_threads; i++) {
        args[i].id = i;
        args[i].max_iters = max_iters;
        args[i].p = p;
        args[i].a = a;
        args[i].X = X;
        args[i].Y = Y;
        args[i].Y_avgs = Y_avgs;
        args[i].n_threads = n_threads;
        args[i].partial_sums = partial_sums;
        pthread_create(&threads[i], NULL, saxpy, (void*)&args[i]);
    }

     //Esperando a que los threads terminen
    for (i = 0; i < n_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    // Terminar barrera
    pthread_barrier_destroy(&barrier);

	gettimeofday(&t_end, NULL);

#ifdef DEBUG
	printf("RES: final vector Y= [ ");
	for(i = 0; i < p-1; i++){
		printf("%f, ", Y[i]);
	}
	printf("%f ]\n", Y[p-1]);
#endif
	
	// Computing execution time
	exec_time = (t_end.tv_sec - t_start.tv_sec) * 1000.0;  // sec to ms
	exec_time += (t_end.tv_usec - t_start.tv_usec) / 1000.0; // us to ms
	printf("Execution time: %f ms \n", exec_time);
	printf("Last 3 values of Y: %f, %f, %f \n", Y[p-3], Y[p-2], Y[p-1]);
	printf("Last 3 values of Y_avgs: %f, %f, %f \n", Y_avgs[max_iters-3], Y_avgs[max_iters-2], Y_avgs[max_iters-1]);
	return 0;
}	
