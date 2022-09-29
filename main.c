#include <time.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <limits.h>

#include "./headers/main.h"
#include "./headers/queue.h"
#include "./headers/customer.h"
#include "./headers/simulation.h"
#include "./headers/fileHandling.h"

gsl_rng *shared_r;
int main(int argc, char **argv) {
    const gsl_rng_type *T;
    T = gsl_rng_default;
    shared_r = gsl_rng_alloc(T);
    gsl_rng_env_setup();
    gsl_rng_set(shared_r, time(0));

    if (argc < 4) {
        printf("Not enough arguments\n");
        exit(1);
    }

    char *allowedParams[] = {
        "maxQueueLength",
        "numServicePoints",
        "closingTime",
        "averageNewCustomersPerInterval",
        "averageCustomerWaitLimit",
        "customerWaitLimitSD",
        "averageCustomerServeTime",
        "customerServeTimeSD"
    };

    int numParams = sizeof(allowedParams)/sizeof(allowedParams[0]);

    /* Read parameters from file */
    char *file = argv[1];
    pair *params = readParameters(file, allowedParams, numParams);

    /* Closing time cannot be a range */
    params[2].item2 = params[2].item1;

    /* Parsing ranges in the parameter file */
    int i;
    for (i=0; i<numParams; i++) {
        if ((params[i].item1 < 0 && i !=0) || (params[i].item2 < params[i].item1 && (int)params[i].item2 != 0)) {
            printf("Invalid argument found in file for parameter %s. Exiting...\n", allowedParams[i]);
            exit(1);
        } 
        if ((int)params[i].item2 == 0) params[i].item2 = params[i].item1;
    }

    /* Parsing number of simulations from terminal argument */
    char *endptr;
    errno = 0;
    unsigned int numSimulations = strtol(argv[2], &endptr, 10);
    if (endptr == argv[2])
    {
        printf("Could not parse number of simulations\n");
        exit(1);
    }
    if ((numSimulations == ULONG_MAX || numSimulations == 0) && errno == ERANGE)
    {
        printf("Number of simulations out of range\n");
        exit(1);
    }
    
    /* Parsing number of repeats from terminal argument */
    unsigned int numRepeats;
    if (argc < 5) numRepeats = 1;
    else {
        errno = 0;
        numRepeats = strtol(argv[4], &endptr, 10);
        if (endptr == argv[4] ||
            ((numSimulations == ULONG_MAX || numSimulations == 0) && errno == ERANGE) ||
            numRepeats <= 0)
        {
            printf("Could not parse number of times to repeat the program. Using default value of 1\n");
            numRepeats = 1;
        }
    }
    
    /* Run simulations including repetitions */
    runSimulations(numRepeats, numSimulations, params, numParams, argv[3]);

    /** For gathering data */
    /*
    int n;
    for (n=2; n<=100; n++) {
        params[3].item1 = (double) n*0.1;
        params[3].item2 = (double) n*0.1;
        runSimulations(numRepeats, numSimulations, params, numParams, argv[3]);
    }
    */
  
    
    gsl_rng_free(shared_r);
    free(params);
    return 0;
}