#ifndef TRANSACTIONS
#define TRANSACTIONS

struct Transaction
{   char loginID[30];
    int transactionID; 
    int accountNumber;
    int operation; // 0 -> Withdraw, 1 -> Deposit,2 -> Transfer
    int transferAcc;
    long int oldBalance;
    long int newBalance;
    char transactionTime[100];

};

#endif