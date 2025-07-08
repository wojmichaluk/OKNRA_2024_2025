//requires additional changes to the code to make it work

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <immintrin.h>

#define BLKSIZE 8
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define IDX(i, j, n) (((j)+ (i)*(n)))

static double gtod_ref_time_sec = 0.0;

/* Adapted from the bl2_clock() routine in the BLIS library */

double dclock()
{
  double the_time, norm_sec;
  struct timeval tv;
  gettimeofday( &tv, NULL );
  if ( gtod_ref_time_sec == 0.0 )
    gtod_ref_time_sec = ( double ) tv.tv_sec;
  norm_sec = ( double ) tv.tv_sec - gtod_ref_time_sec;
  the_time = norm_sec + tv.tv_usec * 1.0e-6;
  return the_time;
}

int ge(double *A, int SIZE)
{
  register int i,j,k;
  double multiplier[4];
  register __m256d mm_multiplier;
  register __m256d tmp0, tmp1, tmp2, tmp3; 

  for (k = 0; k < SIZE; k++) { 
    for (i = k+1; i < SIZE; i++) { 
      multiplier[0] = (A[IDX(i, k, SIZE)]/A[IDX(k, k, SIZE)]);
      multiplier[1] = multiplier[0];
      multiplier[2] = multiplier[0];
      multiplier[3] = multiplier[0];
      mm_multiplier = _mm256_loadu_pd(multiplier);
      for (j = k+1; j < SIZE; ) { 
        if (j < (max(SIZE - BLKSIZE, 0))) {
          // load
          tmp0 = _mm256_loadu_pd(A+IDX(i, j, SIZE));
          tmp1 = _mm256_loadu_pd(A+IDX(k, j, SIZE));
          tmp2 = _mm256_loadu_pd(A+IDX(i, j+4, SIZE));
          tmp3 = _mm256_loadu_pd(A+IDX(k, j+4, SIZE));

          // multiply
          tmp1 = _mm256_mul_pd(tmp1, mm_multiplier);
          tmp3 = _mm256_mul_pd(tmp3, mm_multiplier);

          // subtraction
          tmp0 = _mm256_sub_pd(tmp0, tmp1);
          tmp2 = _mm256_sub_pd(tmp2, tmp3);

          // store
          _mm256_storeu_pd(A + IDX(i, j, SIZE), tmp0);
          _mm256_storeu_pd(A + IDX(i, j+4, SIZE), tmp2);

          j += BLKSIZE;
        } else {
          A[IDX(i, j, SIZE)] = A[IDX(i, j, SIZE)]-A[IDX(k, j, SIZE)]*multiplier[0];
          j++;
        }
      } 
    }
  }
  return 0;
}

int main( int argc, const char* argv[] )
{
  register int i,j;
  int k,iret;
  double dtime;
  int SIZE = 1500;

  // TODO - make near optimal dynamic allocation
  // instead of:
  // double matrix[SIZE][SIZE]; 
  // we have:
  double *matrix = (double*)malloc(SIZE * SIZE * sizeof(double));

  srand(1);
  for (i = 0; i < SIZE; i++) { 
    for (j = 0; j < SIZE; j++) {
      matrix[IDX(i, j, SIZE)] = rand();
    }
  }
  printf("call GE\n");
  dtime = dclock();
  iret = ge(matrix, SIZE);
  dtime = dclock()-dtime;
  printf("Time: %le \n", dtime);

  double check=0.0;
  for (i = 0; i < SIZE; i++) {
    for (j = 0; j < SIZE; j++) {
      check = check + matrix[IDX(i, j, SIZE)];
    }
  }
  printf( "Check: %le \n", check);
  fflush( stdout );
  free(matrix);

  return iret;
}

