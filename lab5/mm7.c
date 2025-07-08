#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define BLOCK_SIZE 64

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


//subroutine for optimization
int mm(double ** first, double ** second, double ** multiply, int SIZE)
{
  int i,j,k;
  int ii, jj, kk; 
  double a_val;
  double local_first[BLOCK_SIZE * BLOCK_SIZE];
  double local_second[BLOCK_SIZE * BLOCK_SIZE];

  for (kk = 0; kk < SIZE; kk += BLOCK_SIZE) {
    for (ii = 0; ii < SIZE; ii += BLOCK_SIZE) {
      for (i = 0; i < BLOCK_SIZE && (ii + i) < SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE && (kk + j) < SIZE; j++) {
          local_first[i * BLOCK_SIZE + j] = first[ii+i][kk+j];
        }
      }
      for (jj = 0; jj < SIZE; jj += BLOCK_SIZE) {
        for (i = 0; kk + i < SIZE && i < BLOCK_SIZE; i++) {
          for (j = 0; jj + j < SIZE && j < BLOCK_SIZE; j++) {
            local_second[i * BLOCK_SIZE + j] = second[kk+i][jj+j];
          }
        }
        for (i = ii; i < ii + BLOCK_SIZE && i < SIZE; i++) { //rows in multiply
          for (k = kk; k < kk + BLOCK_SIZE && k < SIZE; k++) { //columns in first and rows in second
            a_val = local_first[(i-ii) * BLOCK_SIZE + (k-kk)];
            for (j = jj; j < jj + BLOCK_SIZE && j < SIZE; j++) { //columns in multiply
              multiply[i][j] += a_val * local_second[(k-kk) * BLOCK_SIZE + (j-jj)];
            }
          }
        }
      }
    }
  }
  
  return 0;
}



int main( int argc, const char* argv[] ){
  int i,j,iret;
  double ** first;
  double ** second;
  double ** multiply;

  double * first_;
  double * second_;
  double * multiply_;

  int optimal = 1;

  double dtime;

  if (argc == 3)
    optimal = atoi(argv[2]);;

  int SIZE = atoi(argv[1]);

//allocate blocks of continous memory
  first_ = (double*) malloc(SIZE*SIZE*sizeof(double));
  second_ = (double*) malloc(SIZE*SIZE*sizeof(double));
  multiply_ = (double*) malloc(SIZE*SIZE*sizeof(double));

//allocate 2D matrices
  first = (double**) malloc(SIZE*sizeof(double*));
  second = (double**) malloc(SIZE*sizeof(double*));
  multiply = (double**) malloc(SIZE*sizeof(double*));

  if(optimal){
//set pointers to continous blocks
    for (i = 0; i < SIZE; i++) {
      first[i] = first_ + i*SIZE;
      second[i] = second_ + i*SIZE;
      multiply[i] = multiply_ + i*SIZE;
    }
  }
  else{
    for (i = 0; i < SIZE; i++) {
      first[i] = (double*) malloc(SIZE*sizeof(double));
      second[i] = (double*) malloc(SIZE*sizeof(double));
      multiply[i] = (double*) malloc(SIZE*sizeof(double));
    }
  }


//fill matrices with test data
  for (i = 0; i < SIZE; i++) { //rows in first
    for (j = 0; j < SIZE; j++) { //columns in first
      first[i][j]=i+j;
      second[i][j]=i-j;
      multiply[i][j]=0;
    }
  }

//measure mm subroutine computation time
  dtime = dclock();
  iret = mm(first,second,multiply,SIZE);
  dtime = dclock()-dtime;
  printf( "Time: %le \n", dtime);


  fflush( stdout );

//cleanup
  free(first_);
  free(second_);
  free(multiply_);

  if(!optimal){
    for (i = 0; i < SIZE; i++) {
      free(first[i]);
      free(second[i]);
      free(multiply[i]);
    }
  }

  free(first);
  free(second);
  free(multiply);

  return iret;
}
