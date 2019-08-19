/*
 * filename: client.c
 *
 * chatroom client
 *
 * Date: 2019-08-19
 * author; wuxiaorong   version 1.0
 *
 */


#include<stdio.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<pthread.h>


/*
 *
 * Usage: First Enter your name.
 *  ls:  list online users' name
 *  quit: leave
 *  send message @sb_name
 *  message.
 */

void menu()
{
	printf("\033[32m-------------------------------------------\n\033[0m");
	printf("First, you can input ls to see who is online\n");
	printf("then, \n@sb_name\n");
	printf("message\n");
	printf("input \"quit\" to leave\n");
	printf("\033[32m-------------------------------------------\n\033[0m");
}
void setSockNonBlock(int sock) {
	int flags;
	flags = fcntl(sock, F_GETFL, 0);
	if (flags < 0) {
	 	perror("fcntl(F_GETFL) failed");
 		exit(EXIT_FAILURE);
 	}
	if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
	  	perror("fcntl(F_SETFL) failed");
  		exit(EXIT_FAILURE);
 	 }
}

void *receive_msg(void *sock)
{
	int sockfd = *(int *)sock;
//	printf("receive falg %d\n", sockfd);
	char buff[1024];
	char name[20];
//	printf("please input your name : ");
//	fgets(name, sizeof(name), stdin);
//
//    recv(sockfd, name, sizeof(name), 0);
    while(1)
	{
		memset(buff, 0, sizeof(buff));
		recv(sockfd, buff, sizeof(buff), 0);

		//if server was closed, close client
		if(strlen(buff) == 0)
		{
			//code
        	printf("Server was closed!\n");
			exit(EXIT_SUCCESS);
		}
		else  if(!strcmp(buff, "ls\n"))
		{
			//list online users
			printf("\033[44m     Users List      \n\033[0m");
			printf("---------------------------\n");
			continue;
        }
          // puts(buff);
	   printf("\033[32m%s\033[0m", buff);
//		puts(buff);
	}
}

void *send_msg(void *sock)
{
	int sockfd = *(int *)sock;
//	printf(" send falg %d\n", sockfd);
	char buff[1024];
	char name[20];
	printf("please input your name : \n");
	fgets(name, sizeof(name), stdin);

	printf("name: %s\n", name);
	send(sockfd, name, sizeof(name), 0);

	printf("\033[32m %s is online now\n \033[0m", name);

   while(1)
	{
	memset(buff, 0, sizeof(buff));
	fgets(buff, sizeof(buff), stdin);

	send(sockfd, buff, sizeof(buff), 0);

	if(!strcmp(buff, "quit\n"))
	{
		printf("Good Bye!\n");
		exit(EXIT_SUCCESS);
	}
	}
}

int main()
{
	int socket_client = socket(AF_INET, SOCK_STREAM, 0);

    char serv_ip[20];

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(1998);

	printf("please input server ip: ");
	scanf("%s", serv_ip);
	getchar();
	serveraddr.sin_addr.s_addr = inet_addr(serv_ip);
   
     menu();

	if((connect(socket_client, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) < 0)
	{
	    printf("connect fail\n");
		exit(1);
	}
//	setSockNonBlock(socket_client);
//	setSockNonBlock(0);
//	char sendBuf[1024];
//	char recvBuf[1024];

	pthread_t thread1;
	pthread_t thread2;

//	while(1)
//	{
		/*
  
        if(fget(sendbuff, sizeof(sendbuf), stdin) != NULL) 		
	       send(socket_client, sendBuf, sizeof(sendBuf), 0);

		if((strcmp(sendBuf, "exit\n")) == 0)
		{
			break;
		}

		recv(socket_client, recvBuf, sizeof(recvBuf), 0);

		printf("%s",recvBuf);

		memset(sendBuf, 0, sizeof(sendBuf));
		memset(recvBuf, 0, sizeof(recvBuf));
		*/

	  //Usage
	  //
     pthread_create(&thread1, NULL, receive_msg, &socket_client);
     pthread_create(&thread2, NULL, send_msg, &socket_client);
//	}
        while(1);
	close(socket_client);

	return 0;
}


