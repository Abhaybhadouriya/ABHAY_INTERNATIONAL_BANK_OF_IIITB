#ifndef LOANAPPLY
#define LOANAPPLY

#include <time.h>

struct Loanapply
{
    int accountNumber;
    long int newBalance;
    int status; //0->applied , 1->assigned ,2->approved,3->declined
    int approvedByEMP;
    time_t processedTime;
    time_t appliedTime;
};

#endif