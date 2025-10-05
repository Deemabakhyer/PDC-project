// Original C++ code source: GeeksforGeeks
// "KMP Algorithm for Pattern Searching"
// https://www.geeksforgeeks.org/kmp-algorithm-for-pattern-searching/
// Accessed on: October 5, 2025 by Deema Ali Bakhyer.
// ChatGPT assistance included Code conversion from C++ to C, CSV file reading, and token extraction.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>

// Preprocess the pattern to compute the LPS (Longest Prefix Suffix) array
void computeLPSArray(char *pat, int M, int lps[])
{
    int len = 0;
    int i = 1;
    lps[0] = 0; // lps[0] is always 0

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

// KMP search function
int KMPSearch(char *pat, char *txt)
{
    if (txt == NULL)
        return 0; // safeguard

    int M = strlen(pat);
    int N = strlen(txt);

    int lps[M];
    int j = 0; // index for pat[]
    int i = 0; // index for txt[]
    int res = 0;

    computeLPSArray(pat, M, lps);

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
    return res;
}

#define MAX_LINE_LEN 2048

// Driver code
int main()
{
    FILE *file = fopen("i.csv", "r");
    if (!file)
    {
        printf("Error: Could not open file.\n");
        return 1;
    }

    char line[MAX_LINE_LEN];
    double start = 0.0, end = 0.0;
    int total_sum = 0;

    // Skip header
    fgets(line, sizeof(line), file);

    char pat[256]; // Specify a reasonable size for the pattern
    printf("Enter pattern to search: ");
    scanf("%255s", pat); // Limit input to avoid buffer overflow

    while (fgets(line, sizeof(line), file))
    {
        // Remove newline character
        line[strcspn(line, "\n")] = '\0';

        // Extract up to 3rd field
        char *token = strtok(line, ","); // id
        token = strtok(NULL, ",");       // title
        token = strtok(NULL, ",");       // text

        if (token == NULL)
            continue; // skip malformed lines safely

        // Remove quotes if present
        size_t len = strlen(token);
        if (len > 1 && token[0] == '"' && token[len - 1] == '"')
        {
            memmove(token, token + 1, len - 2);
            token[len - 2] = '\0';
        }

        start = omp_get_wtime();
        total_sum += KMPSearch(pat, token);
        end = omp_get_wtime();
    }

    printf("Total matches: %d\n", total_sum);
    printf("Time taken: %f seconds\n", end - start);

    fclose(file);
    return 0;
}
