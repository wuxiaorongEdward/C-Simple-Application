#include "base.h"

typedef struct
{
    char username[30];
    char ip[15];
    int port;
}client;



// sockets for communicating with clients
int sockfds[MAX_USER] = {0};
client client_info[MAX_USER];
void set_sock_non_block(int sockfd);
int update_maxfd(fd_set fds, int maxfd);