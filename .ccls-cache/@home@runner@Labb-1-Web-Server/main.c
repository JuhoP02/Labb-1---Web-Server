#include <netdb.h>
#include <netinet/in.h>
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

char *code_404 = "404 Not Found";
char *code_200 = "200 OK";

void Parse(char *message);
void BuildResponse(char *buf, long int length, char *stat_code,
                   char *mime_type);
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
  // Declare path to the mime map file
  char *mime_map = "mime_map.txt";
  // Declare a path to 404 page
  char *notfound_path = "sample_website/404_not_found.html";
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
    // printf("Socket failed.\n");
    perror("Socket Failed");
    return (-1);
  }

  // Set Socket Options
  setsockopt(sckt, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));

  b = bind(sckt, (struct sockaddr *)&channel, sizeof(channel));
  if (b < 0) {
    // printf("Bind failed!\n");
    perror("Bind Failed");
    return (-1);
  }

  l = listen(sckt, QUEUE_SIZE);
  if (l < 0) {
    // printf("Listen failed!\n");
    perror("Listen Failed failed");
    return (-1);
  }

  while (1) {

    // Reset buffer
    memset(&buf, 0, BUF_SIZE);

    sckt_accept = accept(sckt, 0, 0);
    if (sckt_accept < 0) {
      // printf("Socket accept failed!\n");
      perror("Socket accept Failed");
      return (-1);
    }

    // Read from accepted socket to the buffer
    read(sckt_accept, buf, BUF_SIZE);

    // Get Filename from request
    Parse(buf);

    char final_path[BUF_SIZE];
    memset(&final_path, 0, sizeof(final_path));

    strcat(final_path, file_path);
    strcat(final_path, buf);

    // Store the current status code
    char stat_code[32];
    memset(&stat_code, 0, sizeof(stat_code));

    long int size = 0;

    // Try opening file with requested name
    file = open(final_path, O_RDONLY);
    if (file < 0) {
      // Set status code to 404
      strcpy(stat_code, code_404);
      // Set path to 404 page
      strcpy(final_path, notfound_path);

      file = open(final_path, O_RDONLY);

    } else { // Found a file (code 200)
      strcpy(stat_code, code_200);
    }

    // Get Length of the requested file
    struct stat file_stat;
    if (fstat(file, &file_stat) == -1) {
      printf("Could not get size of file!\n");
    } else
      size = (long int)file_stat.st_size;

    char *file_type; // File extension
    char *mime_type; // Mime type (text/html)

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

    printf("%s", buf);

    // Send response
    send(sckt_accept, buf, strlen(buf), 0);

    // Get all bytes from file (body)
    while (1) {

      // memset(&buf, 0, BUF_SIZE);

      // Read from file
      bytes = read(file, &buf, BUF_SIZE);

      if (bytes <= 0)
        break;

      // Write to socket
      send(sckt_accept, buf, bytes, 0);
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

void BuildResponse(char *buf, long int length, char *stat_code,
                   char *mime_type) {
  memset(buf, 0, BUF_SIZE);

  snprintf(buf, BUF_SIZE,
           "HTTP/1.1 %s\r\nServer: Demo Web Server\r\nContent-Type: "
           "%s\r\nContent-Length: %ld\r\n\r\n",
           stat_code, mime_type, length);
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
      // Get pointer to ' '
      type = strrchr(line, ' ');
      if (type != NULL) { // If found a MIME type from file
        fclose(f);
        return strtok(type, "\n") + 1;
      }
      break;
    }
  }

  // Close file
  fclose(f);
  return "application/octet-stream"; // bytes
}