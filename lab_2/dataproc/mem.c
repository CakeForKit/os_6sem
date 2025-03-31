#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>


#define PATH_LEN 1000

typedef struct {
    unsigned long start_addr;
    unsigned long end_addr;
} MemoryRegion;

void print_region(int mem_fd, unsigned long start_addr,
    unsigned long end_addr, FILE * stream)
{
    size_t region_size = end_addr - start_addr;
    void * buffer = malloc (region_size);
    if (!buffer)
    {
        perror ("malloc");
        return;
    }
    if (lseek (mem_fd, start_addr, SEEK_SET) == (off_t) -1)
    {
        perror ("lseek");
        free (buffer);
        return;
    }
    ssize_t bytes_read = read(mem_fd, buffer, region_size);
    if (bytes_read == -1)
    {
        perror ("read");
        free (buffer);
        return;
    }
    fprintf(stream, " Memory region: %lx-%lx\n", start_addr, end_addr);
    for (size_t i = 0; i < region_size; i ++)
    {
        fprintf (stream, "%02x ", ((unsigned char *) buffer) [i]);
        if ((i + 1) % 16 == 0)
            fprintf (stream, "\n");
    }
    fprintf (stream, "\n");
    free (buffer);
}

MemoryRegion * get_memory_regions(char* pid, size_t * region_count)
{
    char path [PATH_LEN];
    snprintf (path, PATH_LEN, "/proc/%s/maps", pid);
    FILE *file = fopen(path, "r");
    if (!file)
    {
        perror ("fopen");
        return NULL;
    }
    size_t capacity = 10;
    MemoryRegion * regions = malloc(capacity * sizeof (MemoryRegion));
    if (!regions)
    {
        perror ("malloc");
        fclose (file);
        return NULL;
    }
    char *line = NULL;
    size_t line_size = 0;
    size_t count = 0;
    while (getline (& line, & line_size, file) != -1)
    {
        unsigned long start_addr, end_addr;
        if (sscanf (line, "%lx-%lx", &start_addr, &end_addr) == 2)
        {
            if (count >= capacity)
            {
                capacity *= 2;
                MemoryRegion * new_regions = realloc (regions, capacity * sizeof (MemoryRegion));
                if (!new_regions)
                {
                    perror ("realloc");
                    free (regions);
                    fclose (file);
                    free (line);
                    return NULL;
                }
                regions = new_regions;
            }
            regions [count]. start_addr = start_addr;
            regions [count]. end_addr = end_addr;
            count ++;
        }
    }
    fclose (file);
    free (line);
    *region_count = count;
    return regions;
}

void get_mem_info(char* pid, FILE * stream)
{
    size_t region_count = 0;
    MemoryRegion *regions = get_memory_regions(pid, &region_count);
    if (!regions)
    {
        fprintf (stderr, "Failed to get memory regions\n");
        return;
    }
    char mem_path [PATH_LEN];
    snprintf(mem_path, PATH_LEN, "/proc/%s/mem", pid);
    int mem_fd = open(mem_path, O_RDONLY);
    if (mem_fd == -1)
    {
        perror ("open");
        free (regions);
        return;
    }
    for (size_t i = 1; i < 4; i ++) {
        fprintf (stream, "Region %zu: %lx-%lx\n", i, regions[i].start_addr, regions[i].end_addr);
        print_region (mem_fd, regions[i]. start_addr, regions[i].end_addr, stream);
    }
    close (mem_fd);
    free (regions);
}


int main(int argc, char **argv) {
    if (argc != 2) {
       printf("ERR: need %s pid", argv[0]);
       exit(EXIT_FAILURE);
    }
    FILE *file = fopen("./savedata/mem.txt", "w");
    if (!file) {
       perror("fopen");
       exit(EXIT_FAILURE);
    }
    get_mem_info(argv[1], file);
    fclose(file);
 }



