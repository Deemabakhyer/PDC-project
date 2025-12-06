/*
 * MPI Pattern Matching with Scatter+Gather Timing
 *
 * Reads a CSV file and counts occurrences of a pattern in the text column.
 * Measures Communication, Computation, Scatter+Gather, and Total times.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MAX_LINES 1000000
#define BUFFER_SIZE 1024*1024
#define SG_SIZE 1024*1024  // fixed size array for Scatter+Gather

// ======================== KMP Functions =========================
void computeLPSArray(char *pat, int M, int lps[])
{
    int len = 0, i = 1;
    lps[0] = 0;
    while (i < M)
    {
        if (pat[i] == pat[len]) { len++; lps[i++] = len; }
        else { if(len!=0) len = lps[len-1]; else lps[i++] = 0; }
    }
}

int KMPSearch(char *pat, char *txt)
{
    if(!txt) return 0;
    int M = strlen(pat), N = strlen(txt);
    int *lps = (int*)malloc(M*sizeof(int));
    if(!lps) { fprintf(stderr,"Memory allocation failed\n"); exit(1); }
    computeLPSArray(pat,M,lps);

    int i=0,j=0,res=0;
    while(i<N)
    {
        if(pat[j]==txt[i]) { i++; j++; }
        if(j==M) { res++; j=lps[j-1]; }
        else if(i<N && pat[j]!=txt[i]) { j? j=lps[j-1]:i++; }
    }
    free(lps);
    return res;
}

// ======================== Main =========================
int main(int argc, char *argv[])
{
    MPI_Init(&argc,&argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);

    if(argc!=3)
    {
        if(rank==0) printf("Usage: %s <text_file> <pattern>\n", argv[0]);
        MPI_Finalize(); return 1;
    }

    char *filename = argv[1];
    char pattern[256]; strcpy(pattern,argv[2]);
    char **lines = NULL;
    int line_count=0;

    double comm_time=0.0, comp_time=0.0, sg_time=0.0;

    // ======================== Master reads file ==========================
    double t0_comm = MPI_Wtime();
    if(rank==0)
    {
        FILE *file=fopen(filename,"r");
        if(!file){ perror("Error opening file"); MPI_Abort(MPI_COMM_WORLD,1); }
        lines = (char**)malloc(MAX_LINES*sizeof(char*));
        char *buffer = (char*)malloc(BUFFER_SIZE);
        fgets(buffer,BUFFER_SIZE,file); // skip header
        while(fgets(buffer,BUFFER_SIZE,file) && line_count<MAX_LINES)
        {
            buffer[strcspn(buffer,"\n")]=0;
            lines[line_count] = strdup(buffer);
            if(!lines[line_count]) { fprintf(stderr,"Memory allocation failed\n"); break; }
            line_count++;
        }
        fclose(file); free(buffer);
    }
    t0_comm = MPI_Wtime() - t0_comm; comm_time += t0_comm;

    // ======================== Broadcast line_count and pattern ==========================
    double t0 = MPI_Wtime();
    MPI_Bcast(&line_count,1,MPI_INT,0,MPI_COMM_WORLD);
    int pattern_len = strlen(pattern);
    MPI_Bcast(&pattern_len,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(pattern,pattern_len+1,MPI_CHAR,0,MPI_COMM_WORLD);
    double t1 = MPI_Wtime(); comm_time += (t1-t0);

    // ======================== Broadcast each line ==========================
    t0 = MPI_Wtime();
    if(rank!=0) lines=(char**)malloc(MAX_LINES*sizeof(char*));
    for(int i=0;i<line_count;i++)
    {
        int len = (rank==0 && lines[i])? strlen(lines[i])+1:0;
        MPI_Bcast(&len,1,MPI_INT,0,MPI_COMM_WORLD);
        if(len>0)
        {
            if(rank!=0) lines[i] = (char*)malloc(len);
            MPI_Bcast(lines[i],len,MPI_CHAR,0,MPI_COMM_WORLD);
        }
        else lines[i]=NULL;
    }
    t1=MPI_Wtime(); comm_time += (t1-t0);
    MPI_Barrier(MPI_COMM_WORLD);

    // ======================== Scatter + Gather ==========================
    int *sendbuf=NULL, *recvbuf=NULL;
    if(rank==0){ sendbuf=(int*)malloc(SG_SIZE*sizeof(int)); for(int i=0;i<SG_SIZE;i++) sendbuf[i]=i; }
    recvbuf=(int*)malloc((SG_SIZE/size)*sizeof(int));

    t0 = MPI_Wtime();
    MPI_Scatter(sendbuf, SG_SIZE/size, MPI_INT, recvbuf, SG_SIZE/size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Gather(recvbuf, SG_SIZE/size, MPI_INT, sendbuf, SG_SIZE/size, MPI_INT, 0, MPI_COMM_WORLD);
    t1 = MPI_Wtime(); sg_time += (t1-t0);

    if(sendbuf) free(sendbuf);
    if(recvbuf) free(recvbuf);

    // ======================== Compute local sum ==========================
    int chunk_size = line_count/size;
    int start = rank*chunk_size;
    int end = (rank==size-1)? line_count: start+chunk_size;
    int local_sum=0;

    t0=MPI_Wtime();
    for(int i=start;i<end;i++)
    {
        char *line=lines[i]; if(!line) continue;
        char *saveptr; char *token=strtok_r(line,",",&saveptr); token=strtok_r(NULL,",",&saveptr); token=strtok_r(NULL,",",&saveptr);
        if(token){ size_t len=strlen(token); if(len>1 && token[0]=='"' && token[len-1]=='"'){ memmove(token,token+1,len-2); token[len-2]=0; }
            local_sum += KMPSearch(pattern,token);
        }
    }
    t1=MPI_Wtime(); comp_time += (t1-t0);

    // ======================== Reduce ==========================
    t0=MPI_Wtime();
    int total_sum=0;
    MPI_Reduce(&local_sum,&total_sum,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
    t1=MPI_Wtime(); comm_time += (t1-t0);

    double total_time = comm_time + comp_time + sg_time;

    if(rank==0)
    {
        printf("=== MPI Pattern Matching Report ===\n");
        printf("Processes: %d\n", size);
        printf("Total Matches: %d\n", total_sum);
        printf("Communication Time: %f\n", comm_time);
        printf("Scatter+Gather Time: %f\n", sg_time);
        printf("Computation Time: %f\n", comp_time);
        printf("Total Time: %f\n", total_time);
    }

    for(int i=0;i<line_count;i++) if(lines[i]) free(lines[i]);
    free(lines);

    MPI_Finalize();
    return 0;
}
