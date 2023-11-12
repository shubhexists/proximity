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
    struct sockaddr_in clientAddress;
    int error;
    bool acceptedSuccessfully;
};

int clientSockets[MAX_CLIENTS];

struct AcceptedSocket *acceptIncomingConnection(int socketFD)
{
    struct sockaddr_in clientAddress;
    int clientAddressSize = sizeof(clientAddress);
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

void broadcastMessage(int senderSocketFD, char *message)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clientSockets[i] > 0 && clientSockets[i] != senderSocketFD)
        {
            printf("Sending message to %d\n", clientSockets[i]);
            ssize_t sent_bit = send(clientSockets[i], message, strlen(message), 0);
            if (sent_bit < 0)
            {
                printf("Error sending message to %d\n", clientSockets[i]);
            }
            else
            {
                // just print the size of the sent message
                printf("Sent %ld bytes\n", sent_bit);
                printf("Message sent to %d\n", clientSockets[i]);
            }
        }
    }
}

void *threadFunction(void *arg)
{
    int socketFD = *(int *)arg;
    broadcastMessage(socketFD, "New user joined!\n");
    while (true)
    {
        char buffer[1024];
        ssize_t amountrecieved = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
        if (amountrecieved > 0)
        {
            buffer[amountrecieved] = 0;
            printf("%s\n", buffer);
            broadcastMessage(socketFD, buffer);
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
    broadcastMessage(socketFD, "User disconnected.\n");
    pthread_exit(NULL);
}

void recieveAndPrintIncomingDataOnANewThread(int socketFD)
{
    pthread_t id;
    pthread_create(&id, NULL, threadFunction, &socketFD);
}

void startAcceptingIncomingConnection(int socketFD)
{
    while (true)
    {
        struct AcceptedSocket *clientSocket = acceptIncomingConnection(socketFD);
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clientSockets[i] == 0)
            {
                clientSockets[i] = clientSocket->socketFD;
                recieveAndPrintIncomingDataOnANewThread(clientSocket->socketFD);
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
