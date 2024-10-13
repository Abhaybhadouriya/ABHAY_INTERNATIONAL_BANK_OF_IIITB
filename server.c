#include <stdio.h> 
#include <errno.h> 

#include <fcntl.h>      
#include <unistd.h>     
#include <sys/types.h>  
#include <sys/socket.h> 
#include <netinet/ip.h> 

#include <string.h>  
#include <stdbool.h> 
#include <stdlib.h> 
#include <sys/shm.h>  
//================================ RESOURCES ================================
#include "./resource/set.h"
#include "./resource/shFile.h"
#include "./resource/commanFun.h"
#include "./resource/constantTerms.h"
#include "./resource/employeeNeeds.h"
//================================ STRUCT__FUN ================================
#include "./recordStruct/employee.h"
#include "./recordStruct/account.h"
#include "./recordStruct/loanapply.h"
#include "./recordStruct/structs.h"
#include "./recordStruct/transection.h"
#include "./recordStruct/client_data.h"
#include "./recordStruct/customerFeedback.h"
//================================ ADMIN    ==================================
#include "./admin/admin.h"
//================================ CUSTOMER ==================================
#include "./customer/customer.h"
//================================ EMPLOYEE ==================================
#include "./employee/employee.h"
//================================ MANAGER  ==================================
#include "./manager/manager.h"

void connection_handler(int connFD); // Handles the communication with the client
// int *total_clients;  
void main()
{
    int socketFileDescriptor, socketBindStatus, socketListenStatus, connectionFileDescriptor;
    struct sockaddr_in serverAddress, clientAddress;
//  ======================   SHARED MEMORY INITIATIONS START ======================
    
     // Initialize total clients count
    init_shared_memory_total_client();

    // INITIALIZE CURRENT LOGGED IN CLIENT
    // Create shared memory for the set
    init_shared_memorySession_management();
//  ======================   SHARED MEMORY INITIATIONS END   ======================


    socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFileDescriptor == -1)
    {
        perror("Error while creating server socket!");
        _exit(0);
    }

    serverAddress.sin_family = AF_INET;                // IPv4
    serverAddress.sin_port = htons(8081);              // Server will listen to port 8080
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); // Binds the socket to all interfaces

    socketBindStatus = bind(socketFileDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (socketBindStatus == -1)
    {
        perror("Error while binding to server socket!");
        _exit(0);
    }

    socketListenStatus = listen(socketFileDescriptor, 10);
    if (socketListenStatus == -1)
    {
        perror("Error while listening for connections on the server socket!");
        close(socketFileDescriptor);
        _exit(0);
    }

    int clientSize;
    while (1)
    {
        clientSize = (int)sizeof(clientAddress);
        connectionFileDescriptor = accept(socketFileDescriptor, (struct sockaddr *)&clientAddress, &clientSize);
        if (connectionFileDescriptor == -1)
        {
            perror("Error while connecting to client!");
            close(socketFileDescriptor);
        }
        else
        {
            if (!fork())
            {
                // Child will enter this branch
                 (*total_clients)++;
               char message[50];
                sprintf(message, "Total clients: %d\n", *total_clients);
                write(STDOUT_FILENO, message, strlen(message));
                connection_handler(connectionFileDescriptor);
                close(connectionFileDescriptor);
                _exit(0);
            }
        }
    }
    detach_shared_memory();
    close(socketFileDescriptor);
}

void connection_handler(int connectionFileDescriptor)
{
    printf("Client has connected to the server!\n");
 struct clientData clientData;
    char readBuffer[1000], writeBuffer[1000];
    ssize_t readBytes, writeBytes;
    int userChoice;

    writeBytes = write(connectionFileDescriptor, INITIAL_PROMPT, strlen(INITIAL_PROMPT));
    if (writeBytes == -1)
        perror("Error while sending first prompt to the user!");
    else
    {
        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
            perror("Error while reading from client");
        else if (readBytes == 0){
          
            printf("No data was sent by the client ");
            }
        else
        {
            userChoice = atoi(readBuffer);
            switch (userChoice)
            {
            case 1:
                // Admin
                admin_operation_handler(connectionFileDescriptor);
                break;
            case 2:
                // Customer
                customerDriver(connectionFileDescriptor);
                break;
             case 3:
                // Manager
                manager_operation_handler(connectionFileDescriptor);
                break;
             case 4:
                // Employee
                employee_operation_handler(connectionFileDescriptor);
                break;
            default:
                // Exit
                break;
            }
        }
    }
    printf("Terminating connection to client!\n");
    (*total_clients)--;
    char message[50];
    sprintf(message, "Client disconnected. Total clients: %d\n", *total_clients);
    write(STDOUT_FILENO, message, strlen(message));
    }
