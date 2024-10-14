#ifndef EMPLOYEE_FUNCTIONS
#define EMPLOYEE_FUNCTIONS

#include "../resource/commanFun.h"
#include "../admin/admin.h"
#include "../resource/constantTerms.h"
#include "../resource/employeeNeeds.h"
#include "../recordStruct/structs.h"
#include "../recordStruct/client_data.h"
#include "../resource/set.h"
#include "../resource/shFile.h"
#include <sys/stat.h>  // For file mode constants like S_IRWXU
#include <crypt.h>
#include <stdbool.h>

// Semaphores are necessary joint account due the design choice I've made
#include <sys/ipc.h>
#include <sys/sem.h>


// Function Prototypes =================================

bool employee_operation_handler(int connFD);
bool view_employee_account(int connFD,int type,int range,char *str);
bool View_account_trans(int connFD);

// bool logout(int connFD)
// bool login_handler(bool isAdmin, int connFD, struct Account *ptrToCustomer);

// =====================================================

// Function Definition =================================

// =====================================================
int semIdentifier;

bool employee_operation_handler(int connFD)
{    
    struct clientData clientData;
    struct Employee employee;
     
    if (employeee_login_handler(connFD,&employee,&clientData,1))
    {
        ssize_t writeBytes, readBytes;            // Number of bytes read from / written to the client
        char readBuffer[1000], writeBuffer[1000]; // A buffer used for reading & writing to the client







       
        key_t semKey = ftok(ACCOUNT_FILE, clientData.userid); 
        union semun
        {
            int val; 
        } semSet;

        int semctlStatus;
        semIdentifier = semget(semKey, 1, 0); 
        if (semIdentifier == -1)
        {
            semIdentifier = semget(semKey, 1, IPC_CREAT | 0700);
            if (semIdentifier == -1)
            {
                perror("Error while creating semaphore!");
                _exit(1);
            }

            semSet.val = 1; 
            semctlStatus = semctl(semIdentifier, 0, SETVAL, semSet);
            if (semctlStatus == -1)
            {
                perror("Error while initializing a binary sempahore!");
                _exit(1);
            }
        }


        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, EMPLOYEE_LOGIN_WELCOME);
        while (1)
        {
            strcat(writeBuffer, "\n");
            strcat(writeBuffer, EMPLOYEE_MENU);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing ADMIN_MENU to client!");
                return false;
            }
            bzero(writeBuffer, sizeof(writeBuffer));

            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1)
            {
                perror("Error while reading client's choice for ADMIN_MENU");
                return false;
            }

            int choice = atoi(readBuffer);
            switch (choice)
            {
            
            case 1:
                add_customer(connFD);
                break;
            case 2: 
                updateDetails(connFD, false);
                break;
            case 3:
                // view_employee_account(connFD,1,-1,"");
                break;
            case 4:
                // add_employee(connFD);
                break;
            case 5:
                View_account_trans(connFD);
                break;
            case 6:
                change_password(connFD,EMPLY_TYPE,semIdentifier,clientData);            
                break;
            case 7:
                logout(connFD,clientData.username);
                break;
            default:
                write(connFD,"Enter Valid Input\n",strlen("Enter Valid Input\n"));
                break;
            }
        }
    }
    else
    {
        return false;
    }
    return true;
}

bool View_account_trans(int connFD){
    char readBuffer[1000],writeBuffer[1000];
    write(connFD,"Enter the Customer ID to print the Pass Book",strlen("Enter the Customer ID to print the Pass Book"));
    int readBytes=read(connFD,readBuffer,sizeof(readBuffer));
    if(readBytes==-1){
        perror("Error while reading Input : ");
        return false;
    }
    view_transections(connFD,readBuffer);
    return true;
}

#endif