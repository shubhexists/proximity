// OVERALL FUNCTIONING IN THE END OF THE FILE

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <malloc.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

// This function basically creates IpV4 address we provided of the form sockaddr_in
struct sockaddr_in *createIPv4Address(char *ip, int port)
{
    // This line allocates the memory for an instance of sockaddr_in
    struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    // AF_INET -
    address->sin_family = AF_INET;
    // hton()
    address->sin_port = htons(port);
    if (strlen(ip) == 0)
    {
        // THis basically means that if no Ip address is provided, then just assign any IPADDR to it..
        address->sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        //inet_pton 
        inet_pton(AF_INET, ip, &address->sin_addr.s_addr);
    }
    return address;
}

//This function is an infinite loop to listen every message it recieves and print it to the console
void startListeningMessagesAndPrintMessages(int socketFD)
{   
    char buffer[1024];
    while (true)
    {   
        //amount recieved is the number of bytes that is recieved from the remote server
        ssize_t amountrecieved = recv(socketFD, buffer, 1024, 0);
        if (amountrecieved > 0)
        {   
            // This line 
            buffer[amountrecieved] = 0;
            printf("%s", buffer);
        }
        if (amountrecieved == 0)
        {
            //This is recieved if the server closes the connections..
            //YOU WILL NOT BE ABLE TO SEND OR RECIEVE MMESSAGES AFTER THIS
            printf("Server closed the connection\n");
            break;
        }
    }
    //CLOSES THE CONNECTION
    close(socketFD);
}

//This function is make just to be compatible with the requiremnets of the pthread function
// As the pthread function requires the function to take up void arguments and have a return type of void..
void *startListeningMessagesThread(void *arg)
{
    //
    int socketFD = *((int *)arg);
    startListeningMessagesAndPrintMessages(socketFD);
    //End the thread
    pthread_exit(NULL);
}

//This just calls the last function on a seperate thread because it is a blocking function(basically while true will not allow other code behind it to run)
void startListeningMessagesAndPrintMessagesOnSeparateThread(int socketFD)
{
    //Ok this is a tricky one ( had me confused for about a day)
    //Basically we have created a copy of the socketFD parameter, Why not pass the same variable you might ask?
    // It was causing something called the "dangling pointer" error which was passing the wrong FD value to the next function 
    //AS it's value gets changed on the main thread when it would be passed on to the next one...
    int *socketFDPtr = malloc(sizeof(int));
    if (socketFDPtr == NULL)
    {
        perror("Failed to allocate memory");
        return;
    }

    *socketFDPtr = socketFD;
    pthread_t id;
    //Creates a new thread
    pthread_create(&id, NULL, startListeningMessagesThread, socketFDPtr);
}

int main()
{
    //AF_INET - 
    //SOCK_STREAM -
    // 0 - 
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD == -1)
    {
        printf("Socket creation failed\n");
        return -1;
    }

    struct sockaddr_in *serverAddress = createIPv4Address("127.0.0.1", 8080);

    //Asks for a name which we parse at the server side and then we assign this name to the FD...
    printf("Enter your name : ");
    char name[50];
    scanf("%s", name);

    //Connect to the IPAddress assigned
    int result = connect(socketFD, (struct sockaddr *)serverAddress, sizeof(*serverAddress));

    if (result == -1)
    {
        printf("Connection failed\n");
        return -1;
    }
    //This is an ANSI string that tells the program to clear the terminal and start from fresh :)
    printf("\033[2J\033[H");

    //THis is what the server detects while parsing the message...
    char prefix[50] = "/name ";
    char resultName[50 + sizeof(prefix)];
    strcpy(resultName, prefix);
    strcat(resultName, name);
    printf("Welcome %s! It's all your's now :)\n", name);

    //Send the name
    send(socketFD, resultName, strlen(resultName), 0);
    char *line = NULL;
    size_t len = 0;

    //Starts listening messages sent by the server and print it on the terminal..
    startListeningMessagesAndPrintMessagesOnSeparateThread(socketFD);

    while (true)
    {
        //This basically is stdin only... (Take input from the user)
        ssize_t charCount = getline(&line, &len, stdin);
        if (charCount > 0)
        {
            if (strcmp(line, "exit\n") == 0)
            //If the user enters exit, end the connection
                break;
        }
        //Send the message typed
        ssize_t amountwassent = send(socketFD, line, charCount, 0);
        if (amountwassent <= 0)
        {
            //No of bytes can't be negative.. So if it is, It is a Error for sure...
            printf("Error sending message\n");
            break;
        }
    }
    return 0;
}
