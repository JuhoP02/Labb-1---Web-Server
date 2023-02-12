#include <netdb.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read function
#include <stdbool.h>

#define PORT 8080
#define BUF_SIZE 4096
#define QUEUE_SIZE 10

void Parse(char *message);
void BuildResponse();

int main(void) {

  // Declare socket
  int sckt;
  // Declare socket_accept
  int sckt_accept;
  // Declare binding
  int b;
  // Decalre listen
  int l;
  // Declare option value
  int on = 1;
  // Declare a path to the folder
  char *file_path = "sample_website";
  // Declare file descriptor
  int file;
  // Declare bytes
  int bytes;
  // Declare buffer
  char buf[BUF_SIZE];

  // Hold IP-address
  struct sockaddr_in channel;
  memset(&channel, 0, sizeof(channel));

  channel.sin_family = AF_INET;
  channel.sin_addr.s_addr = htonl(INADDR_ANY);
  channel.sin_port = htons(PORT);

  // Open socket for connection
  sckt = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socket < 0) {
    printf("Socket failed.\n");
    return 0;
  }
  // Set Socket Options
  setsockopt(sckt, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));

  b = bind(sckt, (struct sockaddr *)&channel, sizeof(channel));
  if (b < 0) {
    printf("Bind failed!\n");
    return 0;
  }

  l = listen(sckt, QUEUE_SIZE);
  if (l < 0) {
    printf("Listen failed!\n");
    return 0;
  }

  while (1) {
    sckt_accept = accept(sckt, 0, 0);
    if (sckt_accept < 0) {
      printf("Socket accept failed!\n");
      return 0;
    }

    // Read from accepted socket to the buffer

    read(sckt_accept, buf, BUF_SIZE);

    // Get Filename from request
    Parse(buf);

    char final_path[BUF_SIZE];
    memset(&final_path, 0, BUF_SIZE * sizeof(char));

    strcat(final_path, file_path);
    strcat(final_path, buf);

    // Open file for read only
    /*file = open(final_path, O_RDONLY);
    if (file < 0) {
      printf("File opening failed!\n");
    }*/

    bool resource_found = true; 
    
    FILE *f = fopen(final_path, "r");
    long int size;
    if (f == NULL) {
      printf("File opening failed!\n");
      resource_found = false;
    } else {
      fseek(f, 0L, SEEK_END);
      size = ftell(f);
      rewind(f);
      // printf("Size of given file: %ld\n", size);
    }

    // Creates a response for connection
    BuildResponse(buf, size);
    
    printf("Response: %s", buf);

    // Send response
    send(sckt_accept, buf, strlen(buf), 0);

    // Get all bytes from file (body)
    while (1) {
      // Read from file
      bytes = read(file, buf, BUF_SIZE);
      if (bytes <= 0)
        break;

      // Write to socket
      // write(sckt_accept, buf, bytes);
      send(sckt_accept, buf, BUF_SIZE, 0);
    }

    // Close file and socket
    close(file);
    close(sckt_accept);
  }

  return 0;
}

void Parse(char *message) {

  const char *delim = "\n ";

  // Get first line
  char *token = strtok(message, delim);

  // Save first line
  char *array[2];

  // Split string with spaces
  for (int i = 0; i < 2; i++) {
    array[i] = token;
    token = strtok(NULL, delim);
  }

  strcpy(message, array[1]);
}

void BuildResponse(char *buf, long int length) {

  char append[BUF_SIZE];

  memset(&append, 0, sizeof(char));
  
  memset(buf, 0, BUF_SIZE * sizeof(char));

  strcat(buf, "HTTP/1.1 200 OK\r\n");
  strcat(buf, "Server: Web Server\r\n");

  strcat(buf, "Content-Length: ");
  sprintf(append,"%d",(int)length);
  strcat(buf, append);
  //strcat(buf, ltoa(length)); // Seg fault (Sprintf)
  strcat(buf, "\r\n");

  strcat(buf, "Content-Type: text/html\r\n");

  // HTTP/1.1 200 OK\r\n
  // Server: Web Server\r\n
  // Concent-Length: <length>\r\n
  // Concent-Type: text/html\r\n
  // \r\n
  // <body>
}

/*

const char *end_line = "\n";
const char *space = " ";

char *token = strtok(message, end_line);

printf("token: %s", token);

*/