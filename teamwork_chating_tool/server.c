#include "include/server.h"

int main (void) 
{
    struct sockaddr_in server_addr, client_addr;
    int sockfd, client_fd, maxfd, sel;
    socklen_t addr_len;
    int i = 0;
    char ipstr[128];
    int client_id = 1;
    char buffer[BUFFER_LENGTH];
    int recv_bytes;
    int target_fd, target_id;
    memset(client_info, 0, sizeof(client_info));

    /* 
     * 1. create socket
     *     AF_INET -- Ipv4
     *     SOCK_STREAM -- TCP
     *     0 -- default
     */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("error: init socket failed!\n");
        exit(errno);
    }
    set_sock_non_block(sockfd);
    sockfds[0] = sockfd;
    maxfd = sockfd;
    /*
     * 2. bind socket and addr
     */
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;                   // Ipv4
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);    // any ip address in host
    server_addr.sin_port = htons(SERVER_PORT);          // port
    if ((bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))) == -1)
    {
        printf("error: bind failed!\n");
        exit(errno);
    }

    /*
     * 3. listen
     */
    if ((listen(sockfd, 128)) == -1)
    {
        printf("error: listen failed!\n");
        exit(errno);
    }

    inet_ntop(AF_INET, &server_addr.sin_addr.s_addr, ipstr, sizeof(ipstr));
    printf("start socket: %s:%d\n", ipstr, SERVER_PORT);

    // set select params
    fd_set readfds, readfds_bak;
    struct timeval timeout;
    FD_ZERO(&readfds);
    FD_ZERO(&readfds_bak);
    FD_SET(sockfd, &readfds_bak);

    while(1)
    {
        // reset readfds and timeout after select
        readfds = readfds_bak;
        timeout.tv_sec = SELECT_TIMEOUT;
        timeout.tv_usec = 0;
        maxfd = update_maxfd(readfds, maxfd);
        // select readable socket
        sel = select(maxfd + 1, &readfds, NULL, NULL, &timeout);
        if (sel == -1)
        {
            printf("error: select failed!\n");
            exit(-1);
        }
        else if (sel == 0)
        {
            // no ready socket
            continue;
        }

        printf("%d\n", maxfd);
        // read every socket
        for (i = 0; i <= maxfd; ++i)
        {
            // current socket has not data
            if (!FD_ISSET(i, &readfds))
            {
                continue;
            }

            // server sockets
            if (i == sockfds[0])
            {
                addr_len = sizeof(client_addr);
                // accept connect
                if ((client_fd = accept(sockfd, (struct sockaddr*)&client_addr, &addr_len)) == -1)
                {
                    printf("error: accept socket failed! errno: %d\n", errno);
                    continue;
                }
                set_sock_non_block(client_fd);

                // add client socket
                sockfds[client_id] = client_fd;
                if (client_fd > maxfd)
                {
                    maxfd = client_fd;
                }
                FD_SET(client_fd, &readfds_bak);

                // convert ip in dotted decimal notation
                inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ipstr, sizeof(ipstr));
                
                // init client info
                client c;
                strcpy(c.ip, ipstr);
                c.port = ntohs(client_addr.sin_port);
                client_info[client_id] = c;
                
                // send id to client
                message_s2c msg;
                strcpy(msg.sender_name, "server_return_id");
                sprintf(msg.message, "%d" , client_id);

                memset(buffer, 0, BUFFER_LENGTH);
                memcpy(buffer, &msg, sizeof(msg));
                send(client_fd, buffer, BUFFER_LENGTH, 0);

                printf("accept socket, ip: %s\t, port: %d\n, sockefd:%d", ipstr, c.port, client_fd);
                printf("set id: %d\n", client_id);
                client_id++;
            } // if(i == sockfds[0])
            else
            {
                printf("client socket\n");
                // client socket
                memset(buffer, 0, BUFFER_LENGTH);

                /*
                 * receive message
                 */
                recv_bytes = recv(i, buffer, BUFFER_LENGTH, 0);
                if (recv_bytes == 0)
                {
                    printf("error: client closed!\n");
                    FD_CLR(i, &readfds_bak);

                    // find currnet socket owener
                    int index = 1;
                    for (; index < MAX_USER; index++)
                    {
                        if (sockfds[index] == i)
                        {
                            sockfds[index] == -1;
                            printf("------------%s\n", client_info[index].username);
                            strcpy(client_info[index].username, "");
                            //client_info[index].username == "";
                            printf("---------%s\n", client_info[index].username);
                        }
                    }

                    break;
                }
                else if (recv_bytes == -1)
                {
                    printf("error: read message failed!\n");
                    continue;
                }

                //printf("message from client: %s\n", buffer);

                // convert string to struct
                message_c2s msg;
                memcpy(&msg, buffer, recv_bytes);

                // deal message
                int type = msg.type;
                int sender_id = msg.sender_id;
                client c = client_info[sender_id];
                
                printf("receive:sender_id %d username %s message %s\n", msg.sender_id, c.username, msg.message);

                if (type == 1)
                {
                    printf("type: %d\n", type);
                    // type == 1 means client send his username
                    printf("%s\n", msg.message);
                    strcpy(client_info[sender_id].username, msg.message);
                    printf("%s\n", c.username);
                    continue;
                }
                else if (type == 2)
                {
                    // type == 2 means send message to other client
                    /*
                     * send to target client
                     */
                    printf("type: %d\n", type);
                    printf("send message: %d %s\n", msg.receiver_id, msg.message);
                    message_s2c msg2c;
                    strcpy(msg2c.sender_name, c.username);
                    strcpy(msg2c.message, msg.message);

                    memset(buffer, 0, BUFFER_LENGTH);
                    memcpy(buffer, &msg2c, sizeof(msg2c));

                    target_fd = sockfds[msg.receiver_id];
                    send(target_fd, buffer, BUFFER_LENGTH, 0);
                    
                    continue;
                }
                else if (type == 3)
                {
                    printf("type: %d\n", type);
                    // type == 3 means to show all online client
                    message_s2c msg2c;
                    strcpy(msg2c.sender_name, "server_return_online");
                    int index = 1;
                    //int no = 1;
                    for (; index < MAX_USER; ++index)
                    {
                        client cl = client_info[index];
                        if (&cl != NULL && cl.username != NULL && strcmp(cl.username, ""))
                        {
                            char info[1024];
                            sprintf(info, "%d.%s\n", index, cl.username);
                            strcat(msg2c.message, info);
                            //no++;
                        }
                    }
                    
                    printf("%s\n", msg2c.message);
                    // msg2c->message = malloc(sizeof(msg2c->message));
                    // strcpy(msg2c->sender_name, c->username);
                    // strcpy(msg2c->message, msg->message);

                    memset(buffer, 0, BUFFER_LENGTH);
                    memcpy(buffer, &msg2c, sizeof(msg2c));
                    send(i, buffer, BUFFER_LENGTH, 0);
                    printf("send\n");
                }
            } // else of (i == sockfds[0])
        } // for
    } // while(1)
    return 0;
}

/*
 * modify socket to non-block
 */
void set_sock_non_block(int sockfd)
{
    int flags;
    // get original attribute
    flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0)
    {
        printf("error: get socket attribute failed!\n");
        exit(-1);
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        printf("error: set socket attribute failed!\n");
        exit(-1);
    }
}

//函数：更新maxfd
int update_maxfd(fd_set fds, int maxfd) 
{
    int i;
    int new_maxfd = 0;
    for (i = 0; i <= maxfd; i++) 
    {
        if (FD_ISSET(i, &fds) && i > new_maxfd) 
        {
            new_maxfd = i;
        }
    }
    return new_maxfd;
}
