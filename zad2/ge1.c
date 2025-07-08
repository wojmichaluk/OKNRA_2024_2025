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
void ge(double ** A, int SIZE) {
  int i, j, k;

  for (k = 0; k < SIZE; k++) { 
    for (i = k + 1; i < SIZE; i++) { 
      for (j = k + 1; j < SIZE; j++) { 
        A[i][j] -= A[k][j] * (A[i][k] / A[k][k]);
      } 
    }
  }
}

// replicable random matrix initialization
void init_rand_matrix(double ** A, int SIZE) {
  int i, j;
  srand(1);

  for (i = 0; i < SIZE; i++) { 
    for (j = 0; j < SIZE; j++) {
      A[i][j] = rand();
    }
  }
}

// used to check algorithm correctness
double calculate_matrix_sum(const double ** A, int SIZE) {
  int i, j;
  double check = 0.0;

  for (i = 0; i < SIZE; i++) {
    for (j = 0; j < SIZE; j++) {
      check += A[i][j];
    }
  }

  return check;
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
    double **matrix = (double**)malloc(SIZE * sizeof(double*));
    double *matrix_ = (double*)malloc(SIZE * SIZE * sizeof(double));

    for (i = 0; i < SIZE; i++) {
      matrix[i] = matrix_ + i * SIZE;
    }

    init_rand_matrix(matrix, SIZE);

    printf("Calling Gauss elimination algorithm\n");
    dtime = dclock();
    ge(matrix, SIZE);
    dtime = dclock() - dtime;
    printf("Execution time: %lf \n", dtime);

    check = calculate_matrix_sum((const double **)matrix, SIZE);
    printf("Checking correctness: %le \n", check);
    fflush(stdout);

    free(matrix);
    free(matrix_);
  } else if (argc == 2) { // for tests purpose
    FILE* checksums = fopen("ref.txt", "w");
    FILE* times; 
    FILE* flops;

    if (strcmp(argv[1], "time") == 0) {
      times = fopen("times/times1.csv", "w");
    } else if (strcmp(argv[1], "flops") == 0) {
      flops = fopen("flops/flops1.csv", "w");
    }

    for (SIZE = 10; SIZE <= 1200; SIZE += 10) {
      double **matrix = (double**)malloc(SIZE * sizeof(double*));
      double *matrix_ = (double*)malloc(SIZE * SIZE * sizeof(double));

      for (i = 0; i < SIZE; i++) {
        matrix[i] = matrix_ + i * SIZE;
      }

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

      check = calculate_matrix_sum((const double **)matrix, SIZE);
      fprintf(checksums, "%lf\n", check);
      fflush(stdout);

      free(matrix);
      free(matrix_);
    }
  }

  return 0;
}
