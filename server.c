#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE (200)

// create structure for sending info to thread
typedef struct info_t {
  int localsock; // save socket
  int localport; // save port
} info_t;

int file;
int usernumber = 0; // number of users
int array[20];      // socket array, max num = 20

// send thread
void *receivesocket(void *arg) {
  // receive structure
  // create a info ptr to use the info
  info_t *thread = (info_t *)arg;
  int sock = thread->localsock;
  int numport = thread->localport;
  char receivebuffer[BUF_SIZE]; // save message from client
  char sendbuffer[BUF_SIZE];
  char notebuffer[BUF_SIZE];

  while (1) {
    // receive return message from client
    memset(receivebuffer, 0, sizeof(receivebuffer));
    // receive message from client
    int n = recv(sock, receivebuffer, sizeof(receivebuffer), 0);
    //代表已經沒接收到東西
    if (n <= 0)
      break;
    printf("%d send message: %s \n", numport, receivebuffer);

    // save the chat record to file
    memset(notebuffer, 0, sizeof(notebuffer));
    sprintf(notebuffer, "%d", numport);
    strcat(notebuffer, ":");
    strcat(notebuffer, receivebuffer);
    strcat(notebuffer, "\n");
    write(file, notebuffer, sizeof(notebuffer));

    // send message to client except sending guy
    memset(sendbuffer, 0, sizeof(sendbuffer));
    sprintf(sendbuffer, "User %d", numport);
    strcat(sendbuffer, " send: ");
    strcat(sendbuffer, receivebuffer);
    strcat(sendbuffer, "\nInput message:");
  
    for (unsigned int i = 0; i < usernumber; i++) {
      if (array[i] != sock) {
        send(array[i], sendbuffer, strlen(sendbuffer), 0);
      }
    }
  }
  return NULL;
}

int main() {
  // prevent SIGPIPE to terminate program
  sigset_t sigmask;
  sigemptyset(&sigmask);
  sigaddset(&sigmask, SIGPIPE);
  int rc = pthread_sigmask(SIG_BLOCK, &sigmask, NULL);
  if (rc != 0) {
    printf("block sigpipe error\n");
  }

  // get server socket
  int serversock = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in serveraddr;
  memset(&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons(6777);
  socklen_t addrlen = sizeof(serveraddr);
  serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  if (bind(serversock, (struct sockaddr *)&serveraddr, addrlen) == -1) {
    perror("bind failed");
    exit(1);
  }

  // waiting for user require
  printf("It is port 6777\n");

  // cread a new sockaddr_int for accept client socket
  struct sockaddr_in clientaddr;
  // up to 20 users
  pthread_t user[20];

  int clientsock;
  socklen_t len = sizeof(clientaddr);

  info_t thread_array[20];

  int i;
  char portbuffer[BUF_SIZE];

  // start 20 users chat
  listen(serversock, 20);

  file = open("./info.txt", O_CREAT | O_WRONLY | O_APPEND);
  while (1) {
    clientsock = accept(serversock, (struct sockaddr *)&clientaddr, &len);
    if (clientsock < 0) {
      perror("connect failed!");
      exit(1);
    }

    printf("%d is coming the chatroom \n", clientaddr.sin_port);

    // send message to all users
    memset(portbuffer, 0, sizeof(portbuffer));
    sprintf(portbuffer, "%d", clientaddr.sin_port);
    strcat(portbuffer, " is coming the chatroom\n");
    strcat(portbuffer, " \nInput message:");
    for (i = 0; i < usernumber; i++) {
      send(array[i], portbuffer, strlen(portbuffer), 0);
    }

    thread_array[usernumber].localsock = clientsock;
    thread_array[usernumber].localport = clientaddr.sin_port;    

    array[usernumber] = clientsock;

    // create sending thread
    pthread_create(&user[usernumber], NULL, receivesocket,
                   &thread_array[usernumber]);
    usernumber++;
  }

  close(file);
  close(clientsock);
  close(serversock);

  return 0;
}