#ifndef LOANAPPLY
#define LOANAPPLY

#include <time.h>

struct Loanapply
{
    int accountNumber;
    char nameAccountHolder;
    long int newBalance;
    int handleByEmpID;
    char nameEmployee[30];    
    int status; //0->applied , 1->assigned ,2->approved,3->declined
    int approvedByEMP;
    time_t processedTime;
    time_t appliedTime;
};

#endif