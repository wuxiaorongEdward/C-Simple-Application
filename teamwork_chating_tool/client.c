#include "include/client.h"

int main(void)
{
    my_id = -1;
    int port;
    struct sockaddr_in server_addr;
    char * username = malloc(sizeof(username));
    char * ip = malloc(sizeof(ip));
    char send_buffer[BUFFER_LENGTH];

    printf("please enter: \033[47;34m\033[38musername@host:port\033[0m\n");
    scanf("%[^@]@%[^:]:%d", username, ip, &port);

    // printf("%s\n", username);
    // printf("%s\n", ip);
    // printf("%s\n", port);

    // create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("error: init socket failed!\n");
        exit(errno);
    }

    // bind
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;                   // Ipv4
    server_addr.sin_addr.s_addr = inet_addr(ip);        // any ip address in host
    server_addr.sin_port = htons(port);                 // port

    // connect
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("error: connect failed!\n");
        exit(errno);
    }

    printf("connect to server: %s:%d\n", ip, port);

    // receive message
    pthread_t receive;
    pthread_create(&receive, NULL, receive_message, NULL);

    // wait for id
    int retry = 5;
    while (my_id == -1)
    {
        if (retry > 0)
        {
            sleep(1);
            retry--;
        }
        else
        {
            printf("server busy\n");
            exit(-1);
        }

    }

    // send username
    message_c2s msg;
    msg.type = 1;
    msg.sender_id = my_id;
    msg.receiver_id = -1;
    strcpy(msg.message, username);

    memset(send_buffer, 0 , BUFFER_LENGTH);
    memcpy(send_buffer, &msg, BUFFER_LENGTH);

    send(sockfd, send_buffer, BUFFER_LENGTH, 0);

    char command[10];
    while(1)
    {

        memset(send_buffer, 0 , BUFFER_LENGTH);
        memset(command, 0, sizeof(command));
        msg.sender_id = my_id;
        system("clear");
        printf("\033[40;32m\033[47m\033[5m==================================================================\033[0m\n");
        printf("\033[49;34menter 'q' to exit\n\033[0m\n");
        printf("\033[49;34menter 'ls' to show online client\n\033[0m\n");
        printf("\033[49;34menter 'send' and then enter 'id message' to send message to client\n\033[0m\n");
        printf("\033[40;32m\033[47m\033[5m==================================================================\033[0m\n");
        scanf("%s", command);
        if (!strcmp(command, "ls"))
        {
            msg.type = 3;
            msg.receiver_id = -1;
            strcpy(msg.message, "");
        }
        else if (!strcmp(command, "send"))
        {
            int id = -1;
            //char * message = malloc(sizeof(message));
            //scanf("%d %s", &id, message);
            scanf("%d", &id);
            getchar();
            fgets(msg.message, sizeof(msg.message), stdin);
            msg.message[strlen(msg.message) - 1] = '\0';
            printf("message:%s\n",msg.message );

            msg.type = 2;
            msg.receiver_id = id;
            //strcpy(msg.message, message);
            //free(message);
            //message = NULL;
        }
        else if (!strcmp(command, "q"))
        {
            exit(0);
        }
        else
        {
            printf("please follow the prompts.\nremember press 'enter' key on keybord after enter 'send'\n");
            continue;
        }

        memcpy(send_buffer, &msg, sizeof(msg));
        send(sockfd, send_buffer, BUFFER_LENGTH, 0);
        printf("send success\n");
    }// while(1)

    free(username);
    free(ip);
    username = NULL;
    ip = NULL;
    return 0;
}

void *receive_message()
{
    printf("start thread for receive\n");
    char recv_buffer[BUFFER_LENGTH];
    int recv_bytes;
    int target_fd, target_id;

    while(1)
    {
        /*
         * receive message
         */
        memset(recv_buffer, 0, BUFFER_LENGTH);
        // receive message from client, if socekt buffer is empty, thread will be blocked
        recv_bytes = recv(sockfd, recv_buffer, BUFFER_LENGTH, MSG_WAITALL);
        if (recv_bytes == 0)
        {
            printf("error: client closed!\n");
            break;
        }
        else if (recv_bytes == -1)
        {
            printf("error: read message failed!\n");
            continue;
        }

        //printf("get: %s\n", recv_buffer);

        // convert %sstring to struct
        message_s2c msg;
        memcpy(&msg, recv_buffer, recv_bytes);

        if (!strcmp(msg.sender_name, "server_return_id"))
        {
            my_id = atoi(msg.message);
            continue;
        }
        else if (!strcmp(msg.sender_name, "server_return_online"))
        {
            printf("all online clients:\n");
            printf("%s\n", msg.message);
            continue;
        }
        printf("receive message from \033[40;32m%s:\033[0m\n\033%s\n", msg.sender_name, msg.message);
    }
}
