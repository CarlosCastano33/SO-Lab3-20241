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
 * @author     Danny Munera, Carlos Castaño, Neiver Tapia, Sebastian Mora
 * @date       2024
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>

typedef struct{
	int limite_inferior, limite_superior;
	double* vector_promedios;
}Argumentos;

unsigned int seed;
int p;
int n_threads;
int max_iters;

double* X;
double a;
double* Y;
double* Y_avgs;	

//SAXPY iterative SAXPY mfunction
void* saxpy (void* arg){					
	Argumentos *args = (Argumentos *)arg;
	int it;
	int sup = args->limite_superior;
	double* v_p = args->vector_promedios;

	for(it = 0; it < max_iters; it++){
		int j = args->limite_inferior;
		for(j; j < sup; j++){
			Y[j] = Y[j] + a * X[j];
			v_p[it] += Y[j];
		}
		v_p[it] = v_p[it] / (sup-(args->limite_inferior));
	}
}


int main(int argc, char* argv[]){
	// Variables to obtain command line parameters
	seed = 1;
  	p = 10000000;
  	n_threads = 2;
  	max_iters = 1000;
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
	

	int i;
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
	double* Y_avgs_copy;
	Y_avgs_copy = malloc(sizeof(double) * max_iters); 
	memcpy(Y_avgs_copy, Y_avgs, sizeof(double) * max_iters);		
	
	Argumentos args_1, args_2;
	args_1.limite_inferior = 0;
	args_1.limite_superior = (p/2);
	args_1.vector_promedios = Y_avgs;

	args_2.limite_inferior = (p/2);
	args_2.limite_superior = p;
	args_2.vector_promedios = Y_avgs_copy;
	
	pthread_t thread_1, thread_2;
	
	gettimeofday(&t_start, NULL);
	
	pthread_create(&thread_1, NULL, saxpy, &args_1);
	pthread_create(&thread_2, NULL, saxpy, &args_2);
	
	pthread_join(thread_1, NULL);
	pthread_join(thread_2, NULL);
	
	*Y_avgs = (*Y_avgs + *Y_avgs_copy) / 2;

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
	printf("RES: final vector Y_avgs= [ ");
	
	return 0;
}