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
  register int i, j, k;
  register double multiplier;

  for (k = 0; k < SIZE; k++) { 
    for (i = k + 1; i < SIZE; i++) { 
      multiplier = (A[i][k] / A[k][k]);

      for (j = k + 1; j < SIZE; ) { 
        if (j < (MAX(SIZE - BLKSIZE, 0))) {
          A[i][j] -= A[k][j] * multiplier;
          A[i][j+1] -= A[k][j+1] * multiplier;
          A[i][j+2] -= A[k][j+2] * multiplier;
          A[i][j+3] -= A[k][j+3] * multiplier;
          A[i][j+4] -= A[k][j+4] * multiplier;
          A[i][j+5] -= A[k][j+5] * multiplier;
          A[i][j+6] -= A[k][j+6] * multiplier;
          A[i][j+7] -= A[k][j+7] * multiplier;
          A[i][j+8] -= A[k][j+8] * multiplier;
          A[i][j+9] -= A[k][j+9] * multiplier;
          A[i][j+10] -= A[k][j+10] * multiplier;
          A[i][j+11] -= A[k][j+11] * multiplier;
          A[i][j+12] -= A[k][j+12] * multiplier;
          A[i][j+13] -= A[k][j+13] * multiplier;
          A[i][j+14] -= A[k][j+14] * multiplier;
          A[i][j+15] -= A[k][j+15] * multiplier;
          j += BLKSIZE;
        } else {
          A[i][j] -= A[k][j] * multiplier;
          j++;
        }
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

// check algorithm correctness
int check_correctness(const double ** A, int SIZE) {
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
    FILE* times; 
    FILE* flops;

    if (strcmp(argv[1], "time") == 0) {
      times = fopen("times/times5.csv", "w");
    } else if (strcmp(argv[1], "flops") == 0) {
      flops = fopen("flops/flops5.csv", "w");
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

      if (check_correctness((const double **)matrix, SIZE) < 0) {
        printf("Algorithm failed! Incorrect result.");
        exit(1);
      }

      fflush(stdout);
      free(matrix);
      free(matrix_);
    }
  }

  return 0;
}
