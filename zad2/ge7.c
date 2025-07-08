#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <papi.h>

// PAPI macro helper definition
#define NUM_EVENT 2
#define THRESHOLD 100000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__); exit(retval); }

// loop unrolling macro helpers
#define BLKSIZE 16
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define IDX(i, j, n) (((j) + (i) * (n)))

static double gtod_ref_time_sec = 0.0;

// Adapted from the bl2_clock() routine in the BLIS library
double dclock() {
  double the_time, norm_sec;
  struct timeval tv;

  gettimeofday(&tv, NULL);

  if (gtod_ref_time_sec == 0.0) {
    gtod_ref_time_sec = (double)tv.tv_sec;
  }

  norm_sec = (double)tv.tv_sec - gtod_ref_time_sec;
  the_time = norm_sec + tv.tv_usec * 1.0e-6;

  return the_time;
}

// Algorithm to be optimized
void ge(double* A, int SIZE) {
  register int i, j, k;
  register double multiplier;

  for (k = 0; k < SIZE; k++) { 
    for (i = k + 1; i < SIZE; i++) { 
      multiplier = (A[IDX(i, k, SIZE)] / A[IDX(k, k, SIZE)]);

      for (j = k + 1; j < SIZE; ) { 
        if (j < (MAX(SIZE - BLKSIZE, 0))) {
          A[IDX(i, j, SIZE)] -= A[IDX(k, j, SIZE)] * multiplier;
          A[IDX(i, j+1, SIZE)] -= A[IDX(k, j+1, SIZE)] * multiplier;
          A[IDX(i, j+2, SIZE)] -= A[IDX(k, j+2, SIZE)] * multiplier;
          A[IDX(i, j+3, SIZE)] -= A[IDX(k, j+3, SIZE)] * multiplier;
          A[IDX(i, j+4, SIZE)] -= A[IDX(k, j+4, SIZE)] * multiplier;
          A[IDX(i, j+5, SIZE)] -= A[IDX(k, j+5, SIZE)] * multiplier;
          A[IDX(i, j+6, SIZE)] -= A[IDX(k, j+6, SIZE)] * multiplier;
          A[IDX(i, j+7, SIZE)] -= A[IDX(k, j+7, SIZE)] * multiplier;
          A[IDX(i, j+8, SIZE)] -= A[IDX(k, j+8, SIZE)] * multiplier;
          A[IDX(i, j+9, SIZE)] -= A[IDX(k, j+9, SIZE)] * multiplier;
          A[IDX(i, j+10, SIZE)] -= A[IDX(k, j+10, SIZE)] * multiplier;
          A[IDX(i, j+11, SIZE)] -= A[IDX(k, j+11, SIZE)] * multiplier;
          A[IDX(i, j+12, SIZE)] -= A[IDX(k, j+12, SIZE)] * multiplier;
          A[IDX(i, j+13, SIZE)] -= A[IDX(k, j+13, SIZE)] * multiplier;
          A[IDX(i, j+14, SIZE)] -= A[IDX(k, j+14, SIZE)] * multiplier;
          A[IDX(i, j+15, SIZE)] -= A[IDX(k, j+15, SIZE)] * multiplier;
          j += BLKSIZE;
        } else {
          A[IDX(i, j, SIZE)] -= A[IDX(k, j, SIZE)] * multiplier;
          j++;
        }
      } 
    }
  }
}

// replicable random matrix initialization
void init_rand_matrix(double* A, int SIZE) {
  int i, j;
  srand(1);

  for (i = 0; i < SIZE; i++) { 
    for (j = 0; j < SIZE; j++) {
      A[IDX(i, j, SIZE)] = rand();
    }
  }
}

// used to check algorithm correctness
double calculate_matrix_sum(const double* A, int SIZE) {
  int i, j;
  double check = 0.0;

  for (i = 0; i < SIZE; i++) {
    for (j = 0; j < SIZE; j++) {
      check += A[IDX(i, j, SIZE)];
    }
  }

  return check;
}

// check algorithm correctness
int check_correctness(const double* A, int SIZE) {
  FILE* checksums = fopen("ref.txt", "r");
  double matrix_sum = calculate_matrix_sum(A, SIZE);
  double check;
  int retval, curr_size = 10;

  while (curr_size <= SIZE) {
    retval = fscanf(checksums, "%lf\n", &check);
    curr_size += 10;
  }
  
  return check != matrix_sum ? -1 : 0;
}

int main(int argc, const char* argv[]) {
  int i, SIZE;
  double dtime, check;

  // PAPI FLOPS variables
  float real_time, proc_time, mflops;
  float ireal_time, iproc_time, imflops;
  long long flpops, iflpops;
  int retval;

  if (argc == 1) { // single algorithm execution
    SIZE = 1500;
    double *matrix = (double*)malloc(SIZE * SIZE * sizeof(double));

    init_rand_matrix(matrix, SIZE);

    printf("Calling Gauss elimination algorithm\n");
    dtime = dclock();
    ge(matrix, SIZE);
    dtime = dclock() - dtime;
    printf("Execution time: %lf \n", dtime);

    check = calculate_matrix_sum((const double*)matrix, SIZE);
    printf("Checking correctness: %le \n", check);
    fflush(stdout);

    free(matrix);
  } else if (argc == 2) { // for tests purpose
    FILE* times; 
    FILE* flops;

    if (strcmp(argv[1], "time") == 0) {
      times = fopen("times/times7.csv", "w");
    } else if (strcmp(argv[1], "flops") == 0) {
      flops = fopen("flops/flops7.csv", "w");
    }

    for (SIZE = 10; SIZE <= 1200; SIZE += 10) {
      double *matrix = (double*)malloc(SIZE * SIZE * sizeof(double));

      init_rand_matrix(matrix, SIZE);

      if (strcmp(argv[1], "time") == 0) { // measure time
        dtime = dclock();
      } else if (strcmp(argv[1], "flops") == 0) { // measure flops
        if ((retval = PAPI_flops_rate(PAPI_DP_OPS, &ireal_time, &iproc_time, &iflpops, &imflops)) < PAPI_OK) { 
          printf("retval: %d\n", retval);
          exit(1);
        }
      }
      
      // algorithm execution
      ge(matrix, SIZE);

      if (strcmp(argv[1], "time") == 0) { // measure time
        dtime = dclock() - dtime;
        fprintf(times, "%d,%lf\n", SIZE, dtime);
      } else if (strcmp(argv[1], "flops") == 0) { // measure flops
        if ((retval = PAPI_flops_rate(PAPI_DP_OPS, &real_time, &proc_time, &flpops, &mflops)) < PAPI_OK) {
          printf("retval: %d\n", retval);
          exit(1);
        }

        fprintf(flops, "%d,%lf\n", SIZE, 0.001 * mflops);
      }

      if (check_correctness((const double*)matrix, SIZE) < 0) {
        printf("Algorithm failed! Incorrect result.");
        exit(1);
      }

      fflush(stdout);
      free(matrix);
    }
  }

  return 0;
}
