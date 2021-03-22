#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>

#define BUFFER_SIZE 4096
#define PORT 2233
#define BACKLOG 10
#define FILE_NAME "download.zip"

int ServerSocket;

void *HandleDownload(void *clientSocket);
int main()
{
    ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(ServerSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        printf("Failed to bind socket! Possible reason: Socket is already in use by other proccess. \n");
        return -1;
    }

    // Telling the kernel we are listening for new connections with the specified amount of backlog.
    listen(ServerSocket, BACKLOG);

    int clientSocket;
    while (1)
    {
        // Accepting new connections
        clientSocket = accept(ServerSocket, NULL, NULL);

        pthread_t downloadThread;
        int *ptrClientSocket = malloc(sizeof(int));
        *ptrClientSocket = clientSocket;

        pthread_create(&downloadThread, NULL, HandleDownload, ptrClientSocket);
    }

    //After application exit
    printf("Program exited succesfully \n");
    close(ServerSocket);
}

void *HandleDownload(void *ptrClientSocket)
{
    int clientSocket = *((int *)ptrClientSocket);
    free(ptrClientSocket); // Deallocating from heap and allocating on stack.

    FILE *downloadFile;
    downloadFile = fopen(FILE_NAME, "rb");

    printf("New connection received! \n");

    fseek(downloadFile, 0L, SEEK_END);
    unsigned long fileSize = ftell(downloadFile);
    rewind(downloadFile);

    printf("File size to transmit: %li \n", fileSize);
    printf("Transmitting file... \n");

    char ungracefulDisconnect = 0;
    unsigned long readBytes = 0;
    while (readBytes < fileSize)
    {
        if ((readBytes + BUFFER_SIZE) > fileSize)
        {
            int remainingBytes = fileSize - readBytes;
            char sendBuffer[remainingBytes];

            fread(sendBuffer, remainingBytes, 1, downloadFile);

            int sendResult = send(clientSocket, sendBuffer, remainingBytes, 0);
            if (sendResult <= 0)
                break;

            readBytes += remainingBytes;
            continue;
        }

        char sendBuffer[BUFFER_SIZE];
        fread(sendBuffer, BUFFER_SIZE, 1, downloadFile);

        int sendResult = send(clientSocket, sendBuffer, BUFFER_SIZE, 0);
        if (sendResult <= 0)
            break;

        readBytes += BUFFER_SIZE;
    }

    //TODO: Send a packet telling the client download has finished succesfully.
    printf("File transmition has ended! \n");
    close(clientSocket);
    fclose(downloadFile);
}