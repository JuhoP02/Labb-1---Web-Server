#include "mime.h"
#include <string.h>

char *map_name = "mime_map.txt";

char *MapMimeType(const char *file_type) {
  // Open MIME map file
  FILE *f = fopen(map_name, "r");

  char *type;

  char *line;
  // Read lines in file
  while (fgets(line, 64, f) != NULL) {
    if (strcmp(line, file_type) == 1) { // If the line includes the type
      fclose(f);
      type = strrchr(line, '\r');
      if (type != NULL)
        return type + 1;
    }
  }

  // Close file
  fclose(f);

  return "text/plain";
}