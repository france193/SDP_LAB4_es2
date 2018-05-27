/**
 * Name:    Francesco
 * Surname: Longo
 * ID:      223428
 * Lab:     4
 * Ex:      2
 *
 * Implement a sequential program in C that takes a single argument k from the command line. The
 * program creates two vectors (v1 and v2) of dimension k, and a matrix (mat) of dimension kxk,
 * which are filled with random numbers in the range [-0.5 0.5], then it performs the product
 * v1 T * mat * v2, and print the result. This is an example for k=5:
 *
 * Perform the product operation in two steps: v = mat * v2, which produces a new vector v, and
 * result = v1 T * v
 * Then, write a concurrent program using threads that performs the same task. The main thread
 * creates the vectors, the matrix, and k threads. Then, it waits the termination of the other threads.
 * Each thread i performs the product of the i-th row vector of mat and v2, which produces
 * the i-th element of vector v.
 * One of the created threads, the last one terminating its product operation, performs the final
 * operation result= v1 T * v, and prints the result.
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <string.h>
#include <stdarg.h>

#define NUMBER_DECIMALS 4

// prototypes
void print_array(float *array, int size);
float getRandomNumberFloat(float min, float max);
void print_matrix(float ** matrix, int n, int m);
void *thread_code(void *arg);

// typedef
typedef struct Counter{
    int count;
    pthread_mutex_t mutex;
} Counter;

// global variables
int k;
float result = 0, *v, *v1, *v2, **mat;

pthread_t *array_threads;
Counter *counter;
sem_t *barrier;

// main
int main(int argc, char **argv) {
    int *tmp = NULL;

    srand((unsigned int) time(NULL)); // seed for random generator

    printf(" > Program (NON CONCURRENT) started!\n");

    if (argc != 2) {
        printf("Expected 1 argument: %s <k>\n", argv[0]);
        exit(-1);
    }

    k = atoi(argv[1]);

    if (k == 0) {
        printf("You must insert a number greater than 0\n");
        return -2; // user inserted wrong argument
    }

    printf(" > k is: %d!\n", k);

    // allocating arrays and matrix
    v = (float *)malloc(k * sizeof(float));
    if (v == NULL) {
        printf("Error allocating array v\n");
        exit(-3);
    }

    v1 = (float *)malloc(k * sizeof(float));
    if (v1 == NULL) {
        printf("Error allocating array v1\n");
        exit(-4);
    }

    v2 = (float *)malloc(k * sizeof(float));
    if (v2 == NULL) {
        printf("Error allocating array v2\n");
        exit(-5);
    }

    mat = (float **)malloc(k * sizeof(float));
    if (mat == NULL) {
        printf("Error allocating array mat\n");
        exit(-6);
    }
    for (int i=0; i<k; i++) {
        mat[i] = (float *)malloc(k * sizeof(float));
        if (mat[i] == NULL) {
            printf("Error allocating array mat[i]\n");
            exit(7);
        }
    }

    for (int i = 0; i < k; i++) {
        v1[i] = getRandomNumberFloat((float) -0.5, 0.5);
        v2[i] = getRandomNumberFloat((float) -0.5, 0.5);
        mat[i] = malloc(k * sizeof(float));

        for (int j = 0; j < k; j++) {
            mat[i][j] = getRandomNumberFloat((float) -0.5, 0.5);
        }
    }

    printf("\nv1:\n");
    print_array(v1, k);

    printf("\nv2:\n");
    print_array(v2, k);

    printf("\nmat:\n");
    print_matrix(mat, k, k);

    // matrix * v2
    for (int i=0; i<k; i++){
        for (int j=0; j<k; j++){
            v[i] += mat[i][j] * v2[j];
        }
    }

    printf("\nmat * v2 \n");
    print_array(v, k);

    // (v1 * (matrix * v2)
    for (int i=0; i<k; i++){
        result += v1[i] * v[i];
    }

    printf("\n > Result is: %f!\n", result);

    printf("\n > Program (NON CONCURRENT) END!\n");

    /********************************************/

    printf(" > Program (CONCURRENT) started!\n");

    array_threads = malloc(k*sizeof(pthread_t));

    // reallocate arrays and mat
    for (int i = 0; i < k; i++) {
        v1[i] = getRandomNumberFloat((float) -0.5, 0.5);
        v2[i] = getRandomNumberFloat((float) -0.5, 0.5);
        mat[i] = malloc(k * sizeof(float));

        for (int j = 0; j < k; j++) {
            mat[i][j] = getRandomNumberFloat((float) -0.5, 0.5);
        }
    }

    printf("\nv1:\n");
    print_array(v1, k);

    printf("\nv2:\n");
    print_array(v2, k);

    printf("\nmat:\n");
    print_matrix(mat, k, k);

    counter = (Counter *) malloc (sizeof(Counter));

    // initially there are no threads running
    counter->count = 0;
    pthread_mutex_init (&counter->mutex, NULL);

    barrier = (sem_t *) malloc(sizeof(sem_t));

    // initialized to 0 because last thread wakes other threads
    sem_init (barrier, 0, 0);

    for (int i=0; i<k; i++){
        tmp = malloc(sizeof(int));
        *tmp = i;

        if(pthread_create(&array_threads[i], NULL, thread_code, (void *) tmp) != 0) {
            printf("There was an error with the thread creation\n");
            exit(1);
        }
    }

    // wait for the termination of the other threads
    for (int i = 0; i < k; i++) {
        pthread_join(array_threads[i], NULL);
    }

    // free arrays and mat
    free(v);
    free(v1);
    free(v2);
    free(tmp);

    for (int i=0; i<k; i++) {
        free(mat[i]);
    }

    free(mat);
    free(counter);

    return 0;
}


void print_array(float *array, int size) {
    int i;

    for(i=0; i<size; i++) {
        printf("%f\n", array[i]);
    }
}

void print_matrix(float ** matrix, int n, int m) {
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            printf("%.*f\t", NUMBER_DECIMALS, matrix[i][j]);
        }
        printf("\n");
    }
}

float getRandomNumberFloat(float min, float max) {
    // random float interval [min, max]
    float random = min + ((float) rand()) / ((RAND_MAX/(max-min)));

    char * truncated;
    truncated = malloc(8 * sizeof(char));
    sprintf(truncated, "%.*f", NUMBER_DECIMALS, random);

    return (float) atof(truncated);
}

void *thread_code(void *arg){
    int i, j;
    i = *(int *) arg;

    // matrix(row) * v2
    for (j=0; j<k; j++) {
        // no need to protect, each thread writes to a different position
        v[i] += mat[i][j] * v2[j];
    }

    pthread_mutex_lock(&counter->mutex);
    counter->count++;

    if (counter->count == k) { // last thread
        // wake threads waiting at barrier
        for (j=0; j<k; j++) {
            sem_post(barrier);
        }

        // matrix * v2
        printf("\nmat * v2 \n");
        print_array(v, k);

        // v1 * (matrix * v2)
        for (j=0; j<k; j++) {
            result += v1[j]*v[j];
        }

        printf("\nResult is : %f!", result);
    }

    pthread_mutex_unlock(&counter->mutex);

    // wait at the barrier
    sem_wait(barrier);

    pthread_exit(0);
}
