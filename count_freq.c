#include <stdio.h>
#include <string.h>
#include <omp.h> // لإضافة التايمر

int countFreq(const char *pat, const char *txt)
{
    int M = strlen(pat);
    int N = strlen(txt);
    int res = 0;

    for (int i = 0; i <= N - M; i++) {
        int j;
        for (j = 0; j < M; j++) {
            if (txt[i + j] != pat[j])
                break;
        }
        if (j == M)
            res++;
    }
    return res;
}

int main()
{
    FILE *file = fopen("i.csv", "r"); // اسم الملف
    if (file == NULL) {
        printf("Error: could not open file.\n");
        return 1;
    }

    char txt[100000]; // مساحة كافية للملف
    size_t bytesRead = fread(txt, sizeof(char), sizeof(txt) - 1, file);
    fclose(file);
    txt[bytesRead] = '\0'; // نهاية النص

    char pat[100];
    printf("Enter pattern to search: ");
    scanf("%99s", pat);

    double start_time = omp_get_wtime();
    int count = countFreq(pat, txt);
    double end_time = omp_get_wtime();

    printf("Total matches: %d\n", count);
    printf("Time taken: %f seconds\n", end_time - start_time);

    return 0;
}

