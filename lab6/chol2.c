#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

/* Include PAPI */
#include <papi.h>

/* PAPI macro helpers definitions */
#define NUM_EVENT 2
#define THRESHOLD 100000
#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d %s:line %d: \n", retval,__FILE__,__LINE__);  exit(retval); }

/* matrix indexation */
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



int chol(double *A, unsigned int n){
    register unsigned int i,j,k; //OPT1
    register double tmp; //OPT1

    for (j = 0; j < n; j++) {
        for (i = j; i < n; i++) {
            tmp = A[IDX(i, j, n)]; // OPT1
            for (k = 0; k < j; k++) {
                tmp -= A[IDX(i, k, n)] * A[IDX(j, k, n)]; //OPT1
            }
            A[IDX(i, j, n)] = tmp; //OPT1
        }

        if (A[IDX(j, j, n)] < 0.0) {
            return (1);
        }

        A[IDX(j,j,n)] = sqrt(A[IDX(j,j,n)]); //OPT1
        tmp = 1.0 / A[IDX(j,j,n)]; //OPT1
        for (i = j + 1; i < n; i++){
            A[IDX(i, j, n)] *= tmp; //OPT1
        }
    }

    return (0);
}

int main(int argc, char ** argv) {
    double* A;
    double t1, t2;
    int i, j, n, ret;
    double dtime;
    
    /* PAPI FLOPS variables */
    float real_time, proc_time, mflops;
    long long flpops;
    float ireal_time, iproc_time, imflops;
    long long iflpops;
    int retval;
    
    /* PAPI counters variables */
    int tmp;
    int EventSet = PAPI_NULL;
    int event_codes[NUM_EVENT] = { PAPI_L3_DCA, PAPI_PRF_DM }; 
    char errstring[PAPI_MAX_STR_LEN];
    long long values[NUM_EVENT];

    /* read matrix size */
    n = atoi(argv[1]);
    A = malloc(n * n * sizeof(double));
    assert(A != NULL);

    /* fill matrix */
    for (i = 0; i < n; i++) {
        A[IDX(i, i, n)] = 1.0;
    }

    /* default value of what to measure */
    int measure = 0;
    if (argc > 2) {
        measure = atoi(argv[2]);
    }

    /* measurements */
    if (measure == 0) {
        dtime = dclock();
    } else if (measure == 1) {
        if ((retval = PAPI_flops_rate(PAPI_DP_OPS, &ireal_time, &iproc_time, &iflpops, &imflops)) < PAPI_OK) { 
            printf("Could not initialise PAPI_flops\n");
            printf("Your platform may not support floating point operation event.\n"); 
            printf("retval: %d\n", retval);
            exit(1);
        }
    } else if (measure == 2){
        /* initializing library */
        if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT) {
            fprintf(stderr, "Error: %s\n", errstring);
            exit(1);            
        }

        /* Creating event set */
        if ((retval = PAPI_create_eventset(&EventSet)) != PAPI_OK) {
            ERROR_RETURN(retval);
        }

        /* Add the array of events PAPI_TOT_INS and PAPI_TOT_CYC to the eventset */
        if ((retval = PAPI_add_events(EventSet, event_codes, NUM_EVENT)) != PAPI_OK) {
            ERROR_RETURN(retval);
        }

        /* Start counting */
        if ((retval = PAPI_start(EventSet)) != PAPI_OK) {
            ERROR_RETURN(retval);
        }
    }
    
    if (chol(A, n)) {
      fprintf(stderr, "Matrix is not symmetric or not positive definite\n");
    }
    
    if (measure == 0) {
        dtime = dclock() - dtime;
        printf("Time: %le \n", dtime);
    } else if (measure == 1) {
        if((retval = PAPI_flops_rate(PAPI_DP_OPS, &real_time, &proc_time, &flpops, &mflops)) < PAPI_OK) {
            printf("retval: %d\n", retval);
            exit(1);
        }
        printf("Real_time: %f Proc_time: %f flpops: %lld MFLOPS: %f\n", real_time, proc_time, flpops, mflops);
    } else if (measure == 2) {
        /* Stop counting, this reads from the counter as well as stop it. */
        if ((retval = PAPI_stop(EventSet, values)) != PAPI_OK) {
            ERROR_RETURN(retval);
        }
        printf("Level 3 data cache accesses: %lld, data prefetch cache misses: %lld\n", values[0], values[1]);
    }

    fflush(stdout);
    free(A);
    return 0;
}
