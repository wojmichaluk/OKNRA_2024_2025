#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <papi.h>
#include <x86intrin.h>

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
  double multiplier[2];
  register __m128d mm_multiplier;
  register __m128d tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10, tmp11, tmp12, tmp13, tmp14, tmp15;

  for (k = 0; k < SIZE; k++) { 
    for (i = k + 1; i < SIZE; i++) { 
      multiplier[0] = (A[IDX(i, k, SIZE)] / A[IDX(k, k, SIZE)]);
      multiplier[1] = multiplier[0];
      mm_multiplier = _mm_loadu_pd(multiplier);

      for (j = k + 1; j < SIZE; ) { 
        if (j < (MAX(SIZE - BLKSIZE, 0))) {
          // load
          tmp0 = _mm_loadu_pd(A + IDX(i, j, SIZE));
          tmp1 = _mm_loadu_pd(A + IDX(k, j, SIZE));
          tmp2 = _mm_loadu_pd(A + IDX(i, j+2, SIZE));
          tmp3 = _mm_loadu_pd(A + IDX(k, j+2, SIZE));
          tmp4 = _mm_loadu_pd(A + IDX(i, j+4, SIZE));
          tmp5 = _mm_loadu_pd(A + IDX(k, j+4, SIZE));
          tmp6 = _mm_loadu_pd(A + IDX(i, j+6, SIZE));
          tmp7 = _mm_loadu_pd(A + IDX(k, j+6, SIZE));
          tmp8 = _mm_loadu_pd(A + IDX(i, j+8, SIZE));
          tmp9 = _mm_loadu_pd(A + IDX(k, j+8, SIZE));
          tmp10 = _mm_loadu_pd(A + IDX(i, j+10, SIZE));
          tmp11 = _mm_loadu_pd(A + IDX(k, j+10, SIZE));
          tmp12 = _mm_loadu_pd(A + IDX(i, j+12, SIZE));
          tmp13 = _mm_loadu_pd(A + IDX(k, j+12, SIZE));
          tmp14 = _mm_loadu_pd(A + IDX(i, j+14, SIZE));
          tmp15 = _mm_loadu_pd(A + IDX(k, j+14, SIZE));

          // multiply
          tmp1 = _mm_mul_pd(tmp1, mm_multiplier);
          tmp3 = _mm_mul_pd(tmp3, mm_multiplier);
          tmp5 = _mm_mul_pd(tmp5, mm_multiplier);
          tmp7 = _mm_mul_pd(tmp7, mm_multiplier);
          tmp9 = _mm_mul_pd(tmp9, mm_multiplier);
          tmp11 = _mm_mul_pd(tmp11, mm_multiplier);
          tmp13 = _mm_mul_pd(tmp13, mm_multiplier);
          tmp15 = _mm_mul_pd(tmp15, mm_multiplier);

          // subtraction
          tmp0 = _mm_sub_pd(tmp0, tmp1);
          tmp2 = _mm_sub_pd(tmp2, tmp3);
          tmp4 = _mm_sub_pd(tmp4, tmp5);
          tmp6 = _mm_sub_pd(tmp6, tmp7);
          tmp8 = _mm_sub_pd(tmp8, tmp9);
          tmp10 = _mm_sub_pd(tmp10, tmp11);
          tmp12 = _mm_sub_pd(tmp12, tmp13);
          tmp14 = _mm_sub_pd(tmp14, tmp15);

          // store
          _mm_storeu_pd(A + IDX(i, j, SIZE), tmp0);
          _mm_storeu_pd(A + IDX(i, j+2, SIZE), tmp2);
          _mm_storeu_pd(A + IDX(i, j+4, SIZE), tmp4);
          _mm_storeu_pd(A + IDX(i, j+6, SIZE), tmp6);
          _mm_storeu_pd(A + IDX(i, j+8, SIZE), tmp8);
          _mm_storeu_pd(A + IDX(i, j+10, SIZE), tmp10);
          _mm_storeu_pd(A + IDX(i, j+12, SIZE), tmp12);
          _mm_storeu_pd(A + IDX(i, j+14, SIZE), tmp14);

          j += BLKSIZE;
        } else {
          A[IDX(i, j, SIZE)] -= A[IDX(k, j, SIZE)] * multiplier[0];
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
  int i, SIZE, retval;
  double dtime, check;

  // PAPI FLOPS variables
  float real_time, proc_time, mflops;
  float ireal_time, iproc_time, imflops;
  long long flpops, iflpops;

  // PAPI counters variables
  int EventSet;
  int event_codes[NUM_EVENT] = { PAPI_DP_OPS, PAPI_VEC_DP }; 
  char errstring[PAPI_MAX_STR_LEN];
  long long values[NUM_EVENT];

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
    FILE *times, *flops, *vecops;

    if (strcmp(argv[1], "time") == 0) {
      times = fopen("times/times9.csv", "w");
    } else if (strcmp(argv[1], "flops") == 0) {
      flops = fopen("flops/flops9.csv", "w");
    } else if (strcmp(argv[1], "vecops") == 0) {
      vecops = fopen("vecops/vecops9.csv", "w");
    }

    for (SIZE = 10; SIZE <= 1200; SIZE += 10) {
      EventSet = PAPI_NULL;
      double *matrix = (double*)malloc(SIZE * SIZE * sizeof(double));

      init_rand_matrix(matrix, SIZE);

      if (strcmp(argv[1], "time") == 0) { // measure time
        dtime = dclock();
      } else if (strcmp(argv[1], "flops") == 0) { // measure flops
        if ((retval = PAPI_flops_rate(PAPI_DP_OPS, &ireal_time, &iproc_time, &iflpops, &imflops)) < PAPI_OK) { 
          printf("retval: %d\n", retval);
          exit(1);
        }
      } else if (strcmp(argv[1], "vecops") == 0) { // measure vecops
        if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT ) {
          fprintf(stderr, "Error: %s\n", errstring);
          exit(1);            
        }
        
        if ((retval = PAPI_create_eventset(&EventSet)) != PAPI_OK) {
          ERROR_RETURN(retval);
        }
        
        if ((retval = PAPI_add_events(EventSet, event_codes, NUM_EVENT)) != PAPI_OK){
          ERROR_RETURN(retval);
        }
        
        if ((retval = PAPI_start(EventSet)) != PAPI_OK){
          ERROR_RETURN(retval);
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
      } else if (strcmp(argv[1], "vecops") == 0) { // measure vecops
        if ((retval = PAPI_stop(EventSet, values)) != PAPI_OK) {
          ERROR_RETURN(retval);
        }

        fprintf(vecops, "%d,%lld,%lld\n", SIZE, values[0], values[1]);
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
