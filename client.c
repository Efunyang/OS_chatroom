#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE (200)

int file;
char filepath[BUF_SIZE];
char port[BUF_SIZE];

// send thread
void *sendsocket(void *arg) {
  // store send message
  char sendbuffer[BUF_SIZE];
  // store message into file
  char filebuffer[BUF_SIZE];
  int st = *(int *)arg;
  while (1) {
    // initialize sendbuffer
    memset(sendbuffer, 0, sizeof(sendbuffer));
    printf("Input message:\n");
    // get message
    scanf("%s", sendbuffer);
    // initialize filebuffer
    memset(filebuffer, 0, sizeof(filebuffer));
    strcat(filebuffer, sendbuffer);
    strcat(filebuffer, "\n");
    // write to file
    write(file, filebuffer, sizeof(filebuffer));
    // send message
    send(st, sendbuffer, strlen(sendbuffer), 0);
  }
  return NULL;
}

// accept thread
void *receivesocket(void *arg) {
  int st = *(int *)arg;
  char receivebuffer[BUF_SIZE];
  char filebuffer[BUF_SIZE];

  while (1) {
    // read return data from server
    memset(receivebuffer, 0, sizeof(receivebuffer));
    int n = recv(st, receivebuffer, sizeof(receivebuffer), 0);
    // whether communicate is end
    if (n <= 0)
      break;
    memset(filebuffer, 0, sizeof(filebuffer));
    strcat(filebuffer, receivebuffer);
    strcat(filebuffer, "\n");

    // write file
    write(file, filebuffer, sizeof(filebuffer));
    // print message
    printf("%s\n", receivebuffer);
  }
  return NULL;
}

int main() {
  int newsock = socket(AF_INET, SOCK_STREAM, 0);
  // get server socket address
  struct sockaddr_in serveraddr;
  memset(&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;                     // IPv4 structure
  serveraddr.sin_port = htons(6777);                   // host to network
  serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // turn IP into decimal
  socklen_t addrlen = sizeof(serveraddr);
  // connet to server
  if (connect(newsock, (struct sockaddr *)&serveraddr, addrlen) == 0) {
    printf("已連到server端\n");
  }

  // create structure
  struct sockaddr_in localaddr;
  memset(&localaddr, 0, sizeof(localaddr));
  sprintf(port, "%d", localaddr.sin_port);
  strcat(filepath, "./usernote/");
  strcat(filepath, port);

  // open file
  mkdir("./usernote", 0777);
  file = open(filepath, O_CREAT | O_EXCL | O_WRONLY | O_APPEND | O_NONBLOCK);
  if (file == -1) {
    printf("can't open file\n");
  }
  // create send,receive two thread
  pthread_t thread1, thread2;
  pthread_create(&thread1, NULL, sendsocket, &newsock);
  pthread_create(&thread2, NULL, receivesocket, &newsock);
  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  // close files
  close(file);
  close(newsock);

  return 0;
}