#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 256

void error_handling(char *);

int main(int ac, char *av[]) {
  char buf[BUF_SIZE];
  int server_socket, client_socket;
  struct sockaddr_in server_address, client_address;
  socklen_t client_address_size;

  if (ac != 2) {
    printf("Usage : %s <port>\n", av[0]);
    exit(1);
  }

  server_socket = socket(PF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) error_handling("socket");

  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  server_address.sin_port = htons(atoi(av[1]));

  if (bind(server_socket, &server_address, sizeof(server_address)) == -1)
    error_handling("bind");
  printf("Waiting for connecting...\n");
  if (listen(server_socket, 2) == -1) error_handling("listen");

  client_address_size = sizeof(client_address);
  if ((client_socket =
           accept(server_socket, &client_address, &client_address_size)) == -1)
    error_handling("accept");

  char filename[BUF_SIZE];
  size_t filesize;

  recv(client_socket, filename, sizeof(filename), 0);
  filename[strlen(filename)] = '\0';
  printf("Received file: %s\n", filename);

  FILE *file = fopen(filename, "wb");
  if (file == NULL) {
    error_handling("file");
  }

  recv(client_socket, &filesize, sizeof(filesize), 0);

  size_t bytes_received = 0;
  char buffer[BUF_SIZE];
  while (bytes_received < filesize) {
    int nbyte = recv(client_socket, buffer, sizeof(buffer), 0);
    fwrite(buffer, 1, nbyte, file);
    bytes_received += nbyte;
  }

  printf("File received successfully!\n");

  fclose(file);
  close(client_socket);
  close(server_socket);
  return 0;
}

void error_handling(char *msg) {
  perror(msg);
  exit(1);
}
