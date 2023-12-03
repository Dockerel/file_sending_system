#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 256
#define READ 0
#define WRITE 1

void error_handling(char* message);

int main(int argc, char* argv[]) {
  int sock;
  struct sockaddr_in serv_addr;
  char filename[256];
  FILE* file = NULL;

  if (argc != 2) {
    printf("Usage : %s <PORT> \n", argv[0]);
    exit(1);
  }

  while (1) {
    printf("Enter the filename to send: ");
    scanf("%s", filename);

    int pipefd[2];
    char cmd[5] = "";
    pid_t pid;
    struct stat stat_buf;

    if (pipe(pipefd) < 0) error_handling("pipe");

    pid = fork();
    if (pid < 0) error_handling("fork");
    if (pid == 0) {  // child
      close(pipefd[READ]);

      if (stat(filename, &stat_buf) == -1) {
        close(pipefd[WRITE]);
        error_handling("stat");
      } else {
        printf("File name: %s\n", filename);
        printf("File size: %lld bytes\n", (long long)stat_buf.st_size);
        printf("====================\n");
        printf("Send this file (y/n) >>> ");
        scanf("%s", cmd);
        write(pipefd[WRITE], cmd, strlen(cmd) + 1);

        close(pipefd[WRITE]);
        exit(0);
      }
    } else {  // parent
      close(pipefd[WRITE]);
      wait(NULL);

      char cmd_buf[5] = "";
      read(pipefd[READ], cmd_buf, sizeof(cmd_buf));
      if (cmd_buf[0] == 'y') break;
      close(pipefd[READ]);
    }
  }
  printf("File name: %s\n", filename);

  sock = socket(PF_INET, SOCK_STREAM, 0);

  if (sock == -1) error_handling("socket");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(atoi(argv[1]));

  if (connect(sock, &serv_addr, sizeof(serv_addr)) == -1)
    error_handling("connect");

  send(sock, filename, strlen(filename), 0);

  file = fopen(filename, "rb");
  if (file == NULL) {
    error_handling("file");
  }

  fseek(file, 0, SEEK_END);
  size_t fsize = ftell(file);
  fseek(file, 0, SEEK_SET);

  send(sock, &fsize, sizeof(fsize), 0);

  char buffer[BUF_SIZE];
  size_t bytes_sent = 0;
  while (bytes_sent < fsize) {
    size_t read_bytes = fread(buffer, 1, BUF_SIZE, file);
    send(sock, buffer, read_bytes, 0);
    bytes_sent += read_bytes;
  }

  fclose(file);
  close(sock);
  return 0;
}

void error_handling(char* msg) {
  perror(msg);
  exit(1);
}
