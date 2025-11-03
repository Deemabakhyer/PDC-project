#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>

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
            {
                len = lps[len - 1];
            }
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
    if (txt == NULL) return 0;

    int M = strlen(pat);
    int N = strlen(txt);

    int *lps = (int *)malloc(M * sizeof(int));
    if (lps == NULL) {
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

    char *filename = argv[1]; // اسم الملف من التيرمنال
    char *pat = argv[2];      // النمط المراد البحث عنه

    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Error opening file");
        return 1;
    }

    size_t buffer_size = 1024 * 1024; // 1 MB buffer
    char *line = (char *)malloc(buffer_size);
    if (!line)
    {
        fprintf(stderr, "Memory allocation failed for line buffer\n");
        fclose(file);
        return 1;
    }

    // skip header
    fgets(line, buffer_size, file);

    int total_sum = 0;
    double start = omp_get_wtime();

    while (fgets(line, buffer_size, file))
    {
        line[strcspn(line, "\n")] = '\0';

        char *token = strtok(line, ","); // id
        token = strtok(NULL, ",");       // title
        token = strtok(NULL, ",");       // text

        if (!token) continue;

        size_t len = strlen(token);
        if (len > 1 && token[0] == '"' && token[len - 1] == '"')
        {
            memmove(token, token + 1, len - 2);
            token[len - 2] = '\0';
        }

        total_sum += KMPSearch(pat, token);
    }

    double end = omp_get_wtime();

    printf("Total matches: %d\n", total_sum);
    printf("Time taken: %f seconds\n", end - start);

    free(line);
    fclose(file);
    return 0;
}
