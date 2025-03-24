#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#define PATH_LEN 1000
#define BUF_SIZE 1024


void print_page(uint64_t address, uint64_t data, FILE *out) {
   fprintf(out, "0x%-15lx : %-10lx %-10ld %-10ld %-10ld %-10ld\n", address,
            data & (((uint64_t)1 << 55) - 1), (data >> 55) & 1, (data >> 61) & 1, (data >> 62) & 1, (data >> 63) & 1);
}

void get_pagemap_info(const char *proc, FILE *out)
{
   fprintf(out, "PAGEMAP\n"); 
   fprintf(out, "     addr         pfn  soft-dirty    file/shared    swapped     present\n");

   char path[PATH_LEN];
   snprintf(path, PATH_LEN, "/proc/%s/maps", proc);
   FILE *maps = fopen(path, "r");

   snprintf(path, PATH_LEN, "/proc/%s/pagemap", proc);
   int pm_fd = open(path, O_RDONLY);

   char buf[BUF_SIZE + 1] = "\0";
   int len;

   // чтение maps
   while ((len = fread(buf, 1, BUF_SIZE, maps)) > 0) {
      for (int i = 0; i < len; i++)
         if (buf[i] == 0)
            buf[i] = '\n';
      buf[len] = '\0';

      // проход по строкам из maps
      char *save_row;
      char *row = strtok_r(buf, "\n", &save_row);
      while (row) {
         // получение столбца участка адресного пространства
         char *addresses = strtok(row, " ");
         char *start_str, *end_str;

         // получение начала и конца участка адресного пространства
         if ((start_str = strtok(addresses, "-")) && (end_str = strtok(NULL, "-"))) {
            uint64_t start = strtoul(start_str, NULL, 16);
            uint64_t end = strtoul(end_str, NULL, 16);

            for (uint64_t i = start; i < end; i += sysconf(_SC_PAGE_SIZE)) {
               uint64_t offset;
               // поиск смещения, по которому в pagemap находится информация о текущей странице
               uint64_t index = i / sysconf(_SC_PAGE_SIZE) * sizeof(offset);

               pread(pm_fd, &offset, sizeof(offset), index);
               print_page(i, offset, out);
            }
         }
         row = strtok_r(NULL, "\n", &save_row);
      }
   }
   fclose(maps);
   close(pm_fd);
}


int main(int argc, char **argv) {
   if (argc != 2) {
      printf("ERR: need %s pid", argv[0]);
      exit(EXIT_FAILURE);
   }
   FILE *file = fopen("./savedata/pagemap.txt", "w");
   if (!file) {
      perror("fopen");
      exit(EXIT_FAILURE);
   }
   get_pagemap_info(argv[1], file);
   fclose(file);
}