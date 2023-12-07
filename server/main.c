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

// Max clients that can be connected to the server
#define MAX_CLIENTS 10

struct AcceptedSocket
{
    int socketFD;
    // User will be asked to enter his name, and it will be stored here
    char name[100];
    // sockaddr_in is a struct that contains the address family, port, and IP address of the client
    // It represents an IPv4 address, for IPv6 use sockaddr_in6 instead
    struct sockaddr_in clientAddress;
    // This is the color that will be used to display the name of the user
    char colour[30];
    int error;
    bool acceptedSuccessfully;
};

char *random_colour()
{
    int random_number = rand() % 7;
    switch (random_number)
    {
    case 0:
        return "\033[0;31m";
    case 1:
        return "\033[0;32m";
    case 2:
        return "\033[0;33m";
    case 3:
        return "\033[0;34m";
    case 4:
        return "\033[0;35m";
    case 5:
        return "\033[0;36m";
    case 6:
        return "\033[0;37m";
    default:
        return "\033[0m";
    }
}

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
    strcpy(acceptedSocket->colour, random_colour());
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
            // printf("Sending message to %d\n", clientSockets[i].socketFD);
            // Append the name of the user to the message like "/Shubham: Hello World!"
            char messageWithUserName[1024];
            // The following line is to add the color to the username
            // This is called ANSI escape code (basically a string that tells the terminal to change the color)
            strcpy(messageWithUserName, acceptedSocket->colour);
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
                printf("Sent %ld bytes\n\n", sent_bit);
                // printf("Message sent to %d\n", clientSockets[i].socketFD);
            }
        }
    }
}

// This function is called when a new thread is created
// The pthread method requires a function that returns void pointer and takes void pointer as an argument
// None of the other ways worked
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
                // Set name to the name entered by the user
                printf("%s Joined!\n", acceptedSocket->name);
                // TODO - Randomize this entry message just like discord
                broadcastMessage(acceptedSocket, "hopped in!\n");
                // printf("%s\n", buffer + 6);
                continue;
            }
            else
            {
                // If the message is not a name change request, then we just print the message
                printf("%s", buffer);
            }
            // Send the message to all the other clients in the connection
            broadcastMessage(acceptedSocket, buffer);
        }
        else if (amountrecieved == 0)
        {
            // If the amount recieved is 0, then the client has closed the connection
            // TODO - Remove this user from the clientSockets array
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
    // Closing the socket connection
    close(socketFD);
    broadcastMessage(acceptedSocket, "User disconnected.\n");
    // Closing the thread
    pthread_exit(NULL);
}

// Just a function to add a new thread..
void recieveAndPrintIncomingDataOnANewThread(struct AcceptedSocket *acceptedSocket)
{
    pthread_t id;
    pthread_create(&id, NULL, threadFunction, acceptedSocket);
}

// This functions makes an object of struct AcceptedSocket and acceptsTheConnection
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
                // printf("Accepted connection from %d\n", clientSocket->socketFD);
                recieveAndPrintIncomingDataOnANewThread(clientSocket);
                break;
            }
        }
    }
}

int main()
{
    // AF_INET - It refers to the IPV4 address, AF_INET6 refers to IPV6 address
    // SOCK_STREAM - It refers to the TCP connection, SOCK_DGRAM refers to UDP connection
    // 0 - It refers to the protocol, 0 means that the protocol is chosen automatically
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD == -1)
    {
        printf("Socket creation failed\n");
        return -1;
    }

    // sockaddr_in is a struct that contains the address family, port, and IP address of the server
    // It represents an IPv4 address, for IPv6 use sockaddr_in6 instead
    struct sockaddr_in *serverAddress = malloc(sizeof(struct sockaddr_in));
    serverAddress->sin_family = AF_INET;
    // htons() basically converts the port number from host byte order to network byte order
    // This is required because different systems have different byte orders
    serverAddress->sin_port = htons(8080);
    // INADDR_ANY means that this server can accept inputs from any IP Address
    serverAddress->sin_addr.s_addr = INADDR_ANY;

    // Bind function binds the code to the port on our system we asked
    // It fails if we provided a wrong port or an already used port
    // It's 2nd parameter required it to be of type sockaddr and not sockaddr_in, so I just typecasted it.
    int result = bind(socketFD, (struct sockaddr *)serverAddress, sizeof(*serverAddress));
    if (result == 0)
    {
        // printf("Bind successful\n");
    }
    else
    {
        // printf("Bind failed\n");
        return -1;
    }

    // 10 here is the maximum number of entries that this socket would listen to.....
    int listenResult = listen(socketFD, 10);

    startAcceptingIncomingConnection(socketFD);
    // This is called when the server is shut down and we have to free up the port we have occupied
    shutdown(socketFD, SHUT_RDWR);
    return 0;
}

/*
 Overall Function of the server -
    1. Create a socket
    2. Bind the socket to a port
    3. Listen to the socket
    4. Accept the incoming connection
    5. Create a new thread for the every connection that is accepted
    6. Recieve and print the incoming data on the new thread
    7. Repeat from step 4
*/