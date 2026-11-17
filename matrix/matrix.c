#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define CAPACITY 20     // storage size of the matrices (20 x 20)
#define NUM_THREADS 10  // total number of worker threads per operation

int MAX = CAPACITY;     // working size, may be set from the command line

int matA[CAPACITY][CAPACITY];
int matB[CAPACITY][CAPACITY];

int matSumResult[CAPACITY][CAPACITY];
int matDiffResult[CAPACITY][CAPACITY];
int matProductResult[CAPACITY][CAPACITY];

void fillMatrix(int matrix[CAPACITY][CAPACITY]) {
    for(int i = 0; i<MAX; i++) {
        for(int j = 0; j<MAX; j++) {
            matrix[i][j] = rand()%10+1;
        }
    }
}

void printMatrix(int matrix[CAPACITY][CAPACITY]) {
    for(int i = 0; i<MAX; i++) {
        for(int j = 0; j<MAX; j++) {
            printf("%5d", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Work out which rows thread number `id` is responsible for.
// Rows are split as evenly as possible among NUM_THREADS threads
// (for a 20 x 20 matrix each thread gets 2 rows).
static void getRowRange(int id, int *start, int *end) {
    int base = MAX / NUM_THREADS;
    int extra = MAX % NUM_THREADS;
    *start = id * base + (id < extra ? id : extra);
    *end = *start + base + (id < extra ? 1 : 0);
}

// Fetches the appropriate coordinates from the argument, and sets
// the cell of matSumResult at the coordinates to the sum of the
// values at the coordinates of matA and matB.
void* computeSum(void* args) { // pass in the number of the ith thread
    int id = *(int*)args;
    free(args);

    int start, end;
    getRowRange(id, &start, &end);
    for (int i = start; i < end; i++) {
        for (int j = 0; j < MAX; j++) {
            matSumResult[i][j] = matA[i][j] + matB[i][j];
        }
    }
    return NULL;
}

// Fetches the appropriate coordinates from the argument, and sets
// the cell of matDiffResult at the coordinates to the difference of the
// values at the coordinates of matA and matB.
void* computeDiff(void* args) { // pass in the number of the ith thread
    int id = *(int*)args;
    free(args);

    int start, end;
    getRowRange(id, &start, &end);
    for (int i = start; i < end; i++) {
        for (int j = 0; j < MAX; j++) {
            matDiffResult[i][j] = matA[i][j] - matB[i][j];
        }
    }
    return NULL;
}

// Fetches the appropriate coordinates from the argument, and sets
// the cell of matProductResult at the coordinates to the inner product
// of matA and matB.
void* computeProduct(void* args) { // pass in the number of the ith thread
    int id = *(int*)args;
    free(args);

    int start, end;
    getRowRange(id, &start, &end);
    for (int i = start; i < end; i++) {
        for (int j = 0; j < MAX; j++) {
            int sum = 0;
            for (int k = 0; k < MAX; k++) {
                sum += matA[i][k] * matB[k][j];
            }
            matProductResult[i][j] = sum;
        }
    }
    return NULL;
}

// Spawn threads to fill the result matrices.
// How many threads will you spawn? 10 per operation, each handling
// a block of rows (30 threads total for the three operations).
int main(int argc, char *argv[]) {
    srand(time(0));  // Do Not Remove. Just ignore and continue below.

    // 0. Get the matrix size from the command line and assign it to MAX
    if (argc > 1) {
        MAX = atoi(argv[1]);
    }
    if (MAX <= 0 || MAX > CAPACITY) {
        MAX = CAPACITY;
    }

    // 1. Fill the matrices (matA and matB) with random values.
    fillMatrix(matA);
    fillMatrix(matB);

    // 2. Print the initial matrices.
    printf("Matrix A:\n");
    printMatrix(matA);
    printf("Matrix B:\n");
    printMatrix(matB);

    // 3. Create pthread_t objects for our threads.
    pthread_t sumThreads[NUM_THREADS];
    pthread_t diffThreads[NUM_THREADS];
    pthread_t productThreads[NUM_THREADS];

    // 4. Create the threads. Each one gets its own malloc'd thread number,
    // which it uses to work out its block of rows and then frees.
    for (int i = 0; i < NUM_THREADS; i++) {
        int *idSum = malloc(sizeof(int));
        int *idDiff = malloc(sizeof(int));
        int *idProd = malloc(sizeof(int));
        if (!idSum || !idDiff || !idProd) {
            fprintf(stderr, "malloc failed\n");
            exit(1);
        }
        *idSum = i;
        *idDiff = i;
        *idProd = i;

        pthread_create(&sumThreads[i], NULL, computeSum, idSum);
        pthread_create(&diffThreads[i], NULL, computeDiff, idDiff);
        pthread_create(&productThreads[i], NULL, computeProduct, idProd);
    }

    // 5. Wait for all threads to finish.
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(sumThreads[i], NULL);
        pthread_join(diffThreads[i], NULL);
        pthread_join(productThreads[i], NULL);
    }

    // 6. Print the results.
    printf("Results:\n");
    printf("Sum:\n");
    printMatrix(matSumResult);
    printf("Difference:\n");
    printMatrix(matDiffResult);
    printf("Product:\n");
    printMatrix(matProductResult);
    return 0;
}