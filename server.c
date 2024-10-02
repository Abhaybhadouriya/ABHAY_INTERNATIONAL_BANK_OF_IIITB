#include <stdio.h> // Import for `printf` & `perror` functions
#include <errno.h> // Import for `errno` variable

#include <fcntl.h>      // Import for `fcntl` functions
#include <unistd.h>     // Import for `fork`, `fcntl`, `read`, `write`, `lseek, `_exit` functions
#include <sys/types.h>  // Import for `socket`, `bind`, `listen`, `accept`, `fork`, `lseek` functions
#include <sys/socket.h> // Import for `socket`, `bind`, `listen`, `accept` functions
#include <netinet/ip.h> // Import for `sockaddr_in` stucture

#include <string.h>  // Import for string functions
#include <stdbool.h> // Import for `bool` data type
#include <stdlib.h>  // Import for `atoi` function
#include <sys/shm.h>  // For shared memory
#include "./resource/constantTerms.h"
#include "./resource/commanFun.h"
#include "./recordStruct/account.h"
#include "./recordStruct/loanapply.h"
#include "./recordStruct/structs.h"
#include "./recordStruct/transection.h"
#include "./admin/admin.h"
#include "./customer/customer.h"
#include "./employee/employee.h"
#include "./manager/manager.h"
#include "./recordStruct/employee.h"

void connection_handler(int connFD); // Handles the communication with the client
int *total_clients;  // Shared memory for total client count
void main()
{
    int socketFileDescriptor, socketBindStatus, socketListenStatus, connectionFileDescriptor;
    struct sockaddr_in serverAddress, clientAddress;

  int shmid;
    key_t key = 1234;  // Shared memory key

    // Create shared memory for total client count
    if ((shmid = shmget(key, sizeof(int), 0666 | IPC_CREAT)) < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    total_clients = (int *)shmat(shmid, NULL, 0);
    *total_clients = 0;  // Initialize total clients count


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

    close(socketFileDescriptor);
}

void connection_handler(int connectionFileDescriptor)
{
    printf("Client has connected to the server!\n");

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
                // customer_operation_handler(connectionFileDescriptor);
                break;
             case 3:
                // Customer
                // manager_operation_handler(connectionFileDescriptor);
                break;
             case 4:
                // Customer
                // employee_operation_handler(connectionFileDescriptor);
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
