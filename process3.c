#include"process.h"

static int * common_sig_t;
static int * communication_sig_t;
static pid_t pid_p2;
static pthread_t thread;

static void sigHandler(int signo)
{

    if(signo == common_sig_t[0]) // start
    {
        kill(pid_p2 , communication_sig_t[0]);
    }
    else if(signo == common_sig_t[1]) // stop
    {
        kill(pid_p2 , communication_sig_t[1]);
    }
    else if(signo == common_sig_t[2]) // koniec
    {
        kill(pid_p2 , communication_sig_t[2]);

    }
}

static void communicationSigHandler(int signo)
{
    if(signo == communication_sig_t[0]) // start
    {

    }
    else if(signo == communication_sig_t[1]) // stop
    {

    }
    else if(signo == communication_sig_t[2]) // koniec
    {
        pthread_cancel(thread);
    }
}

static void * mainLoop(void * read_pipe_fd )
{
    int * fd = (int *)read_pipe_fd;
    unsigned char buffer[BUFFER_SIZE];
    unsigned char readed_bytes;

    while(1)
    {

        if((readed_bytes = read(*fd , &buffer , BUFFER_SIZE )) > 0)
        {
            pthread_testcancel();
            unsigned short a;
            for( a =0 ; a < readed_bytes ; a++)
                //fprintf(stderr, "%x", buffer[a]);
                printf("%x", buffer[a]);
        }

    }
    pthread_exit(EXIT_SUCCESS);
}

void startProcess3( char *nazwa , common_sig_struct * sig_struct , pipe_sig_pkg * pkg23 )
{
    common_sig_t = sig_struct->sig_t;
    communication_sig_t = pkg23->sig_tab;

    if(pkg23->pid)
        pid_p2 = pkg23->pid;
    else if( read(pkg23->pid_pipe_r_fd , &pid_p2 , sizeof( pid_t )) <=0 )
        fatalError(PID_PIPE_ERR);

    strcpy(nazwa,"Proc 3   " );

    // Rejestracja sygnalow
    if( signalReg( common_sig_t , sig_struct->sig_t_size ,sigHandler ) != SUCCESS)
        fatalError(SIG_REG_ERR);

    // Rejestracja sygnalow dodatkowych
    if( signalReg( communication_sig_t , pkg23->sig_tab_size ,communicationSigHandler ) != SUCCESS)
        fatalError(SIG_REG_ERR);


    if ( pthread_create(&thread, NULL, (void *)mainLoop, (void *)&pkg23->pipe_fd[0] ) )
        fatalError(T_CREATION_ERR);
    pthread_join(thread , NULL);

    // zwalnianie zasobow i "zamykanie" niedomknietych spraw :P

    if( signalDeReg( common_sig_t , sig_struct->sig_t_size ) != SUCCESS)
        fatalError(SIG_DEREG_ERR);

    if( signalDeReg( communication_sig_t , pkg23->sig_tab_size ) != SUCCESS)
        fatalError(SIG_DEREG_ERR);

    fprintf(stderr, "proces 3 zakonczony \n");
}
