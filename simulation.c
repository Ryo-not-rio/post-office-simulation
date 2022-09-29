#include <gsl/gsl_randist.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "./headers/simulation.h"
#include "./headers/queue.h"
#include "./headers/customer.h"

void oneTimeStep(Queue *queue, unsigned int *currentlyServing, unsigned long *served, unsigned long *timedOut, unsigned int *totalTime, unsigned int *totalNum, Customer *tills[], unsigned int tillNum);

void oneSimulation(FILE *fptr, int logEachStep, int timeSteps, float arriveRate, int tillNum, int maxLength, 
                   float averageWaitLimit, float waitLimitSD, float averageServeTime, float serveTimeSD,
                   unsigned long *servedLog, unsigned long *unfulfilledLog, unsigned long *timedOutLog, unsigned long *emptyTimeLog, double *averageWaitTimeLog);
void runSimulationLoop(FILE *fptr, int numSimulations, float params[]);

void oneTimeStep(Queue *queue, unsigned int *currentlyServing, unsigned long *served, unsigned long *timedOut, unsigned int *totalTime, unsigned int *totalNum, Customer *tills[], unsigned int tillNum) {
    /* Serving customers*/
    int i;

    Customer *customer;
    for (i=0; i<tillNum; i++) {
        if (tills[i] == NULL) {
            /* If there is a free till serve a customer */
            if ((customer = dequeue(queue)) != NULL) {
                tills[i] = customer;
                (*totalNum)++;
                (*currentlyServing)++;
                *totalTime += customer -> waitLimit - customer -> waitTime;
            }
        } else {
            /* Process a customer being served */
            customer = tills[i];
            customer -> serveTime--;
            if (customer -> serveTime == 0) {
                /* Customer is done being served */
                (*served)++;
                (*currentlyServing)--;
                tills[i] = NULL;
                i--;
                free(customer);
            }
        }
    }

    /* Process customers in queue */
    Node *node = queue -> head;
    while (node != NULL) {
        customer = node -> value;
        customer -> waitTime--;
        if (customer -> waitTime == 0) {
            /* Customer has timed out */
            Node *tempNode = node -> next;
            queueRemove(queue, node);
            (*timedOut)++;
            node = tempNode;
        } else {
            node = node -> next;
        }
    }
}

void oneSimulation(FILE *fptr, int logEachStep, int timeSteps, float arriveRate, int tillNum, int maxLength, 
                   float averageWaitLimit, float waitLimitSD, float averageServeTime, float serveTimeSD,
                   unsigned long *servedLog, unsigned long *unfulfilledLog, unsigned long *timedOutLog,
                   unsigned long *emptyTimeLog, double *averageWaitTimeLog) {

    Queue *queue = makeQueue();
    int timeCount = 0;
    
    unsigned int currentlyServing = 0;
    unsigned int totalTime = 0;
    unsigned int totalCustomerNum = 0;

    Customer **tills;
    int i;

    if (!(tills = (Customer **)malloc(sizeof(Customer *)*tillNum))) {
        printf("Could not allocate memory for tills\n");
        exit(1);
    }
    for (i=0; i<tillNum; i++) {
        tills[i] = NULL;
    }

    /* Simulate for the specified number of time steps and then serve all remaining customers */
    while (timeCount < timeSteps || queue -> length > 0 || currentlyServing > 0) {        
        oneTimeStep(queue, &currentlyServing, servedLog, timedOutLog, &totalTime, &totalCustomerNum, tills, tillNum);

        /* Add Customer to queue if post office is open */
        if (timeCount < timeSteps) {
            for (i=0; i<gsl_ran_poisson(shared_r, (double) arriveRate); i++) {
                if (queue -> length < maxLength || maxLength == -1) {
                    /* Customer joins queue */
                    Customer *new_customer = makeCustomer(averageWaitLimit, waitLimitSD, averageServeTime, serveTimeSD);
                    queueAdd(queue, new_customer);
                } else (*unfulfilledLog)++;
            }
        }

        if (timeCount >= timeSteps) {
            if (*emptyTimeLog == 0 && logEachStep) fprintf(fptr, "---Closing Time---\n");
            (*emptyTimeLog)++;
        }

        /* Log each loop if logEachStep == True */
        if (logEachStep) {
            fprintf(fptr, "%d: beingServed=%d, peopleInQueue=%d, fulfilled=%d, unfulfilled=%d, timed_out=%d\n", 
                           timeCount, currentlyServing, queue->length, *servedLog, *unfulfilledLog, *timedOutLog);
        }
        timeCount++;
    }
    free(queue);
    free(tills);
    *averageWaitTimeLog += (double)totalTime/totalCustomerNum;
}

void runSimulationLoop(FILE *fptr, int numSimulations, float params[]) {
    fprintf(fptr, "arriveRate, tillNum, maxLength, averageWaitLimit, waitLimitSD, averageServeTime, serveTimeSD\n");
    fprintf(fptr, "%.3f, %d, %d, %.3f, %.3f, %.3f, %.3f\n", 
            params[3], (int)params[1], (int)params[0], params[4], params[5], params[6], params[7]);

    /* These variables are passed around as pointers to log all the information necessary */
    unsigned long servedLog = 0;
    unsigned long unfulfilledLog = 0;
    unsigned long timedOutLog = 0;
    unsigned long emptyTimeLog = 0;
    double averageWaitTimeLog = 0;

    /* Simulating for specified amount of times */
    int i;
    for (i=0; i<numSimulations; i++) {
        oneSimulation(
            fptr,
            numSimulations == 1,
            params[2], params[3], params[1], params[0], params[4], params[5], params[6], params[7],
            &servedLog, &unfulfilledLog, &timedOutLog, &emptyTimeLog, &averageWaitTimeLog
        );
    }

    /* Writing to end of file */
    if (numSimulations == 1) {
        fprintf(fptr, "Time taken to empty queue after closing: %d\n", emptyTimeLog);
        fprintf(fptr, "Average wait time for fulfilled customers: %lf\n", averageWaitTimeLog);
    } else {
        fprintf(fptr, "Average fulfilled customers: %lf\n", (double)servedLog/numSimulations);
        fprintf(fptr, "Average unfulfilled customers: %lf\n", (double)unfulfilledLog/numSimulations);
        fprintf(fptr, "Average timed out customers: %lf\n", (double)timedOutLog/numSimulations);
        fprintf(fptr, "Average of \"Average wait time for fulfilled customers\": %lf\n", averageWaitTimeLog/numSimulations);
        fprintf(fptr, "Average time taken to empty queue after closing: %lf\n", (double)emptyTimeLog/numSimulations);
    }
}


void runSimulations(int numRunPrograms, int numSimulationsPerRun, pair params[], int numParams, char fileName[]) {
    FILE *fptr;
    char useFileName[100];

    /**
     * Array to be filled with parameters to be simulated with, which are chosen 
     * from the range in the params[] array.
     */
    float *passParams;
    if (!(passParams = (float *)malloc(sizeof(float)*numParams))) {
        printf("Could not allocate array for parameters\n");
        exit(1);
    }

    /* Repeating simulations for the specified number of times */
    int i;
    for (i=0; i<numRunPrograms; i++) {

        /* Changing output folder to ./experiments if repetition num > 1 */
        if (numRunPrograms > 1) {
            struct stat st = {0};

            if (stat("./experiments", &st) == -1) {
                mkdir("./experiments", 0700);
            }
            strcpy(useFileName, "./experiments/");
        }
        else strcpy(useFileName, fileName);

        /* Setting the parameters and fileName*/
        int j;
        for (j=0; j<numParams; j++) {
            /* Choosing specific parameter from range */
            float rand_num = gsl_rng_uniform_pos(shared_r);
            float param = rand_num * (params[j].item2-params[j].item1) + params[j].item1;
            passParams[j] = param;

            /**
             * Creating file name according to chosen parameter if 
             * repetition num > 1
             */
            if (numRunPrograms > 1) {
                char temp[7];
                sprintf(temp, "_%3.1f", param);
                int k;
                
                for (k=0; k<strlen(temp); k++) {
                    /* Replace any . with - in the parameter and add it to end of file name */
                    if (temp[k] != '.') {
                        int length = strlen(useFileName);
                        useFileName[length] = temp[k];
                        useFileName[length+1] = '\0';
                    } else {
                        strcat(useFileName, "-");
                    }
                }
            }
        }

        if (numRunPrograms > 1){
            strcat(useFileName, ".txt");
        }
        fptr = fopen(useFileName, "w");

        runSimulationLoop(fptr, numSimulationsPerRun, passParams);
        
        fclose(fptr);
    }
    free(passParams);
}