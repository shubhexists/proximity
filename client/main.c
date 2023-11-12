// #include <stdio.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <string.h>
// #include <malloc.h>
// #include <arpa/inet.h>
// #include <stdbool.h>
// #include <unistd.h>
// #include <pthread.h>

// struct sockaddr_in *createIPv4Address(char *ip, int port)
// {
//     struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
//     address->sin_family = AF_INET;
//     address->sin_port = htons(port);
//     if (strlen(ip) == 0)
//         address->sin_addr.s_addr = INADDR_ANY;
//     else
//         inet_pton(AF_INET, ip, &address->sin_addr.s_addr);
//     return address;
// }

// void startListeningMessagesAndPrintMessages(int socketFD)
// {
//     char buffer[1024];
//     while (true)
//     {
//         ssize_t amountrecieved = recv(socketFD, buffer, 1024, 0);
//         if (amountrecieved > 0)
//         {
//             buffer[amountrecieved] = 0;
//             printf("%s\n", buffer);
//         }
//         if (amountrecieved <= 0)
//         {
//             break;
//         }
//     }
//     close(socketFD);
// }

// void *startListeningMessagesThread(void *arg)
// {
//     int socketFD = *(int *)arg;
//     startListeningMessagesAndPrintMessages(socketFD);
//     pthread_exit(NULL);
// }

// void startListeningMessagesAndPrintMessagesOnSeparateThread(int socketFD){
//     pthread_t id;
//     pthread_create(&id,NULL,startListeningMessagesThread,&socketFD);
// }

// int main()
// {
//     // Socket function inside sys/socket.h
//     // AF_INET is the address family for IPv4, For IPv6 use AF_INET6
//     // SOCK_STREAM is the type of socket, SOCK_STREAM is for TCP, SOCK_DGRAM is for UDP
//     // 0 is the protocol, 0 means use default protocol for the given address family and socket type
//     int socketFD = socket(AF_INET, SOCK_STREAM, 0);
//     // socket function returns -1 on failure
//     if (socketFD == -1)
//     {
//         printf("Socket creation failed\n");
//         return -1;
//     }

//     // struct sockaddr_in is the structure for IPv4 addresses
//     // struct sockaddr_in6 is the structure for IPv6 addresses
//     // struct sockaddr is the generic structure for all addresses but since we used AF_INET, we use sockaddr_in
//     struct sockaddr_in *serverAddress = createIPv4Address("127.0.0.1", 8080);

//     // CHECK THIS otherwise we can also use inet_pton() function to convert IP address from string to binary
//     int result = connect(socketFD, (struct sockaddr *)serverAddress, sizeof(*serverAddress));

//     if (result == -1)
//     {
//         printf("Connection failed\n");
//         return -1;
//     }
//     else
//     {
//         printf("Connection successful\n");
//     }

//     char *line = NULL;
//     size_t len = 0;

//     startListeningMessagesAndPrintMessagesOnSeparateThread(socketFD);

//     while (true)
//     {
//         ssize_t charCount = getline(&line, &len, stdin);
//         if (charCount > 0)
//         {
//             if (strcmp(line, "exit\n") == 0)
//                 break;
//         }
//         ssize_t amountwassent = send(socketFD, line, charCount, 0);
//     }

//     close(socketFD);
//     // //Create a buffer to store the data
//     // //What a buffer is? A buffer is a temporary storage area in memory that holds the data while it is being transferred from one place to another
//     // char* message;
//     // //Send data to the server
//     // // \r is carriage return, \n is line feed (Even I don't know what it is lol)
//     // message = "GET / HTTP/1.1\r\nHost: www.google.com\r\n\r\n";
//     // send(socketFD, message, strlen(message), 0);

//     // char buffer[1024];
//     // //Receive data from the server
//     // //Bruh, whoever wrote these functions is a f***ing god :D
//     // recv(socketFD, buffer, 1024, 0);

//     // printf("%s\n", buffer);
//     return 0;
// }

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <malloc.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

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
void startListeningMessagesAndPrintMessages(int socketFD)
{
    printf("Listening messages on socket %d\n", socketFD);
    char buffer[1024];
    while (true)
    {
        ssize_t amountrecieved = recv(socketFD, buffer, 1024, 0);
        printf("Message from : %d\n", socketFD);
        if (amountrecieved > 0)
        {
            buffer[amountrecieved] = 0;
            printf("Received: %s\n", buffer);
        }
        else if (amountrecieved == 0)
        {
            printf("Server closed the connection\n");
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
}

void *startListeningMessagesThread(void *arg)
{
    int socketFD = *(int *)arg;
    startListeningMessagesAndPrintMessages(socketFD);
    pthread_exit(NULL);
}

void startListeningMessagesAndPrintMessagesOnSeparateThread(int socketFD)
{
    pthread_t id;
    pthread_create(&id, NULL, startListeningMessagesThread, &socketFD);
}

int main()
{
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD == -1)
    {
        printf("Socket creation failed\n");
        return -1;
    }

    struct sockaddr_in *serverAddress = createIPv4Address("127.0.0.1", 8080);

    int result = connect(socketFD, (struct sockaddr *)serverAddress, sizeof(*serverAddress));

    if (result == -1)
    {
        printf("Connection failed\n");
        return -1;
    }
    else
    {
        printf("Connection successful\n");
    }

    char *line = NULL;
    size_t len = 0;

    startListeningMessagesAndPrintMessagesOnSeparateThread(socketFD);

    while (true)
    {
        ssize_t charCount = getline(&line, &len, stdin);
        if (charCount > 0)
        {
            if (strcmp(line, "exit\n") == 0)
                break;
        }
        ssize_t amountwassent = send(socketFD, line, charCount, 0);
        if (amountwassent <= 0)
        {
            printf("Error sending message\n");
            break;
        }
    }

    close(socketFD);
    return 0;
}
