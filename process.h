#ifndef PROCESS_H_INCLUDED
#define PROCESS_H_INCLUDED

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<err.h>
#include<fcntl.h>
#include<pthread.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/prctl.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>

#define SIZE_OF_T(x) (sizeof(x)/sizeof(x[0]))
#define BUFFER_SIZE   1000 * sizeof(char) // bufor nie moze byc wiekszy niz 255
#define BUFFER_SIZE_WITH_HEADER  (BUFFER_SIZE + sizeof(unsigned char))

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* Jest zdefiniowane w sys/sem.h */
#else
union semun
{
    int val;						// wartosc dla SETVAL
    struct semid_ds *buf;			// bufor dla IPC_STAT, IPC_SET
    unsigned short int *array;		// tablica dla GETALL, SETALL
    struct seminfo *__buf;			// bufor dla IPC_INFO
};
#endif

typedef struct shm_sem_pkg
{
    int sem_id;
    size_t sem_id_size;
    char * shm_ptr;
    size_t shm_size;
} shm_sem_pkg;

typedef struct pipe_sig_pkg
{
    int * pipe_fd;
    int * sig_tab;
    size_t sig_tab_size;
} pipe_sig_pkg;

typedef struct common_sig_struct
{
    int * sig_t;
    size_t sig_t_size;
} common_sig_struct;

typedef enum Message { SUCCESS,

                       P_CREATION_ERR,

                       SIG_REG_ERR,
                       SIG_DEREG_ERR,

                       SHM_KEY_CREATION_ERR,
                       SHM_SEG_CREATION_ERR,
                       SHM_ADD_ERR,

                       SEM_CREATION_ERR,
                       SEM_KEY_CREATION_ERR,
                       SEM_SET_ERR,

                       PIPE_CREATION_ERR,

                       READ_ERR,

                       T_CREATION_ERR,

                     } Message;


void startProcess1();
void startProcess2();
void startProcess3();
void fatalError(enum Message type);
Message signalReg(int tab[] , size_t size , __sighandler_t handler);
Message signalDeReg(int tab[] , size_t size );

int semlock(int semid , unsigned short in_sem_num);
int semunlock(int semid , unsigned short in_sem_num);
int semWait( int semid, unsigned short in_sem_num );
int semVal(int semid, unsigned short in_sem_num);

#endif // PROCESS_H_INCLUDED
