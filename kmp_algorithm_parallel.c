#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

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
    if (argc != 3)
    {
        printf("Usage: %s <text_file> <pattern>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];
    char *pattern = argv[2];

    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Error opening file");
        return 1;
    }

    char **lines = (char **)malloc(MAX_LINES * sizeof(char *));
    if (!lines)
    {
        fprintf(stderr, "Memory allocation failed for lines array\n");
        fclose(file);
        return 1;
    }

    char *buffer = (char *)malloc(BUFFER_SIZE);
    if (!buffer)
    {
        fprintf(stderr, "Memory allocation failed for buffer\n");
        fclose(file);
        free(lines);
        return 1;
    }

    // Skip header
    fgets(buffer, BUFFER_SIZE, file);

    int line_count = 0;
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

    int total_sum = 0;
    double start_time = omp_get_wtime();

    // Parallel processing
    #pragma omp parallel for
    for (int i = 0; i < line_count; i++)
    {
        char *line = lines[i];

        char *token = strtok(line, ","); // id
        token = strtok(NULL, ",");       // title
        token = strtok(NULL, ",");       // text

        if (token != NULL)
        {
            size_t len = strlen(token);
            if (len > 1 && token[0] == '"' && token[len - 1] == '"')
            {
                memmove(token, token + 1, len - 2);
                token[len - 2] = '\0';
            }

            int count = KMPSearch(pattern, token);

            // Simply add count to total_sum without atomic/reduction
            total_sum += count;
        }
    }

    double end_time = omp_get_wtime();

    printf("Total matches: %d\n", total_sum);
    printf("Time taken: %f seconds\n", end_time - start_time);

    // Free allocated lines
    for (int i = 0; i < line_count; i++)
        free(lines[i]);
    free(lines);

    return 0;
}
