#include <netdb.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 8080
#define BUF_SIZE 4096
#define QUEUE_SIZE 10

typedef struct status_code {
  int code;
  char *text;
} status_code;

status_code code_404 = {.code = 404, .text = "404 Not Found"};
status_code code_200 = {.code = 200, .text = "200 OK"};

char *mime_map = "mime_map.txt";

void Parse(char *message);
void BuildResponse(char *buf, long int length, status_code code,
                   char mime_type[]);
char *GetFileType(const char *path);
char *MapMimeType(const char *mime_file, const char *file_type);

int main(void) {

  // Declare socket
  int sckt;
  // Declare socket_accept
  int sckt_accept;
  // Declare binding
  int b;
  // Declare listen
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
    memset(&final_path, 0, sizeof(final_path));

    strcat(final_path, file_path);
    strcat(final_path, buf);

    // Open file for read only
    /*file = open(final_path, O_RDONLY);
    if (file < 0) {
      printf("File opening failed!\n");
    }*/

    status_code stat_code;
    long int size = 0;

    // Try opening file with requested name
    // FILE *f = fopen(final_path, "rb");
    file = open(final_path, O_RDONLY);
    if (file < 0) {
      printf("Final path: %s\n", final_path);
      printf("File descriptor: %d\n", file);
      stat_code = code_404;

      printf("File opening failed!\n");

      // Set path to 404 page
      strcpy(final_path, "sample_website/404_not_found.html");

      file = open(final_path, O_RDONLY);

    } else { // Found a file (code 200)
      stat_code = code_200;
      printf("File descriptor: %d\n", file);
    }
    /*
        if (f == NULL) { // 404 resource not found
          stat_code = code_404;

          printf("File opening failed!\n");

          // Set path to 404 page
          strcpy(final_path, "sample_website/404_not_found.html");
          f = fopen(final_path, "rb");

        } else { // Found a file (code 200)
          stat_code = code_200;
        }
    */
    // Get Length of file in bytes
    /*fseek(f, 0L, SEEK_END);
    size = ftell(f);
    rewind(f);*/

    struct stat file_stat;
    if (fstat(file, &file_stat) == -1) {
      printf("Could not get size of file!\n");
    }
    size = (long int)file_stat.st_size;

    char *file_type;
    char *mime_type;

    // Gets the file type from path
    file_type = GetFileType(final_path);
    if (file_type == NULL) {
      printf("No File Type Found!\n");
    }

    // Gets the MIME type from file type
    mime_type = MapMimeType(mime_map, file_type);
    if (mime_type == NULL) {
      printf("No MIME Type Found!\n");
    }

    // Creates a response for connection
    BuildResponse(buf, size, stat_code, mime_type);

    printf("Response: %s", buf);

    // Send response
    // send(sckt_accept, buf, strlen(buf), 0);
    send(sckt_accept, buf, strlen(buf), 0);

    int sent_bytes = 0;

    // Get all bytes from file (body)
    while (1) {

      memset(&buf, 0, BUF_SIZE);

      // Read from file
      // bytes = read(file, buf, BUF_SIZE);
      // bytes = fread(&buf, 1, BUF_SIZE, f);

      bytes = read(file, &buf, BUF_SIZE);

      if (bytes <= 0)
        break;

      // Write to socket
      sent_bytes += send(sckt_accept, buf, bytes, 0);
      // sent_bytes += write(sckt_accept, buf, bytes);

      printf("Sent %d bytes (%d/%ld)\n", bytes, sent_bytes, size);
      // sent_bytes += send(sckt_accept, buf, bytes, 0);
    }

    // printf("Sent %d bytes\n\n", sent_bytes);

    // Close file and socket
    // fclose(f);
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

void BuildResponse(char *buf, long int length, status_code stat_code,
                   char mime_type[]) {

  // char append[BUF_SIZE];

  memset(buf, 0, BUF_SIZE);

  /*
  memset(&append, 0, sizeof(char));
  strcat(buf, "HTTP/1.1 ");
  strcat(buf, stat_code.text);
  strcat(buf, " \r\n");

  strcat(buf, "Server: Web Server\r\n");

  strcat(buf, "Content-Length: ");
  sprintf(append, "%d", (int)length);
  strcat(buf, append);
  // strcat(buf, ltoa(length)); // Seg fault (Sprintf)
  strcat(buf, "\r\n");

  strcat(buf, "Content-Type: ");
  strcat(buf, mime_type);
  strcat(buf, "\r\n\r\n");*/

  snprintf(buf, BUF_SIZE,
           "HTTP/1.1 %s\r\nServer: Web Server\r\nContent-Type: "
           "%s\r\nContent-Length: %d\r\n\r\n",
           stat_code.text, mime_type, (int)length);

  // HTTP/1.1 200 OK\r\n
  // Server: Web Server\r\n
  // Concent-Length: <length>\r\n
  // Concent-Type: text/html\r\n
  // \r\n
  // <body>
}

char *GetFileType(const char *path) {

  // Gets pointer to last '.'
  char *last;
  last = strrchr(path, '.');

  // Couldn't find a type
  if (last == NULL)
    return "txt";

  return last + 1;
}

char *MapMimeType(const char *mime_file, const char *file_type) {

  char *type;
  char line[64];

  // Open MIME map file
  FILE *f = fopen(mime_file, "r");

  // Read lines in file
  while (fgets(line, 64, f)) {

    // If the line includes the type
    if (strstr(line, file_type) != NULL) {
      type = strrchr(line, ' ');
      if (type != NULL) {
        fclose(f);
        return strtok(type, "\n") + 1;
      }
      break;
    }
  }

  // Close file
  fclose(f);
  return "application/octet-stream";
}