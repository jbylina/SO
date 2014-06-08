#include"process.h"

int * common_sig_t;
shm_sem_pkg * pkg;
pthread_t thread;

static void sigHandler(int signo)
{

    if(signo == common_sig_t[0]) // start
    {
        if( semVal(pkg->sem_id , 0)  > 0 )
        {
            semlock(pkg->sem_id , 0);
        }

    }
    else if(signo == common_sig_t[1]) // stop
    {
        if( semVal(pkg->sem_id , 0)  == 0)
        {
            semunlock(pkg->sem_id , 0);
        }

    }
    else if(signo == common_sig_t[2]) // koniec
    {
        semlock(pkg->sem_id, 3); // ustawienie flagi-sygnalu do zabicia procesu nr 1

        if( semVal(pkg->sem_id , 0)  > 0 )
        {
            semlock(pkg->sem_id , 0);
        }
    }
}

static void * mainLoop()
{
    char * shm = pkg->shm_ptr;
    char buffer[BUFFER_SIZE];
    short int readed_bytes;


    while(1)
    {
        semWait(pkg->sem_id, 0); // semafor z semop = 0 jest odpowiedzilny za to czy pracujemy czy nie

        if( !semVal(pkg->sem_id , 3))
        {
            semlock(pkg->sem_id, 4); // ustawienie flagi sygnalu do zabicia procesu nr 2
            semunlock(pkg->sem_id, 2); // wpuszczenie proc 2 do sprawdzenia flagi i zabicia sie

            pthread_cancel(thread);
            pthread_testcancel();
        }

        if( (readed_bytes = read( STDIN_FILENO ,  buffer , BUFFER_SIZE))  > 0)
        {

            semlock(pkg->sem_id, 1);

            if( !semVal(pkg->sem_id , 3))
            {
                if ( semVal(pkg->sem_id , 4))
                    semlock(pkg->sem_id, 4); // ustawienie flagi sygnalu do zabicia procesu nr 2

                semunlock(pkg->sem_id, 2); // wpuszczenie proc 2 do sprawdzenia flagi i zabicia sie
               
                pthread_cancel(thread);
                pthread_testcancel();
            }

            shm[0] = (unsigned char)readed_bytes;

            unsigned short a;

            for(a=0; a < readed_bytes ; a++)
                shm[a+1] = buffer[a];

            semunlock(pkg->sem_id, 2);

        }
        else if( readed_bytes == -1)
        {
            fatalError(READ_ERR);
        }
        else
        {
            break; // zamykanie programu bo koniec wejscia
        }
    }
    pthread_exit(EXIT_SUCCESS);
}

void startProcess1( char *nazwa , common_sig_struct * sig_struct , shm_sem_pkg * pkg12 )
{
    common_sig_t = sig_struct->sig_t;
    pkg = pkg12;

    strcpy(nazwa,"Proc 1   " );

    // Rejestracja wspolnych sygnalow
    if( signalReg( common_sig_t , sig_struct->sig_t_size ,sigHandler ) != SUCCESS)
        fatalError(SIG_REG_ERR);


    if ( pthread_create(&thread, NULL, (void *)mainLoop, NULL) )
        fatalError(T_CREATION_ERR);
    pthread_join(thread , NULL);


    // zwalnianie zasobow i "zamykanie" niedomknietych spraw :P
    if( signalDeReg( common_sig_t , sig_struct->sig_t_size ) != SUCCESS)
        fatalError(SIG_DEREG_ERR);
}
