#include"process.h"

void fatalError(enum Message type)
{
    fprintf(stderr, "KOD BLEDU: %d\n" , errno);
    switch(type)
    {
    case P_CREATION_ERR:
        errx(1, "Blad P_CREATION_ERR! " );
    case SHM_KEY_CREATION_ERR:
        errx(2, "Blad SHM_KEY_CREATION_ERR!");
    case SHM_SEG_CREATION_ERR:
        errx(3, "Blad SHM_SEG_CREATION_ERR!");
    case SHM_ADD_ERR:
        errx(4, "Blad SHM_ADD_ERR!");
    case SEM_CREATION_ERR:
        errx(5, "Blad SEM_CREATION_ERR!");
    case SEM_KEY_CREATION_ERR:
        errx(6, "Blad SEM_KEY_CREATION_ERR!");
    case SEM_SET_ERR:
        errx(7, "Blad SEM_SET_ERR!");
    case PIPE_CREATION_ERR:
        errx(8, "Blad PIPE_CREATION_ERR!");
    case READ_ERR:
        errx(9, "Blad READ_ERR!");
    case T_CREATION_ERR:
        errx(10, "Blad READ_ERR!");
    default:
        errx( -1, "Nieznany blad");
    }
}


Message signalReg(int tab[] , size_t size , __sighandler_t handler)
{
    enum Message err = SUCCESS;
    size_t a;
    for(a = 0; a < size ; a++)
    {
        if( signal(tab[a] , handler) == SIG_ERR)
        {
            err = SIG_REG_ERR;
            break;
        }
    }
    return err;
}

Message signalDeReg(int tab[] , size_t size )
{
    enum Message err = SUCCESS;
    size_t a;
    for(a = 0; a < size ; a++)
    {
        if( signal(tab[a] , SIG_IGN) == SIG_ERR)
        {
            err = SIG_REG_ERR;
            break;
        }
    }
    return err;
}


int semWait( int semid, unsigned short in_sem_num )
{
    struct sembuf opr;

    opr.sem_num =  in_sem_num;
    opr.sem_op 	= 0;        // czekanie
    opr.sem_flg =  0;        // operacja blokujaca

    if (semop(semid, &opr, 1) == -1)
    {
        warn("Blad blokowania semafora!");
        return 0;
    }
    else
    {
        return 1;
    }

}


int semlock(int semid, unsigned short in_sem_num)
{
    struct sembuf opr;

    opr.sem_num =  in_sem_num;
    opr.sem_op 	= -1;        // blokowanie
    opr.sem_flg =  0;        // operacja blokujaca

    if (semop(semid, &opr, 1) == -1)
    {
        warn("Blad blokowania semafora nr %d!" ,in_sem_num );
        return 0;
    }
    else
    {
        return 1;
    }
}

int semunlock(int semid, unsigned short in_sem_num)
{
    struct sembuf opr;

    /*
     	struct sembuf {
        	ushort semnum;
        	short sem_op;
        	ushort sem_flg;
    	};
    *
    * gdzie:
    * 	semnum - numer semafora,
    * 	sem_op - operacja na semaforze:
    * 		sem_op > 0 - (V) zwiekszenie semafora o wartosc "sem_op"
    * 		sem_op < 0 - (P) polozenie semafora (wstrzymanie procesu)
    * 					 lub zmniejszenie semafora o wartosc "sem_op"
    * 		sem_op = 0 - (Z) "przejscie pod semaforem", odwrotnosc (P)
    */

    opr.sem_num = in_sem_num;
    opr.sem_op 	= 1;
    opr.sem_flg = 0;

    if (semop(semid, &opr, 1) == -1)
    {
        warn("Blad odblokowania semafora nr %d!" ,in_sem_num );
        return 0;
    }
    else
    {
        return 1;
    }
}

int semVal(int semid, unsigned short in_sem_num)
{
    return semctl(semid,in_sem_num,GETVAL);
}

