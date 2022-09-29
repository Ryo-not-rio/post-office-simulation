#ifndef __CUSTOMER_H
#define __CUSTOMER_H

typedef struct CustomerStruct {
    int waitTime;
    int waitLimit;
    int serveTime;
} Customer;

Customer *makeCustomer(float averageWaitTime, float waitTimeSD, float averageServeTime, float serveTimeSD);
void printCustomer(Customer *customer);
#endif