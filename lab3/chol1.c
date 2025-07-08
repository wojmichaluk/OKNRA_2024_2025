#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#define IDX(i, j, n) (((j)+ (i)*(n)))



static double gtod_ref_time_sec = 0.0;

/* Adapted from the bl2_clock() routine in the BLIS library */

double dclock(){
    double the_time, norm_sec;
    struct timeval tv;
    gettimeofday( &tv, NULL );
    if ( gtod_ref_time_sec == 0.0 )
        gtod_ref_time_sec = ( double ) tv.tv_sec;
    norm_sec = ( double ) tv.tv_sec - gtod_ref_time_sec;
    the_time = norm_sec + tv.tv_usec * 1.0e-6;
    return the_time;
}



int chol(double * A, unsigned int n){
    int i, j, k;

    for (j = 0; j < n; j++) {
        for (i = j; i < n; i++) {
            for (k = 0; k < j; k++) {
                A[IDX(i, j, n)] -= A[IDX(i, k, n)] * A[IDX(j, k, n)];
            }
        }

        if (A[IDX(j, j, n)] < 0.0) {
            return (1);
        }

        A[IDX(j, j, n)] = sqrt(A[IDX(j, j, n)]);
        for (i = j + 1; i < n; i++){
            A[IDX(i, j, n)] /= A[IDX(j, j, n)];
        }
    }

    return (0);
}

int main(int argc, char ** argv){
    double* A;
    double t1, t2;
    int i, j, n, ret;
    double dtime;

    n = atoi(argv[1]);
    A = malloc(n * n * sizeof(double));
    assert(A != NULL);

    for (i = 0; i < n; i++) {
        A[IDX(i, i, n)] = 1.0;
    }

    dtime = dclock();
    if (chol(A, n)) {
      fprintf(stderr,"Matrix is not symmetric or not positive definite\n");
    }
    dtime = dclock()-dtime;
    printf( "Time: %le \n", dtime);

    fflush( stdout );

    free(A);
    return 0;
}

