#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read function

#define PORT 8080
#define BUF_SIZE 4096
#define QUEUE_SIZE 10

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

    // print contents of buffer
    // printf("Message from client: %s", buf);

    // Open file for read only
    file = open(buf, O_RDONLY);
    if (file < 0) {
      printf("%s",buf);
      printf("File opening failed!\n");
      // return 0;
    }

    // Get all bytes from file
    while (1) {
      // Read from file
      bytes = read(file, buf, BUF_SIZE);
      if (bytes <= 0)
        break;

      // Write to socket
      write(sckt_accept, buf, bytes);
    }

    // Close file and socket
    close(file);
    close(sckt_accept);
  }

  return 0;
}