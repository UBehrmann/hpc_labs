#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_KMER 100

typedef struct {
    int count;
    char kmer[MAX_KMER];
} KmerEntry;

typedef struct {
    KmerEntry *entries;
    int count;
    int capacity;
} KmerTable;

void init_kmer_table(KmerTable *table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void add_kmer(KmerTable *table, const char *kmer) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->entries[i].kmer, kmer) == 0) {
            table->entries[i].count++;
            return;
        }
    }

    if (table->count >= table->capacity) {
        table->capacity = (table->capacity == 0) ? 1 : table->capacity * 2;
        table->entries = realloc(table->entries, table->capacity * sizeof(KmerEntry));
        if (!table->entries) {
            perror("Error reallocating memory for k-mer table");
            exit(1);
        }
    }

    strcpy(table->entries[table->count].kmer, kmer);
    table->entries[table->count].count = 1;
    table->count++;
}

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static char *read_file(const char *filename, long *file_size) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc((size_t)*file_size + 1);
    if (!buffer) {
        perror("Error allocating memory for file buffer");
        fclose(file);
        return NULL;
    }

    if (fread(buffer, 1, (size_t)*file_size, file) != (size_t)*file_size) {
        fprintf(stderr, "Error reading file.\n");
        free(buffer);
        fclose(file);
        return NULL;
    }
    buffer[*file_size] = '\0';
    fclose(file);
    return buffer;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <k>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_file = argv[1];
    int k = atoi(argv[2]);
    char kmer[MAX_KMER];

    if (k <= 0) {
        fprintf(stderr, "Error: k must be a positive integer.\n");
        return EXIT_FAILURE;
    }

    long file_size;
    char *buffer = read_file(input_file, &file_size);
    if (!buffer) {
        return EXIT_FAILURE;
    }

    if (k > file_size) {
        fprintf(stderr, "Error: Reached end of file before reading k-mer.\n");
        free(buffer);
        return EXIT_FAILURE;
    }

    KmerTable table;
    init_kmer_table(&table);

    double t1 = now_sec();
    for (long i = 0; i <= file_size - k; i++) {
        memcpy(kmer, buffer + i, (size_t)k);
        kmer[k] = '\0';
        add_kmer(&table, kmer);
    }
    double t2 = now_sec();

    printf("Results:\n");
    for (int i = 0; i < table.count; i++) {
        printf("%s: %d\n", table.entries[i].kmer, table.entries[i].count);
    }
    printf("Time = %.6f s\n", t2 - t1);

    free(table.entries);
    free(buffer);

    return 0;
}
