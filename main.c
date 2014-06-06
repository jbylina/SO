#include"process.h"
/*
Semafor Sygnały Pam.Dzielona PIPE
wszystkie obsluguja 3 sygnaly stop , start , koniec
*/


static void startMainProcess()
{

    while(1)
    {
        sleep(1);
    }

}

int main(int argc, char *argv[])
{

    // tablic wspolnych sygnalow do komunikacji
    int communication_sig_table[3] = { SIGUSR1 , SIGUSR2 , SIGABRT };

    const int sem_count = 5;
    int common_signals[3] = { SIGFPE , SIGTSTP , SIGBUS };
    common_sig_struct common_signals_struct =
    {
        .sig_t = common_signals,
        .sig_t_size = SIZE_OF_T(common_signals)
    };

    // tworzenie pamieci wspoldzielonej
    int shm_id;
    key_t shm_key;
    char *shm_ptr;

    if ((shm_key = ftok(".", 'L')) == -1)
        fatalError(SHM_KEY_CREATION_ERR);

    if ((shm_id = shmget(shm_key, BUFFER_SIZE_WITH_HEADER , IPC_CREAT | 0666)) < 0)
        fatalError(SHM_SEG_CREATION_ERR);


    if ((shm_ptr = shmat(shm_id, NULL, 0)) == (char *) -1)
        fatalError(SHM_ADD_ERR);

    //towrzenie semaforow
    key_t sem_key;
    int   sem_id;
    union semun sem_ctl;

    if ((sem_key = ftok("/etc", 'S')) == -1)
        fatalError(SEM_KEY_CREATION_ERR);

    if ((sem_id = semget(sem_key, sem_count, IPC_CREAT | 0666)) < 0 )
        fatalError(SEM_CREATION_ERR);

    sem_ctl.val = 1; // pdniesienie semafora ( 0 - nie przechodzi dalej  1 - przechozdi dalej )
    int a;
    for( a = 0; a < sem_count; a++  )
        if (semctl(sem_id, a, SETVAL, sem_ctl) == -1)
            fatalError(SEM_SET_ERR);

    // semafor procesu 2 zalaczony na 0 aby nie czytal dopuki proces 1 nie wpisze
    sem_ctl.val = 0;
    semctl(sem_id, 2, SETVAL, sem_ctl);


    // semafor procesu 1 ktory stopuje przesylanie dalej
    sem_ctl.val = 0;
    semctl(sem_id, 0, SETVAL, sem_ctl);



    // torzenie paczki dla procesu jeden i dwa
    shm_sem_pkg package12;
    package12.sem_id = sem_id;
    package12.sem_id_size = sem_count;
    package12.shm_ptr = shm_ptr;
    package12.shm_size = BUFFER_SIZE_WITH_HEADER;

    // tworzenie pipe-a
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1)
        fatalError(PIPE_CREATION_ERR);

    // tworzenie paczki dla procesu 2 i 3
    pipe_sig_pkg package23;
    package23.pipe_fd = pipe_fd;
    package23.sig_tab = communication_sig_table;
    package23.sig_tab_size = SIZE_OF_T(communication_sig_table);


    pid_t pid = fork();
    if(pid == -1)  //błąd
        fatalError(P_CREATION_ERR);
    else if(pid > 0) // rodzic
    {
        pid = fork();
        if(pid == -1)  //błąd
            fatalError(P_CREATION_ERR);
        else if(pid > 0) // rodzic
        {
            pid_t pid_p2 = pid;
            pid = fork();
            if(pid == -1)  //błąd
                fatalError(P_CREATION_ERR);
            else if (pid > 0 )
                startMainProcess();
            else  // dziecko - PROCES 3
                startProcess3( argv[0] , pid_p2 , &common_signals_struct , &package23 );
        }
        else   // dziecko - PROCES 2
            startProcess2( argv[0] , &common_signals_struct,  &package12 , &package23 );
    }
    else        // dziecko - PROCES 1
        startProcess1(  argv[0],  &common_signals_struct , &package12 );


    return 0;
}
