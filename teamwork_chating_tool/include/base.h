#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <malloc.h>
#include <fcntl.h>

#define SERVER_PORT 10000
#define BUFFER_LENGTH 1024
// select timout in seconds
#define SELECT_TIMEOUT  5

#define MAX_USER 20

typedef struct
{
    int type;
    int sender_id;
    int receiver_id;
    char message[256];
}message_c2s;

typedef struct
{
    char sender_name[30];
    char message[256];
}message_s2c;