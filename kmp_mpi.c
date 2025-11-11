#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MAX_LINES 1000000
#define BUFFER_SIZE 1024*1024

// ======================== KMP Functions =========================
void computeLPSArray(char *pat, int M, int lps[])
{
    int len = 0;
    int i = 1;
    lps[0] = 0;

    while (i < M)
    {
        if (pat[i] == pat[len])
        {
            len++;
            lps[i] = len;
            i++;
        }
        else
        {
            if (len != 0)
                len = lps[len - 1];
            else
            {
                lps[i] = 0;
                i++;
            }
        }
    }
}

int KMPSearch(char *pat, char *txt)
{
    if (!txt) return 0;

    int M = strlen(pat);
    int N = strlen(txt);

    int *lps = (int *)malloc(M * sizeof(int));
    if (!lps)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    computeLPSArray(pat, M, lps);

    int i = 0, j = 0, res = 0;
    while (i < N)
    {
        if (pat[j] == txt[i])
        {
            j++;
            i++;
        }

        if (j == M)
        {
            res++;
            j = lps[j - 1];
        }
        else if (i < N && pat[j] != txt[i])
        {
            if (j != 0)
                j = lps[j - 1];
            else
                i++;
        }
    }

    free(lps);
    return res;
}

// ======================== Main =========================
int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 3)
    {
        if (rank == 0)
            printf("Usage: %s <text_file> <pattern>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    char *filename = argv[1];
    char pattern[256];
    strcpy(pattern, argv[2]);

    char **lines = NULL;
    int line_count = 0;

    if (rank == 0)
    {
        FILE *file = fopen(filename, "r");
        if (!file)
        {
            perror("Error opening file");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        lines = (char **)malloc(MAX_LINES * sizeof(char *));
        char *buffer = (char *)malloc(BUFFER_SIZE);
        if (!lines || !buffer)
        {
            fprintf(stderr, "Memory allocation failed\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Skip header
        fgets(buffer, BUFFER_SIZE, file);

        while (fgets(buffer, BUFFER_SIZE, file) && line_count < MAX_LINES)
        {
            buffer[strcspn(buffer, "\n")] = '\0';
            lines[line_count] = strdup(buffer);
            if (!lines[line_count])
            {
                fprintf(stderr, "Memory allocation failed for line %d\n", line_count);
                break;
            }
            line_count++;
        }

        fclose(file);
        free(buffer);
    }

    // Broadcast line count to all processes
    MPI_Bcast(&line_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Broadcast pattern
    int pattern_len = strlen(pattern);
    MPI_Bcast(&pattern_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(pattern, pattern_len + 1, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Calculate range for each process
    int chunk_size = line_count / size;
    int start = rank * chunk_size;
    int end = (rank == size - 1) ? line_count : start + chunk_size;

    // Broadcast lines (all processes get full data)
    if (rank != 0)
        lines = (char **)malloc(MAX_LINES * sizeof(char *));
    for (int i = 0; i < line_count; i++)
    {
        int len = 0;
        if (rank == 0 && lines[i])
            len = strlen(lines[i]) + 1;
        MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (len > 0)
        {
            if (rank != 0)
                lines[i] = (char *)malloc(len);
            MPI_Bcast(lines[i], len, MPI_CHAR, 0, MPI_COMM_WORLD);
        }
        else
        {
            lines[i] = NULL;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    // Local computation
    int local_sum = 0;
    for (int i = start; i < end; i++)
    {
        char *line = lines[i];
        if (!line) continue;

        char *saveptr;
        char *token = strtok_r(line, ",", &saveptr); // id
        token = strtok_r(NULL, ",", &saveptr);       // title
        token = strtok_r(NULL, ",", &saveptr);       // text

        if (token != NULL)
        {
            size_t len = strlen(token);
            if (len > 1 && token[0] == '"' && token[len - 1] == '"')
            {
                memmove(token, token + 1, len - 2);
                token[len - 2] = '\0';
            }
            local_sum += KMPSearch(pattern, token);
        }
    }

    int total_sum = 0;
    MPI_Reduce(&local_sum, &total_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();

    if (rank == 0)
    {
        printf("Total matches: %d\n", total_sum);
        printf("Time taken: %f seconds\n", end_time - start_time);
    }

    for (int i = 0; i < line_count; i++)
        if (lines[i]) free(lines[i]);
    free(lines);

    MPI_Finalize();
    return 0;
}
