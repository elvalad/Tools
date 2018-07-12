#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

#define HELLO_WORLD_SERVER_PORT    6666
#define LENGTH_OF_LISTEN_QUEUE     20
#define BUFFER_SIZE                1024
#define DEPTH_WIDTH                320
#define DEPTH_HEIGHT               240
#define POINTER_NUM                (DEPTH_WIDTH*DEPTH_HEIGHT)

typedef struct
{
    int index;
    int depth[POINTER_NUM];
}Node;

int main(int argc, char **argv)
{
    // set socket's address information
    struct sockaddr_in   server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);

    // create a stream socket
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        printf("Create Socket Failed!\n");
        exit(1);
    }

    //bind
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        printf("Server Bind Port: %d Failed!\n", HELLO_WORLD_SERVER_PORT);
        exit(1);
    }

    // listen
    if (listen(server_socket, LENGTH_OF_LISTEN_QUEUE))
    {
        printf("Server Listen Failed!\n");
        exit(1);
    }

    while(1)
    {
        struct sockaddr_in client_addr;
        socklen_t          length = sizeof(client_addr);

        int new_server_socket = accept(server_socket, (struct sockaddr*)&client_addr, &length);
        if (new_server_socket < 0)
        {
            printf("Server Accept Failed!\n");
            break;
        }

        Node *myNode=(Node*)malloc(sizeof(Node));

        int needRecv=sizeof(Node);
        char *buffer=(char*)malloc(needRecv);
        int pos=0;
        int len;
        while(pos < needRecv)
        {
            len = recv(new_server_socket, buffer+pos, BUFFER_SIZE, 0);
            if (len < 0)
            {
                printf("Server Recieve Data Failed!\n");
                break;
            }
            pos+=len;

        }
        close(new_server_socket);
        memcpy(myNode, buffer, needRecv);
        printf("<<<<<<<<<<Rece info test<<<<<<<<<<\n");
        printf("index:%d\n"
    			"depth[5000] :%5d depth[10000]:%5d depth[15000]:%5d depth[20000]:%5d\n"
    			"depth[25000]:%5d depth[30000]:%5d depth[35000]:%5d depth[40000]:%5d\n", 
    			myNode->index,
    			myNode->depth[5000],  myNode->depth[10000], myNode->depth[15000], myNode->depth[20000],
    			myNode->depth[25000], myNode->depth[30000], myNode->depth[35000], myNode->depth[40000]);
        buffer = NULL;
        myNode = NULL;
        free(buffer);
        free(myNode);
    }
    close(server_socket);

    return 0;
}