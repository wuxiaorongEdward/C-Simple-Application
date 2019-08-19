/*
 * filename: server.c
 *
 * This is the chatroom server, a client send message to other user, the server will deal with it,It uses the default port 1998.
 * you can change is as define.
 *
 * Date: 2019-08-19
 * author: wuxiaorong   version 1.0
 * 
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<netinet/in.h>

#define PORT 1998
#define TRUE 1
#define FALSE 0
#define USER_NUMBER 5
#define MAX_SIZE 1024

typedef struct 
{
	int socket_id;
	char client_name[20];
	int flag;
}Client_info, *Client_info_p;

int current_index = 0;

//store client information
Client_info users[USER_NUMBER];


void add_new_client(Client_info users[], int max, int socket_id);
void *new_client_recv_send(void *args);


int main()
{   /*
	   *
	   * newsockfd: accept new client conect server
	   * client_len: the length of client_addr
	   *
	   */
	int newsockfd, client_len;

	int i;

	pthread_t thread;   //thread id

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);   //create a socket
	struct sockaddr_in servaddr, clientaddr;
	memset(&servaddr, 0, sizeof(servaddr));

	//bind ip and port
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    //initial the user flag
	for(i = 0; i < USER_NUMBER; i++)
	{
		users[i].flag = -1;
	}

	//bind socket with ip and port
    if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
	{
		perror("bind failed\n");
		exit(EXIT_FAILURE);
	}

	listen(sockfd, 10);

	while(1)
	{
         client_len = sizeof(clientaddr);
		 newsockfd = accept(sockfd, (struct sockaddr *)&clientaddr, &client_len);

		 if(newsockfd < 0)
		 {
			 perror("can not bind local address\n");
		 }
        
         printf("new socket : %d\n", newsockfd);

         add_new_client(users, USER_NUMBER, newsockfd);
		 pthread_create(&thread, NULL, new_client_recv_send, &newsockfd);
	}

}

//add a new socket id in this array
void add_new_client(Client_info users[], int max, int socket_id)
{
	int i = (current_index + 1) % USER_NUMBER;

	while(users[i].flag != -1)
	{
		i++;
	}
	users[i].socket_id = socket_id;
	users[i].flag = 1;
	current_index++;
}

void *new_client_recv_send(void *args)
{
	char buff[MAX_SIZE];

	char name[20];

	char *to_name;
   
	int sock_c = *(int *)args;

	int i;

	int current_sock;

	//receive name
	memset(name, 0, sizeof(buff));
	recv(sock_c, name, sizeof(name), 0);

	printf("%s\n", name);

	//set name
	strcpy(users[current_index].client_name, name);
	puts(users[current_index].client_name);

	while(1)
	{
        memset(buff, 0, sizeof(buff));
		recv(sock_c, buff, sizeof(buff), 0);

		if(buff[0] == '@')  //select user
		{
			to_name = strchr(buff, '@');
			to_name++;
            //to person name		
			printf("to name : %s\n", to_name);

			for(i = 0; i < USER_NUMBER; i++)
			{
				//search the target person
				if(!strcmp(to_name, users[i].client_name))
				{
                      current_sock = users[i].socket_id;
                      printf("search socket : %d\n", current_sock);
				}
            
			}
		}
        //if client ctrl+c to terminate the client, deal with it
		else if(strlen(buff) == 0)
		{

			for(i = 0; i < USER_NUMBER; i++)
			{
				if(users[i].socket_id == sock_c)
				{
					users[i].flag = -1;
					
					close(sock_c);

					printf("%d closed by user\n", sock_c);
					return NULL;

				}
		}
		}
		//if client quit, close client socket, and free its space
		else if(!strcmp(buff, "quit\n"))
		{
			printf("Bye!\n");
			for(i = 0; i < USER_NUMBER; i++)
			{
				if(users[i].socket_id == sock_c)
				{
					users[i].flag = -1;
					
					close(sock_c);

					printf("closed %d\n", sock_c);
					return NULL;

				}
			}
		}
		//receive message from client for listing online users
		else if(!strcmp(buff, "ls\n"))
		{
			send(sock_c, "ls\n", sizeof("ls\n"), 0);
			for(i = 0; i < USER_NUMBER; i++)
			{
				printf("i : %d\n", i);
				if(users[i].flag == 1)
				{
					printf("%d  ---> %s\n", i, users[i].client_name);
					send(sock_c, users[i].client_name, sizeof(users[i].client_name), 0);
				}
			}
		}
        else
		{
			//send message to the selected user format: message -- from_user
			puts(buff);
			buff[strlen(buff) - 1] = '\0';

			strcat(buff, " --- ");
			strcat(buff, name);
			send(current_sock, buff, sizeof(buff), 0);
		}


	}
}
