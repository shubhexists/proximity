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
    // printf("Listening messages on socket %d\n", socketFD);
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
        if (amountrecieved == 0)
        {
            printf("Server closed the connection\n");
            break;
        }
    }
    close(socketFD);
}

void *startListeningMessagesThread(void *arg)
{
    int socketFD = *((int *)arg);
    startListeningMessagesAndPrintMessages(socketFD);
    pthread_exit(NULL);
}

void startListeningMessagesAndPrintMessagesOnSeparateThread(int socketFD)
{
    int *socketFDPtr = malloc(sizeof(int));
    if (socketFDPtr == NULL)
    {
        perror("Failed to allocate memory");
        return;
    }

    *socketFDPtr = socketFD;
    pthread_t id;
    pthread_create(&id, NULL, startListeningMessagesThread, socketFDPtr);
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

    return 0;
}
