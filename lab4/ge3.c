//requires additional changes to the code to make it work

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

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

int ge(double ** A, int SIZE)
{
  register int i,j,k;
  register double multiplier;

  for (k = 0; k < SIZE; k++) { 
    for (i = k+1; i < SIZE; i++) { 
      multiplier = (A[i][k]/A[k][k]);
      for (j = k+1; j < SIZE; j++) { 
         A[i][j] = A[i][j]-A[k][j]*multiplier;
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
  double **matrix = (double**)malloc(SIZE * sizeof(double*));
  double *matrix_ = (double*)malloc(SIZE * SIZE * sizeof(double));
  for (i = 0; i < SIZE; i++) {
    matrix[i] = matrix_ + i * SIZE;
  }

  srand(1);
  for (i = 0; i < SIZE; i++) { 
    for (j = 0; j < SIZE; j++) {
      matrix[i][j] = rand();
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
      check = check + matrix[i][j];
    }
  }
  printf( "Check: %le \n", check);
  fflush( stdout );
  free(matrix);
  free(matrix_);

  return iret;
}

