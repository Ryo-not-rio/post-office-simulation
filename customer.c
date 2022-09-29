#include <stdlib.h>
#include <gsl/gsl_randist.h>

#include "./headers/main.h"
#include "./headers/customer.h"

Customer *makeCustomer(float averageWaitTime, float waitTimeSD, float averageServeTime, float serveTimeSD) {
    Customer *customer;
    if (!(customer = (Customer *)malloc(sizeof(Customer)))) {
        printf("%s\n", "Failed to allocate memory for customer");
        exit(1);
    }

    /** Set wait time with a gaussian distribution of mean averageWaitTime, SD waitTimeSD,
     * with a minimum value of 1.
     * Serve time set in same manner.
     * 0.5 is added to both time to account for 
     **/

    /* waitLimit stores the customer's limit and waitTime is used to
    * keep track of how long the user has left in their limit.
    */
    int waitTime = gsl_ran_gaussian(shared_r, waitTimeSD) + averageWaitTime + 0.5;
    if (waitTime < 1) waitTime = 1;
    customer -> waitTime = waitTime;
    customer -> waitLimit = waitTime;

    int serveTime = gsl_ran_gaussian(shared_r, serveTimeSD) + averageServeTime + 0.5;
    if (serveTime < 1) serveTime = 1;
    customer -> serveTime = serveTime;
    return customer;
}

void printCustomer(Customer *customer) {
    printf("Customer wait limit: %d\n", customer -> waitTime);
    printf("Customer serve time: %d\n", customer -> serveTime);
}

