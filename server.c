//compile with command g++ server.c -o server -lpthread
//run with:  ./server IP port
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h> 

#define MAX_CLIENTS 100

// Client structure
typedef struct {
    int socket;
    struct sockaddr_in address; 
    int port;
} Client;

Client clients[MAX_CLIENTS];  // Array to store client information
struct sockaddr_in client_addresses[MAX_CLIENTS];  // Array to store client addresses
int client_count = 0;  // Count of connected clients
pthread_mutex_t client_count_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex for client count

// Function to send a message to all clients
void send_all(char *message) {
    for (int i = 0; i < client_count; i++) {
        write(clients[i].socket, message, strlen(message));
    }
}

// Function to print all connected clients
void print_clients() {
    printf("Connected clients:\n");
    for (int i = 0; i < client_count; i++) {
        printf("%d- %s:%d\n", i, inet_ntoa(clients[i].address.sin_addr), clients[i].port);
    }
}

// Function to handle client communication
void* handle_client(void* arg) {
    int client_socket = *(int*)arg;//client socket
    char buffer[256];//buffer to store the message
    char response[256];//buffer to store the response
    int received_bytes;//number of bytes received from client
    struct sockaddr_in client_address; //client address
    socklen_t client_address_len = sizeof(client_address); //client address length
    if (getpeername(client_socket, (struct sockaddr*)&client_address, &client_address_len) == -1) {
        perror("getpeername");
    }

    int flag=0;
    while (1) {
        // Receive the response from the client
        while ((received_bytes = recv(client_socket, response, sizeof(response), 0)) > 0) { //receive the message from the client
            if (flag==0) {
                printf("Receiving messages from client %s:%d:\n", 
                inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port)); //print the client address
                flag++;
            }
            if (strncmp("EOM", response, 3) == 0) {
                printf("End of Message signal received.\n");
                flag=0;
                break;  // Break the loop if "EOM" is received
            }
            printf("%s", response);
            memset(&response, 0, sizeof(response));
        }
        // Check if the client has disconnected
        if (received_bytes == 0) {
        printf("Client %s:%d disconnected.\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port)); 
        pthread_mutex_lock(&client_count_mutex); //lock the mutex to update the client count
        for (int i = 0; i < client_count; i++) {
            if (clients[i].socket == client_socket) {
                for (int j = i; j < client_count - 1; j++) {
                    clients[j] = clients[j + 1];
                }
                client_count--;
                break;
            }
        }
        pthread_mutex_unlock(&client_count_mutex); //unlock the mutex after updating the client count
        close(client_socket);
        return NULL;
        }
        if (received_bytes < 0) {
            perror("Error reading from socket");
            break;
        }
    }

    // Close the connection
    close(client_socket);
    return NULL;
}

// Function to handle server commands
void* handle_commands(void* arg) {
    char buffer[256]; // Buffer to store the command

    while (1) {
        memset(&buffer, 0, sizeof(buffer));
        fgets(buffer, sizeof(buffer), stdin);
        strtok(buffer, "\n");
        if (buffer[0] == '\0') {
            continue;  // Skip this iteration if the buffer is empty
        }

        // Check for specific commands
        if (strncmp(buffer, "sendall ", 8) == 0) {
            // Send a message to all clients
            send_all(buffer + 8);
        } else if (strcmp(buffer, "print clients") == 0) {
            // Print the list of connected clients
            print_clients();
        } else if (strncmp(buffer, "send ", 5) == 0) {
            // Send a message to a specific client
            char* space = strchr(buffer + 5, ' ');
            if (space != NULL) {
                *space = '\0';
                int client_index = atoi(buffer + 5); // Get the client index
                if (client_index >= 0 && client_index < client_count) { // Check if the client index is valid
                    write(clients[client_index].socket, space + 1, strlen(space + 1)); // Send the message to the client
                } else {
                    printf("Invalid client index\n");
                }
            } else {
                printf("Invalid command format. Use: send [client index] [message]\n");
            }
        } else if (strncmp(buffer, "cd ", 3) == 0) {
            // Send a 'cd' command to a specific client
            char* space = strchr(buffer + 3, ' ');
            if (space != NULL) {
                *space = '\0';
                int client_index = atoi(buffer + 3); // Get the client index
                if (client_index >= 0 && client_index < client_count) { // Check if the client index is valid
                    write(clients[client_index].socket, buffer, strlen(buffer)); // Send the 'cd' command to the client
                } else {
                    printf("Invalid client index\n");
                }
            } else {
                printf("Invalid command format. Use: cd [client index] [directory]\n");
            }
        } else {
            printf("Unknown command\n");
        }
    }

    return NULL;
}

int main(int argc, char const *argv[]) {
    int sock;
    struct sockaddr_in server_address; // Server address structure
    socklen_t client_length; // Client address length

    // Check if the correct number of arguments are provided
    if(argc < 3){
       printf("[+] Usage: \n");
       printf("[+] Example: ./server 192.168.0.100 9001 \n");
       return 1;
    }

    // Create a socket for communication
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error creating socket");
        return 1;
    }
    // Allow reuse of local addresses
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) { //set the socket options
        perror("Error setting socket options");
        return 1;
    }
    // Set up the server address structure
    server_address.sin_family = AF_INET; // Set the address family
    server_address.sin_addr.s_addr = inet_addr(argv[1]); // Set the IP address
    server_address.sin_port = htons(atoi(argv[2])); // Set the port number

    // Bind the socket
    if (bind(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) { //bind the socket to the server address
        perror("Error binding socket");
        return 1;
    }

    // Listen for connections
    if (listen(sock, 5) < 0){
        perror("Error listening on socket");
        return 1;
    }

    printf("Server started. Listening on port %s\n", argv[2]);

    printf("Available commands:\n");
    printf("sendall [message] - Send a message to all clients\n");
    printf("print clients - Print the list of connected clients\n");
    printf("send [client index] [message] - Send a message to a specific client\n");

    pthread_t command_thread; //command thread
    pthread_create(&command_thread, NULL, handle_commands, NULL); //create a thread to handle the commands

    while (1) {
        // Accept a new client
        client_length = sizeof(client_addresses[client_count]); //get the client address length
        int client_socket = accept(sock, (struct sockaddr *)&client_addresses[client_count], &client_length); //accept the client
        if (client_socket < 0) {
            perror("Error accepting client");
            continue;
        }

        printf("Client connected from %s:%d\n", inet_ntoa(client_addresses[client_count].sin_addr),
         ntohs(client_addresses[client_count].sin_port)); //print the client address
        pthread_mutex_lock(&client_count_mutex); //lock the mutex to update the client count
        if (client_count < MAX_CLIENTS) {
            clients[client_count].socket = client_socket; 
            clients[client_count].address = client_addresses[client_count];  // Store the address of the client
            clients[client_count].port = ntohs(client_addresses[client_count].sin_port); // Store the port of the client
            client_count++;
        }
        pthread_mutex_unlock(&client_count_mutex); //unlock the mutex

        // Create a new thread to handle the client
        pthread_t client_thread;
        pthread_create(&client_thread, NULL, handle_client, &client_socket); //create a thread to handle the client
    }

    return 0; 
}