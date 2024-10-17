/*
Student No.: 111652031
Student Name: Pang-Chun, Chung
Email: caco.sc11@nycu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/
#include <iostream>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/time.h>
using namespace std;

void init(int dim, unsigned int *A, unsigned int *B){
    for(int i=0;i<dim;i++){
        for(int j=0;j<dim;j++){
            A[i * dim + j] = i * dim + j;
            B[i * dim + j] = i * dim + j;
        }
    }
}

void matrixMultiplication(unsigned int *A, unsigned int *B, unsigned int *C, int start, int end, int dim){
    for(int i=start;i<end;i++){
        for(int j=0;j<dim;j++){
            C[i * dim + j] = 0;
            for(int k=0;k<dim;k++){
                C[i * dim + j] += A[i * dim + k] * B[k * dim + j];
            }
        }
    }
}

unsigned int calCheckSum(unsigned int *C, int dim){
    unsigned int checkSum = 0;
    for(int i=0;i<dim;i++){
        for(int j=0;j<dim;j++){
            checkSum += C[i * dim + j];
        }
    }
    return checkSum;
}

int main(){
    int dim;
    cout << "Enter the dimension of the matrix: ";
    cin >> dim;
    unsigned int *A = (unsigned int *)malloc(sizeof(unsigned int) * dim * dim);
    unsigned int *B = (unsigned int *)malloc(sizeof(unsigned int) * dim * dim);
    init(dim, A, B);
    int shmid = shmget(0, sizeof(int) * dim * dim, IPC_CREAT | 0666);
    unsigned int *C = (unsigned int *)shmat(shmid, NULL, 0);
    for(int processNum=1;processNum<=16;processNum++){
        struct timeval start, end;
        gettimeofday(&start, NULL);
        for(int p=0;p<processNum;p++){
            pid_t pid = fork();
            if(pid == 0){
                int step = dim / processNum;
                int l = p * step;
                int r = (p == processNum - 1) ? dim : l + step;
                matrixMultiplication(A, B, C, l, r, dim);
                shmdt(C);
                exit(0);
            }
        }
        for(int i=0;i<processNum;i++)
            wait(NULL);
        gettimeofday(&end, NULL);
        int sec = end.tv_sec - start.tv_sec;
        int usec = end.tv_usec - start.tv_usec;
        unsigned int chechSum = calCheckSum(C, dim);
        cout << "Multiplying matrices using " << processNum << " process";
        if(processNum > 1)
            cout << "es";
        cout << endl;
        cout << "Elapsed time: " << sec + (usec / 1000000.0) << "sec, " << "Checksum: " << chechSum << endl;
    }
    shmdt(C);
    shmctl(shmid, IPC_RMID, NULL);
    free(A);
    free(B);
}