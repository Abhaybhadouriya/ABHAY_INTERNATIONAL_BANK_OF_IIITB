#ifndef TRANSACTIONS
#define TRANSACTIONS

#include <time.h>

struct Transaction
{
    int transactionID; 
    int accountNumber;
    int operation; // 0 -> Withdraw, 1 -> Deposit,2 -> Transfer
    int transferAcc;
    long int oldBalance;
    long int newBalance;
    time_t transactionTime;
};

#endif