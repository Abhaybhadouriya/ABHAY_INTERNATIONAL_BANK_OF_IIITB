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
#include "../recordStruct/client_data.h"

// Function Prototypes =================================

bool login_handler(bool isAdmin, int connFD, struct Account *ptrToCustomer,struct clientData *clientData);
bool get_account_details(int connFD, struct Account *customerAccount);
bool get_customer_details(int connFD, int customerID);
bool get_transaction_details(int connFD, int accountNumber);
char* getRole(int role);
int get_last_number_of_loginID(char *input);
bool view_employee_account(int connFD,int role,int range,char *str);
bool updateDetails(int connFD,bool isAdmin);
bool logout(int connFD,char *str);
bool delete_account(int connFD,bool isAdmin);

// ==========================================2===========

// Function Definition =================================

bool logout(int connFD,char *str){
    return remove_from_shared_set(str);
}


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
bool login_handler(bool isAdmin, int connFD, struct Account *ptrToCustomerID,struct clientData *clientData)
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
            if (strcmp(hashedPassword, ADMIN_PASS_WORD) == 0){
                 if(!add_to_shared_set(ADMIN_USER_NAME))
               {
                bzero(writeBuffer, sizeof(writeBuffer));
                writeBytes = write(connFD,CUSTOMER_ALREADY_LOGGED_IN, strlen(CUSTOMER_ALREADY_LOGGED_IN));
                return false;
               }
               else{ 
                   strcpy((*clientData).name ,ADMIN_USER_NAME);
                strcpy((*clientData).username,ADMIN_USER_NAME);
               return true;
               }
                }
        }
        else
        {
            if (strcmp(hashedPassword, account.password) == 0)
            {


              if(!account.active){
                  bzero(writeBuffer, sizeof(writeBuffer));
                write(connFD,"Account is currently Not Active Please contact you Bank!$", strlen("Account is currently Not Active Please contact you Bank!$"));
                 return false;
               }
               if(!add_to_shared_set(account.login))
               {
                bzero(writeBuffer, sizeof(writeBuffer));
                writeBytes = write(connFD,CUSTOMER_ALREADY_LOGGED_IN, strlen(CUSTOMER_ALREADY_LOGGED_IN));
                return false;
               }
                strcpy((*clientData).name ,account.name);
                strcpy((*clientData).username,account.login);
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


bool updateDetails(int connFD,bool isAdmin)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];
    char *FILE_TO_FETCH;
    struct Account account;
    struct Employee employee;
    int role;
    int customerID;
    char targetCusID[100];
    if(isAdmin){
        writeBytes = write(connFD, ADMIN_MODIFY_PROMPT, strlen(ADMIN_MODIFY_PROMPT));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_MODIFY_PROMT message to client!");
            return false;
        }
        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting TYPE from client!");
            return false;
        }


        role = atoi(readBuffer);
         writeBytes = write(connFD, ADMIN_GET_UNIQUE_ID, strlen(ADMIN_GET_UNIQUE_ID));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_GET_UNIQUE_ID to client!");
            return false;
        }
        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while reading UNIQUE ID from client!");
            return false;
        }
        strcpy(targetCusID, readBuffer);
        customerID=get_last_number_of_loginID(readBuffer);


    }


    off_t offset;
    int lockingStatus;

    int customerFileDescriptor = open(role==1?ACCOUNT_FILE:EMPLOYEE_FILE, O_RDONLY);
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
        // readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    
    offset = role==1?lseek(customerFileDescriptor, customerID * sizeof(struct Employee), SEEK_SET):lseek(customerFileDescriptor, customerID * sizeof(struct Employee), SEEK_SET);
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
        // readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    else if (offset == -1)
    {
        perror("Error while seeking to required customer record!");
        return false;
    }

    struct flock lock = {F_RDLCK, SEEK_SET, offset, role==1?sizeof(struct Account):sizeof(struct Employee), getpid()};
    lockingStatus = fcntl(customerFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Couldn't obtain lock on customer record!");
        return false;
    }
    
    readBytes = role==1?read(customerFileDescriptor, &account, sizeof(struct Account)):read(customerFileDescriptor, &employee, sizeof(struct Employee));
    
    if (readBytes == -1)
    {
        perror("Error while reading customer record from the file!");
        return false;
    }

    // Unlock the record
    lock.l_type = F_UNLCK;
    fcntl(customerFileDescriptor, F_SETLK, &lock);

    close(customerFileDescriptor);

    writeBytes = write(connFD, role==1?ADMIN_MOD_CUSTOMER_MENU:ADMIN_MOD_EMPLOYEE_MENU, strlen(role==1?ADMIN_MOD_CUSTOMER_MENU:ADMIN_MOD_EMPLOYEE_MENU));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_MOD_CUSTOMER_MENU message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error while getting customer modification menu choice from client!");
        return false;
    }

    int choice = atoi(readBuffer);
    if (choice == 0)
    { // A non-numeric string was passed to atoi
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, ERRON_INPUT_FOR_NUMBER);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing ERRON_INPUT_FOR_NUMBER message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    switch (choice)
    {
    case 1:
        writeBytes = write(connFD, ADMIN_MOD_CUSTOMER_NEW_NAME, strlen(ADMIN_MOD_CUSTOMER_NEW_NAME));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_MOD_CUSTOMER_NEW_NAME message to client!");
            return false;
        }
      
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for customer's new name from client!");
            return false;
        }
        if(role==1){
        strcpy(account.name, readBuffer);
        }else{
        strcpy(employee.name ,readBuffer);
        }
        break;
    case 2:
        writeBytes = write(connFD, ADMIN_MOD_CUSTOMER_NEW_AGE, strlen(ADMIN_MOD_CUSTOMER_NEW_AGE));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_MOD_CUSTOMER_NEW_AGE message to client!");
            return false;
        }
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for customer's new age from client!");
            return false;
        }
        int updatedAge = atoi(readBuffer);
        if (updatedAge == 0)
        {
            // Either client has sent age as 0 (which is invalid) or has entered a non-numeric string
            bzero(writeBuffer, sizeof(writeBuffer));
            strcpy(writeBuffer, ERRON_INPUT_FOR_NUMBER);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing ERRON_INPUT_FOR_NUMBER message to client!");
                return false;
            }
            readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
            return false;
        }
        if(role==1){
        account.age= updatedAge;
        }else{
        employee.age =updatedAge;
        }
        break;
    case 3:
        writeBytes = write(connFD, ADMIN_MOD_CUSTOMER_NEW_GENDER, strlen(ADMIN_MOD_CUSTOMER_NEW_GENDER));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_MOD_CUSTOMER_NEW_GENDER message to client!");
            return false;
        }
        
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for customer's new gender from client!");
            return false;
        }
        if(role==1){
        account.gender=readBuffer[0];
        }else{
        employee.gender=readBuffer[0];
        }
        
        break;
    case 4:
        writeBytes = write(connFD, ADMIN_ADD__EMPLOYEE_ROLE, strlen(ADMIN_ADD__EMPLOYEE_ROLE));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_MOD_CUSTOMER_NEW_GENDER message to client!");
            return false;
        }
        
        readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting response for customer's new gender from client!");
            return false;
        }
         int updatedRole = atoi(readBuffer);
        if (updatedRole == 0)
        {
            // Either client has sent age as 0 (which is invalid) or has entered a non-numeric string
            bzero(writeBuffer, sizeof(writeBuffer));
            strcpy(writeBuffer, "InValid Role");
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1)
            {
                perror("Error while writing ERRON_INPUT_FOR_NUMBER message to client!");
                return false;
            }
            readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
            return false;
        }
        employee.role =updatedRole-1;
        break;
    default:
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, INVALID_MENU_CHOICE);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing INVALID_MENU_CHOICE message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }

    customerFileDescriptor = open(role==1?ACCOUNT_FILE:EMPLOYEE_FILE, O_WRONLY);
    if (customerFileDescriptor == -1)
    {
        perror("Error while opening customer file");
        return false;
    }
    offset = lseek(customerFileDescriptor, customerID*(role==1?sizeof(struct Account):sizeof(struct Employee)), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to required customer record!");
        return false;
    }

    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    lockingStatus = fcntl(customerFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error while obtaining write lock on customer record!");
        return false;
    }

    writeBytes = role==1?write(customerFileDescriptor,  &account, sizeof(struct Account)):write(customerFileDescriptor, &employee, sizeof(struct Employee));
    if (writeBytes == -1)
    {
        perror("Error while writing update customer info into file");
    }

    lock.l_type = F_UNLCK;
    fcntl(customerFileDescriptor, F_SETLKW, &lock);

    close(customerFileDescriptor);

    writeBytes = write(connFD, ADMIN_MOD_CUSTOMER_SUCCESS, strlen(ADMIN_MOD_CUSTOMER_SUCCESS));
    if (writeBytes == -1)
    {
        perror("Error while writing ADMIN_MOD_CUSTOMER_SUCCESS message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read

    return true;
}
bool delete_account(int connFD,bool isAdmin)
{
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    struct Account account;
    struct Employee employee;
    int role;
    int customerID;
    char targetCusID[1000];
    if(isAdmin){
        writeBytes = write(connFD, ADMIN_WHO_TO_DEACT_ACT, strlen(ADMIN_WHO_TO_DEACT_ACT));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_MODIFY_PROMT message to client!");
            return false;
        }
        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting TYPE from client!");
            return false;
        }

        int temp = atoi(readBuffer); 
        if(temp==1){
            role =1;
        }else if(temp==2||temp==3){
            role =2;
        }else{
            writeBytes = write(connFD, "Invalid Input", strlen("Invalid Input"));
            return true;

        }
         writeBytes = write(connFD, ADMIN_GET_UNIQUE_ID, strlen(ADMIN_GET_UNIQUE_ID));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_GET_UNIQUE_ID to client!");
            return false;
        }
        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while reading UNIQUE ID from client!");
            return false;
        }
        strcpy(targetCusID, readBuffer);
        customerID=get_last_number_of_loginID(readBuffer);


    }


    // writeBytes = write(connFD, ADMIN_DEL_ACCOUNT_NO, strlen(ADMIN_DEL_ACCOUNT_NO));
    // if (writeBytes == -1)
    // {
    //     perror("Error writing ADMIN_DEL_ACCOUNT_NO to client!");
    //     return false;
    // }

    // bzero(readBuffer, sizeof(readBuffer));
    // readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    // if (readBytes == -1)
    // {
    //     perror("Error reading account number response from the client!");
    //     return false;
    // }

    // int accountNumber = atoi(readBuffer);

    int accountFileDescriptor = open(role==1?ACCOUNT_FILE:EMPLOYEE_FILE, O_RDONLY);
    if (accountFileDescriptor == -1)
    {
        // Account record doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, ACCOUNT_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing ACCOUNT_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }


    int offset = lseek(accountFileDescriptor, customerID*( role==1?sizeof(struct Account):sizeof(struct Employee)), SEEK_SET);
    if (errno == EINVAL)
    {
        // Customer record doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, ACCOUNT_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1)
        {
            perror("Error while writing ACCOUNT_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer)); // Dummy read
        return false;
    }
    else if (offset == -1)
    {
        perror("Error while seeking to required account record!");
        return false;
    }

    struct flock lock = {F_RDLCK, SEEK_SET, offset, (role==1?sizeof(struct Account):sizeof(struct Employee)), getpid()};
    int lockingStatus = fcntl(accountFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1)
    {
        perror("Error obtaining read lock on Account record!");
        return false;
    }
    if(role==1)readBytes = read(accountFileDescriptor, &account, sizeof(struct Account));
    else readBytes = read(accountFileDescriptor, &employee, sizeof(struct Employee));
    if (readBytes == -1)
    {
        perror("Error while reading Account record from file!");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(accountFileDescriptor, F_SETLK, &lock);

    close(accountFileDescriptor);

    bzero(writeBuffer, sizeof(writeBuffer));
    if (role==1&&account.balance == 0|| role==2)
    {   
        bool status = role==1?account.active:employee.active;
        writeBytes = write(connFD, status?ADMIN_DEACT_NOTIFY:ADMIN_ACT_NOTIFY, strlen(status?ADMIN_DEACT_NOTIFY:ADMIN_ACT_NOTIFY));
        if (writeBytes == -1)
        {
            perror("Error while writing ADMIN_MODIFY_PROMT message to client!");
            return false;
        }
        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error while getting TYPE from client!");
            return false;
        }
        int temp = atoi(readBuffer); 
        if(temp>1){ 
            
            write(connFD, "Operation Cancelled Redirecting to Main Menu^", strlen("Operation Cancelled Redirecting to Main Menu^"));
            return true;
        }
        // No money, hence can close account
        if(role==1) account.active = !account.active;
        else employee.active = !employee.active;

        accountFileDescriptor = open(role==1?ACCOUNT_FILE:EMPLOYEE_FILE, O_WRONLY);
        if (accountFileDescriptor == -1)
        {
            perror("Error opening Account file in write mode!");
            return false;
        }

        offset = lseek(accountFileDescriptor, customerID*( role==1?sizeof(struct Account):sizeof(struct Employee)), SEEK_SET);
        if (offset == -1)
        {
            perror("Error seeking to the Account!");
            return false;
        }

        lock.l_type = F_WRLCK;
        lock.l_start = offset;

        int lockingStatus = fcntl(accountFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error obtaining write lock on the Account file!");
            return false;
        }
         if(role==1)  writeBytes = write(accountFileDescriptor, &account, sizeof(struct Account));
        else   writeBytes = write(accountFileDescriptor, &employee, sizeof(struct Employee));
        if (writeBytes == -1)
        {
            perror("Error deleting account record!");
            return false;
        }

        lock.l_type = F_UNLCK;
        fcntl(accountFileDescriptor, F_SETLK, &lock);

        strcpy(writeBuffer, ADMIN_DEL_ACCOUNT_SUCCESS);
    }
    else
        // Account has some money ask customer to withdraw it
        strcpy(writeBuffer, ADMIN_DEL_ACCOUNT_FAILURE);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error while writing final DEL message to client!");
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