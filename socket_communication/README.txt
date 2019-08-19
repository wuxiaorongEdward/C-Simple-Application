This progran is going to create a mini chatting tool, ervery time you can only send messages to a single person.
It uses C/S model. you send message to the server, then the server send your message to the target user.

First:
compile server.c and client.c.
----------------------------------------
gcc server.c -o s -lpthread

gcc client.c -o c -lpthread
----------------------------------------

Second:
Run server and client in different terminals, you can open serveral terminals
server
-----------------------------------------
./s
------------------------------------------
./c
----------------------------------------
please input server ip: 127.0.0.1
-------------------------------------------
First, you can input ls to see who is online
then,
@sb_name
message
input "quit" to leave
-------------------------------------------
please input your name :
edward
name: edward
---------------------------------------

input 'ls' to see online users:
ls
     Users List      
---------------------------
edward
wuxiaorong

after you(edward) know who is online, you can send message to him:
for example:
@wuxiaorong
How is it going

input 'quit' to close client.
