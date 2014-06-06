#include"process.h"

static int * common_sig_t;
static int * communication_sig_t;
shm_sem_pkg * pkg;
pthread_t thread;


static void communicationSigHandler(int signo)
{
    if(signo == communication_sig_t[0]) // start
    {

        if( semVal(pkg->sem_id , 0) > 0 )
        {
            semlock(pkg->sem_id , 0);
        }

    }
    else if(signo == communication_sig_t[1]) // stop
    {
        if( semVal(pkg->sem_id , 0)  == 0)
        {
            semunlock(pkg->sem_id , 0);
        }

    }
    else if(signo == communication_sig_t[2]) // koniec
    {


    }
}

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
        if( semVal(pkg->sem_id , 0)   == 0)
        {
            semunlock(pkg->sem_id , 0);
        }

    }
    else if(signo == common_sig_t[2]) // koniec
    {

        semlock(pkg->sem_id, 4); // ustawienie flagi sygnalu do zabicia procesu nr 1

        if( semVal(pkg->sem_id , 0)  > 0 )
        {
            semlock(pkg->sem_id , 0);
        }
        // poinformowanie procesu 3
    }

}

static void * mainLoop(void * write_pipe_fd )
{
    char * shm = pkg->shm_ptr;
    int * fd = (int *)write_pipe_fd;

    unsigned char readed_bytes;

    while(1)
    {
        semlock(pkg->sem_id, 2);

        if( !semVal(pkg->sem_id , 4))
        {
            if( semVal(pkg->sem_id, 3) ) // ustawienie flagi sygnalu do zabicia procesu nr 1
            {
                semlock(pkg->sem_id, 3);
            }

            if( semVal(pkg->sem_id , 0) > 0 )
            {
                semlock(pkg->sem_id , 0); // jbc odblokowanie jesli dostalismy kill podczas stopu
            }

            semunlock(pkg->sem_id, 1); // wpuszczenie proc 1 do sprawdzenia flagi i zabicia sie

            pthread_cancel(thread);
            pthread_testcancel();
        }

        readed_bytes = (unsigned char)shm[0];
        write(*fd, &shm[1] ,readed_bytes);
        semunlock(pkg->sem_id , 1);
    }

    pthread_exit(EXIT_SUCCESS);
}


void startProcess2( char *nazwa, common_sig_struct * sig_struct , shm_sem_pkg * pkg12 , pipe_sig_pkg * pkg23 )
{
    common_sig_t = sig_struct->sig_t;
    communication_sig_t = pkg23->sig_tab;
    close(pkg23->pipe_fd[0]); // zamkniecie nieuzywanego deskryptora czytania
    pkg = pkg12;

    strcpy(nazwa,"Proc 2   ");

    // Rejestracja sygnalow
    if( signalReg( common_sig_t , sig_struct->sig_t_size ,sigHandler ) != SUCCESS)
        fatalError(SIG_REG_ERR);

    // Rejestracja sygnalow dodatkowych
    if( signalReg( communication_sig_t , pkg23->sig_tab_size ,communicationSigHandler ) != SUCCESS)
        fatalError(SIG_REG_ERR);

    if ( pthread_create(&thread, NULL, (void *)mainLoop, (void *)&pkg23->pipe_fd[1] ) )
        fatalError(T_CREATION_ERR);
    pthread_join(thread , NULL);

    fprintf(stderr, "watek 2 zakonczony \n");

    // zwalnianie zasobow i "zamykanie" niedomknietych spraw :P

    if( signalDeReg( common_sig_t , sig_struct->sig_t_size ) != SUCCESS)
        fatalError(SIG_DEREG_ERR);

    if( signalDeReg( communication_sig_t , pkg23->sig_tab_size ) != SUCCESS)
        fatalError(SIG_DEREG_ERR);

    close(pkg23->pipe_fd[1]);
}
