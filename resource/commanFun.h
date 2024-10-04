#ifndef COMMON_FUNCTIONS
#define COMMON_FUNCTIONS

#include <stdio.h>     // Import for `printf` & `perror`
#include <unistd.h>    // Import for `read`, `write & `lseek`
#include <string.h>    // Import for string functions
#include <stdbool.h>   // Import for `bool` data type
#include <sys/types.h> // Import for `open`, `lseek`
#include <sys/stat.h>  // Import for `open`
#include <fcntl.h>     // Import for `open`
#include <stdlib.h>    // Import for `atoi`
#include <errno.h>     // Import for `errno`
#include <crypt.h>

// #include  "../database/account.abhay.bank"
// #include "../database/loan.abhay.bank"
// #include "../database/transactions.abhay.bank"
#include "./set.h"
#include "./shFile.h"
#include "../admin/admin_cred.h"
#include "./constantTerms.h"
#include "../recordStruct/account.h"
#include "../recordStruct/employee.h"
#include "../recordStruct/structs.h"
// Function Prototypes =================================

bool login_handler(bool isAdmin, int connFD, struct Account *ptrToCustomer);
bool get_account_details(int connFD, struct Account *customerAccount);
bool get_customer_details(int connFD, int customerID);
bool get_transaction_details(int connFD, int accountNumber);
char* getRole(int role);
int get_last_number_of_loginID(char *input);
bool view_employee_account(int connFD,int role,int range,char *str);
// =====================================================

// Function Definition =================================




bool view_employee_account(int connFD,int role,int range,char *str){
     ssize_t readBytes, writeBytes;             // Number of bytes read from / written to the socket
    char readBuffer[1000], writeBuffer[10000]; // A buffer for reading from / writing to the socket
    char tempBuffer[1000];
    char tempCustID[1000];
    int employeeID;
    struct Employee account;
    int customerFileDescriptor;
     
    struct flock lock = {F_RDLCK, SEEK_SET, 0, range==-1?0:sizeof(struct Employee), getpid()};
    int toSeek=0;
    
    // if requested by employee
    if (range != -1)
    {       
       toSeek = get_last_number_of_loginID(str);
        employeeID = atoi(readBuffer);
    }
    customerFileDescriptor = open(EMPLOYEE_FILE, O_RDONLY);
    if (customerFileDescriptor == -1)
    {
        // Customer File doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "No Employee");
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing EMPLOYEE_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    off_t fileSize = lseek(customerFileDescriptor, 0, SEEK_END);

    if(range!=-1&&fileSize<=toSeek*sizeof(struct Employee)){
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "NO Employee");
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing EMPLOYEE_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return true;
    }
    int offset = lseek(customerFileDescriptor, (range==-1?0:toSeek) * sizeof(struct Employee), SEEK_SET);
    write(connFD, writeBuffer, strlen(writeBuffer));
    if (errno == EINVAL)
    {
        // Customer record doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, "NO Employee");
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing EMPLOYEE_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    else if (offset == -1)
    {
        perror("Error while seeking to required Employee record!");
        return false;
    }
    lock.l_start = offset;

    int lockingStatus = fcntl(customerFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining read lock on the Customer file!");
        return false;
    }
    char printstr[10000];
    if(range!=-1){
    readBytes = read(customerFileDescriptor, &account, sizeof(struct Employee));
    
    if (readBytes == -1)
    {
        perror("Error reading customer record from file!");
        return false;
    }
    }else{
        struct  Employee employee;
       off_t offset = lseek(customerFileDescriptor, 0, SEEK_END); // Get the total size of the file

lseek(customerFileDescriptor, 0, SEEK_SET); // Reset file pointer to the start of the file
    bzero(writeBuffer, sizeof(writeBuffer));

for (off_t i = 0; i < offset; i += sizeof(struct Employee)) {
  
    ssize_t readBytes = read(customerFileDescriptor, &employee, sizeof(struct Employee));

    if (readBytes == sizeof(struct Employee)) {
        if((int)employee.role==role){
        sprintf(writeBuffer, "Empsloyee Details - \n\tID : %d\n\tName : %s\n\tGender : %c\n\tAge: %d\n\tLoginID : %s\n\tRole : %s\n\n", employee.empID, employee.name, employee.gender, employee.age,employee.login,getRole(employee.role));
    //    *printstr+=writeBuffer;
        strcat(printstr, writeBuffer);}
    } else if (readBytes == 0) {
        break;
    } else {
        break;
    }
}

    }
    lock.l_type = F_UNLCK;
    fcntl(customerFileDescriptor, F_SETLK, &lock);
    if(range !=-1){
    bzero(writeBuffer, sizeof(writeBuffer));
      if (strcmp(tempCustID, account.login) != 0){
        strcpy(writeBuffer, CUSTOMER_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
     
      }else{
    
    sprintf(writeBuffer, "Empsloyee Details - \n\tID : %d\n\tName : %s\n\tGender : %c\n\tAge: %d\n\tLoginID : %s", account.empID, account.name, account.gender, account.age,account.login);
      }
    strcat(writeBuffer, "\n\nYou'll now be redirected to the main menu...^");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    }else {
    writeBytes = write(connFD,printstr, strlen(printstr));
    }
    if (writeBytes == -1)
    {
        perror("Error writing Employee info to client!");
        return false;
    }
    bzero(writeBuffer, sizeof(writeBuffer));
    bzero(printstr,sizeof(printstr));

    // readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
    return true;
};
bool login_handler(bool isAdmin, int connFD, struct Account *ptrToCustomerID)
{
    ssize_t readBytes, writeBytes;            // Number of bytes written to / read from the socket
    char readBuffer[1000], writeBuffer[1000]; // Buffer for reading from / writing to the client
    char tempBuffer[1000];
    struct Account account;

    int ID;

    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));

    // Get login message for respective user type
    if (isAdmin)
        strcpy(writeBuffer, ADMIN_LOGIN_WELCOME);
    else
        strcpy(writeBuffer, CUSTOMER_LOGIN_WELCOME);

    // Append the request for LOGIN ID message
    strcat(writeBuffer, "\n");
    strcat(writeBuffer, ENTER_LOGIN_ID);

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing WELCOME & LOGIN_ID message to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading login ID from client!");
        return false;
    }

    bool userFound = false;

    if (isAdmin)
    {
        if (strcmp(readBuffer, ADMIN_USER_NAME) == 0)
            userFound = true;
    }
    else
    {
        bzero(tempBuffer, sizeof(tempBuffer));
        strcpy(tempBuffer, readBuffer);
        // strtok(tempBuffer, "-");
        // ID = atoi(strtok(NULL, "-"));
        ID  =  get_last_number_of_loginID(tempBuffer);
        int customerFileFD = open(ACCOUNT_FILE, O_RDONLY);
        if (customerFileFD == -1)
        {
            perror("Error opening customer file in read mode!");
            return false;
        }

        off_t offset = lseek(customerFileFD, ID * sizeof(struct Account), SEEK_SET);
        if (offset >= 0)
        {
            struct flock lock = {F_RDLCK, SEEK_SET, ID * sizeof(struct Account), sizeof(struct Account), getpid()};

            int lockingStatus = fcntl(customerFileFD, F_SETLKW, &lock);
            if (lockingStatus == -1)
            {
                perror("Error obtaining read lock on customer record!");
                return false;
            }

            readBytes = read(customerFileFD, &account, sizeof(struct Account));
            if (readBytes == -1)
            {
                ;
                perror("Error reading customer record from file!");
            }

            lock.l_type = F_UNLCK;
            fcntl(customerFileFD, F_SETLK, &lock);
            char accountNumberStr[100];
            // Buffer to hold the string representation of the account number
            snprintf(accountNumberStr, sizeof(accountNumberStr), "%s", account.login); 
            // Convert int to string
            if (strcmp(accountNumberStr, readBuffer) == 0)
                userFound = true;
            
                else{
                 writeBytes = write(connFD, INVALID_LOGIN, strlen(INVALID_LOGIN));
                     close(customerFileFD);
                return false;
            }
              
            close(customerFileFD);
        }
        else
        {
            writeBytes = write(connFD, CUSTOMER_LOGIN_ID_DOESNT_EXIT, strlen(CUSTOMER_LOGIN_ID_DOESNT_EXIT));
        }
    }

    if (userFound)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        writeBytes = write(connFD, ENTER_PASSWORD, strlen(ENTER_PASSWORD));
        if (writeBytes == -1)
        {
            perror("Error writing PASSWORD message to client!");
            return false;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == 1)
        {
            perror("Error reading password from the client!");
            return false;
        }

        char hashedPassword[1000];
        strcpy(hashedPassword, crypt(readBuffer, SALT_BAE));
        if (isAdmin)
        {
            if (strcmp(hashedPassword, ADMIN_PASS_WORD) == 0)
                return true;
        }
        else
        {
            if (strcmp(hashedPassword, account.password) == 0)
            {

               if(!add_to_shared_set(account.login))
               {
                bzero(writeBuffer, sizeof(writeBuffer));
                writeBytes = write(connFD,CUSTOMER_ALREADY_LOGGED_IN, strlen(CUSTOMER_ALREADY_LOGGED_IN));
                return false;
               }
               else 
               return true;
            }
        }

        bzero(writeBuffer, sizeof(writeBuffer));
        writeBytes = write(connFD, INVALID_PASSWORD, strlen(INVALID_PASSWORD));
    }
    else
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        writeBytes = write(connFD, INVALID_LOGIN, strlen(INVALID_LOGIN));
    }

    return false;
}

bool get_customer_details(int connFD, int customerID)
{
    ssize_t readBytes, writeBytes;             // Number of bytes read from / written to the socket
    char readBuffer[1000], writeBuffer[10000]; // A buffer for reading from / writing to the socket
    char tempBuffer[1000];
    char tempCustID[1000];

    struct Account account;
    int customerFileDescriptor;
    struct flock lock = {F_RDLCK, SEEK_SET, 0, sizeof(struct Account), getpid()};
    int toSeek=0;
    if (customerID == -1)
    {
        writeBytes = write(connFD, GET_CUSTOMER_ID, strlen(GET_CUSTOMER_ID));
        if (writeBytes == -1)
        {
            perror("Error while writing GET_CUSTOMER_ID message to client!");
            return false;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error getting customer ID from client!");
            ;
            return false;
        }
        strcpy(tempCustID, readBuffer);
        // tempCustID =readBuffer;
       toSeek = get_last_number_of_loginID(readBuffer);
        customerID = atoi(readBuffer);
        // printf("%d",customerID);
    }

    customerFileDescriptor = open(ACCOUNT_FILE, O_RDONLY);
    if (customerFileDescriptor == -1)
    {
        // Customer File doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, CUSTOMER_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing CUSTOMER_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    off_t fileSize = lseek(customerFileDescriptor, 0, SEEK_END);
    // char buf1[10];
    // char buf2[10];
    //     sprintf(buf1, "%ld", fileSize);
    //     sprintf(buf2,"%ld",toSeek*sizeof(struct Account));
    //     sprintf(writeBuffer,"%s - %s",buf1,buf2);
    //     write(connFD,writeBuffer,strlen(writeBuffer));
    if(fileSize<=toSeek*sizeof(struct Account)){
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, CUSTOMER_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing CUSTOMER_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return true;
    }
    int offset = lseek(customerFileDescriptor, toSeek * sizeof(struct Account), SEEK_SET);
    write(connFD, writeBuffer, strlen(writeBuffer));
    if (errno == EINVAL)
    {
        // Customer record doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, CUSTOMER_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing CUSTOMER_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    else if (offset == -1)
    {
        perror("Error while seeking to required customer record!");
        return false;
    }
    lock.l_start = offset;

    int lockingStatus = fcntl(customerFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining read lock on the Customer file!");
        return false;
    }

    readBytes = read(customerFileDescriptor, &account, sizeof(struct Account));
    if (readBytes == -1)
    {
        perror("Error reading customer record from file!");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(customerFileDescriptor, F_SETLK, &lock);

    bzero(writeBuffer, sizeof(writeBuffer));
      if (strcmp(tempCustID, account.login) != 0){
        strcpy(writeBuffer, CUSTOMER_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
     
      }else{
    sprintf(writeBuffer, "Customer Details - \n\tID : %d\n\tName : %s\n\tGender : %c\n\tAge: %d\n\tLoginID : %s", account.accountNumber, account.name, account.gender, account.age,account.login);

    strcat(writeBuffer, "\n\nYou'll now be redirected to the main menu...^");
}
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing customer info to client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
    return true;
}

char* getRole(int role){
    if(role==0) return "Manager";
    return "Employee";
}
int get_last_number_of_loginID(char *input){
    char *token;
    token = strtok(input, "-");
    token = strtok(NULL, "-");
    return atoi(token);
}
#endif