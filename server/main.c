// OVERALL FUNCTIONING IN THE END OF THE FILE

#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10

struct AcceptedSocket
{
    int socketFD;
    // User will be asked to enter his name, and it will be stored here
    char name[100];
    // sockaddr_in is a struct that contains the address family, port, and IP address of the client
    // It represents an IPv4 address, for IPv6 use sockaddr_in6 instead
    struct sockaddr_in clientAddress;
    int error;
    bool acceptedSuccessfully;
};

// const char* getRandomColor() {
//     const char* colors[] = {"\033[1;31m", "\033[1;32m", "\033[1;33m", "\033[1;34m", "\033[1;35m", "\033[1;36m"};
//     int numColors = sizeof(colors) / sizeof(colors[0]);
//     int randomIndex = rand() % numColors;
//     return colors[randomIndex];
// }

// A global variable to store currently accepted clients..
struct AcceptedSocket clientSockets[MAX_CLIENTS];

struct AcceptedSocket *acceptIncomingConnection(int socketFD)
{
    struct sockaddr_in clientAddress;
    int clientAddressSize = sizeof(clientAddress);
    // accept() blocks the program until a client connects to the server
    // Also sockaddr is a generic struct that can represent both IPv4 and IPv6 addresses
    // and accept required the 2nd param to be sockaddr, (Hence did the type casting)
    int clientSocketFD = accept(socketFD, (struct sockaddr *)&clientAddress, &clientAddressSize);
    struct AcceptedSocket *acceptedSocket = malloc(sizeof(struct AcceptedSocket));
    acceptedSocket->socketFD = clientSocketFD;
    acceptedSocket->clientAddress = clientAddress;
    acceptedSocket->acceptedSuccessfully = clientSocketFD > 0;
    if (!acceptedSocket->acceptedSuccessfully)
    {
        acceptedSocket->error = clientSocketFD;
    }
    return acceptedSocket;
}

// This function sends message recieced from one client to all the other clients in the connection
void broadcastMessage(struct AcceptedSocket *acceptedSocket, char *message)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clientSockets[i].socketFD > 0 && clientSockets[i].socketFD != acceptedSocket->socketFD)
        {
            printf("Sending message to %d\n", clientSockets[i].socketFD);
            // Append the name of the user to the message like "/Shubham: Hello World!"
            char messageWithUserName[1024];
            // strcpy(messageWithUserName, "/");
            strcpy(messageWithUserName, "\033[1;31m");
            strcat(messageWithUserName, acceptedSocket->name);
            strcat(messageWithUserName, " : ");
            strcat(messageWithUserName, "\033[0m");
            strcat(messageWithUserName, message);
            // send_bit is basically the no of bytes of information that is transferred.
            ssize_t sent_bit = send(clientSockets[i].socketFD, messageWithUserName, strlen(messageWithUserName), 0);
            if (sent_bit < 0) // As number of bytes can never be less than 0,
            {
                printf("Error sending message to %d\n", clientSockets[i].socketFD);
            }
            else
            {
                printf("Sent %ld bytes\n", sent_bit);
                printf("Message sent to %d\n", clientSockets[i].socketFD);
            }
        }
    }
}

void *threadFunction(void *arg)
{
    // arg is a void pointer, so we need to typecast it to int pointer
    struct AcceptedSocket *acceptedSocket = (struct AcceptedSocket *)arg;
    int socketFD = acceptedSocket->socketFD;

    while (true)
    {
        char buffer[1024];
        ssize_t amountrecieved = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
        if (amountrecieved > 0)
        {
            buffer[amountrecieved] = 0;
            // If the start of the message is /name, then the user is trying to set his name
            if (strncmp(buffer, "/name", 5) == 0)
            {
                // We are replacing the \n character with \0, so that the string ends there
                buffer[strcspn(buffer, "\n")] = '\0';
                // We are copying the name from the buffer to the name field of the acceptedSocket
                // We are copying from the 6th character, because the first 5 characters are /name
                strcpy(acceptedSocket->name, buffer + 6);
                printf("Name set to %s\n", acceptedSocket->name);
                broadcastMessage(acceptedSocket, "hopped in!\n");
                printf("%s\n", buffer + 6);
                continue;
            }
            else
            {
                printf("%s\n", buffer);
            }
            broadcastMessage(acceptedSocket, buffer);
        }
        else if (amountrecieved == 0)
        {
            printf("Client closed the connection\n");
            break;
        }
        else
        {
            perror("Error receiving data");
            printf("Connection closed\n");
            break;
        }
    }
    close(socketFD);
    broadcastMessage(acceptedSocket, "User disconnected.\n");
    pthread_exit(NULL);
}

void recieveAndPrintIncomingDataOnANewThread(struct AcceptedSocket *acceptedSocket)
{
    pthread_t id;
    pthread_create(&id, NULL, threadFunction, acceptedSocket);
}

void startAcceptingIncomingConnection(int socketFD)
{
    while (true)
    {
        struct AcceptedSocket *clientSocket = acceptIncomingConnection(socketFD);
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clientSockets[i].socketFD == 0)
            {
                clientSockets[i] = *clientSocket;
                printf("Accepted connection from %d\n", clientSocket->socketFD);
                recieveAndPrintIncomingDataOnANewThread(clientSocket);
                break;
            }
        }
    }
}

int main()
{
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD == -1)
    {
        printf("Socket creation failed\n");
        return -1;
    }

    struct sockaddr_in *serverAddress = malloc(sizeof(struct sockaddr_in));
    serverAddress->sin_family = AF_INET;
    serverAddress->sin_port = htons(8080);
    serverAddress->sin_addr.s_addr = INADDR_ANY;

    int result = bind(socketFD, (struct sockaddr *)serverAddress, sizeof(*serverAddress));
    if (result == 0)
    {
        printf("Bind successful\n");
    }
    else
    {
        printf("Bind failed\n");
        return -1;
    }

    int listenResult = listen(socketFD, 10);

    startAcceptingIncomingConnection(socketFD);
    shutdown(socketFD, SHUT_RDWR);
    return 0;
}
