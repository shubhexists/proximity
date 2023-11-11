#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <malloc.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

struct AcceptedSocket
{
    int socketFD;
    struct sockaddr_in clientAddress;
    int error;
    bool acceptedSuccessfully;
};

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

struct sockaddr_in *createIPv4Address(char *ip, int port)
{
    struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    if (strlen(ip) == 0)
        address->sin_addr.s_addr = INADDR_ANY;
    else
        inet_pton(AF_INET, ip, &address->sin_addr.s_addr);
    return address;
}

void recieveAndPrintIncomingData(int socketFD)
{
    char buffer[1024];
    while (true)
    {
        ssize_t amountrecieved = recv(socketFD, buffer, 1024, 0);
        printf("Message from : %d\n", socketFD);
        if (amountrecieved > 0)
        {
            buffer[amountrecieved] = 0;
            printf("%s\n", buffer);
        }
        if (amountrecieved <= 0)
        {
            break;
        }
    }
}

void *threadFunction(void *arg)
{
    int socketFD = *(int *)arg;
    recieveAndPrintIncomingData(socketFD);
    close(socketFD);
    pthread_exit(NULL);
}

void recieveAndPrintIncomingDataOnANewThread(int socketFD)
{
    pthread_t id;
    pthread_create(&id, NULL, threadFunction, &socketFD);
    pthread_detach(id);
}

void startAcceptingIncomingConnection(int socketFD)
{
    while (true)
    {
        struct AcceptedSocket *clientSocket = acceptIncomingConnection(socketFD);
        recieveAndPrintIncomingDataOnANewThread(clientSocket->socketFD);
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

    struct sockaddr_in *serverAddress = createIPv4Address("", 8080);
    printf("Server address: %s\n", inet_ntoa(serverAddress->sin_addr));
    printf("Server port: %d\n", ntohs(serverAddress->sin_port));

    // This tells the OS to bind this process to the given IP address and port number
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

    // This tells the OS to listen for incoming connections
    // The second argument is the backlog, it is the maximum number of connections that can be waiting while the process is handling a particular connection
    // If the queue is full, the client will get an error
    int listenResult = listen(socketFD, 10);

    // This tells the OS to accept the incoming connection
    startAcceptingIncomingConnection(socketFD);
    // Create a buffer to store the data
    shutdown(socketFD, SHUT_RDWR);
    return 0;
}
