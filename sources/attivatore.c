#include "../headers/attivatore.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


#define printf(...) 

int shmid;
int *shared_array;
int semid;

// Definizione dell'operazione del semaforo
struct sembuf acquire_operation = {0, -1, SEM_UNDO};//Numero semaforo, operazione da eseguire, flag(annulla operazione se processo termina bruscamente)
struct sembuf release_operation = {0, 1, SEM_UNDO};


void initialize_shm() {
    // Ottieni l'ID della memoria condivisa
    shmid = shmget(SHM_KEY, SIZE_STATISTICHE * sizeof(int), IPC_CREAT | 0666);//chiave univoca memoria condivisa, dimensione della memoria condivisa in queto caso 10*dimensione int(4)
    if (shmid == -1) {                                              //combinazione di flag di controllo, nella creazione(se non esiste viene creata) 
        perror("Errore nella creazione della memoria condivisa");//in or bit a bit con diritti di utilizzo in questo caso chiunque può eseguire/leggere/modificare.
        exit(EXIT_FAILURE);
    }

    // Attacca la memoria condivisa al processo
    shared_array = (int *)shmat(shmid, NULL, 0);//collega array a memoria condivisa, in questo caso ritorna un array di interi dalla memoria shmid
    if (shared_array == (int *)(-1)) {          //non viene specificato l'indirizzo in cui collegare il segmento di memoria(perchè null),
                                                //nessun flag aggiuntivo da considerare(perchè 0)
        perror("Errore nell'attaccare la memoria condivisa");
        exit(EXIT_FAILURE);
    }

    // Ottieni l'ID del semaforo
    semid = semget(SHM_KEY, 1, IPC_CREAT | 0666);//chiave identificazione insieme semafori, semafori presenti nell'insieme in questo caso 1, classici flag
    if (semid == -1) {
        perror("Errore nella creazione del semaforo");
        exit(EXIT_FAILURE);
    }

    // Inizializza il valore del semaforo
    if (semctl(semid, 0, SETVAL, 1) == -1) {                           //imposta il valore del primo semaforo di semid, 0=indice semaforo,
        perror("Errore nell'inizializzazione del valore del semaforo");//SETVAL= indica l'operazione di settare il valore, 1=valore da impostare
        exit(EXIT_FAILURE);
    }
}

int main() {

    struct timespec req, rem;  // Nanosecondi richiesti e nanosecondi rimasti da aspettare per la nanosleep
    req.tv_sec = STEP_ATTIVATORE_SEC;
    req.tv_nsec = STEP_ATTIVATORE_NSEC; 
    
    initialize_shm();

    // Ottenimento dell'ID della coda di messaggi
    int msg_id_1 = msgget(MSG_KEY_1, IPC_CREAT | 0666);
    if (msg_id_1 == -1) {
        perror("Errore nella creazione della coda di messaggi");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Ottenimento delle informazioni sulla coda di messaggi
        struct msqid_ds queue_info;
        if (msgctl(msg_id_1, IPC_STAT, &queue_info) == -1) {                       //identificatore coda di messaggi, comando da eseguire 
            perror("Errore nel recupero delle informazioni sulla coda di messaggi");//IPC_STAT indica che vuole ottenere informazioni dalla coda, 
            exit(EXIT_FAILURE);                                                    //puntatore alla struttura in cui vengono salvate le informazioni
        }

        printf("\nCoda: %d\n",queue_info.msg_qnum);

        if (queue_info.msg_qnum != 0) {
            // Ricevere il messaggio corrispondente all'indice selezionato
            struct msg_buffer message;
            if ((msgrcv(msg_id_1, &message, sizeof(message.pid), 1, IPC_NOWAIT) == -1)) {//ID, puntatore dove verrà salvato messaggio, dimensione,
                //perror("Errore nella ricezione del messaggio dalla coda di messaggi"); || tipo di messaggio che si vuole ricevere, IPC_NOWAIT flag che infica che non bloccante 
                printf("Tipo del messaggio ricevuto: %d\n", message.msg_type);
                //exit(EXIT_FAILURE);
            }
            else {
                pid_t selected_pid = message.pid;

                //coda messsaggi type=pid
                message.msg_type = selected_pid;
                message.pid = selected_pid;

                if (msgsnd(msg_id_1, &message, sizeof(message.pid), 0) == -1) {//id coda di messaggi, puntatore in cui si trova messaggio da inviare, 
                                                                               //size del messaggio, flag del messaggio
                                                                               //0 significa che l'invio del messaggio è bloccante, quindi il processo bloccato finchè non viene inviato
                    perror("Errore nell'invio del PID all'Atomo per la scissione");
                }
                else {
                    // Acquisisci il semaforo
                    if (semop(semid, &acquire_operation, 1) == -1) {//id semafori, operazioni da eseguire, eseguita solo 1 operazione da acquire operation
                        perror("Errore nell'acquisizione del semaforo");
                    }

                    // Utilizza l'array in memoria condivisa come desiderato
                    shared_array[0] = shared_array [0]+1;
                    shared_array[1] = shared_array[1]+1;

                    // Rilascia il semaforo
                    if (semop(semid, &release_operation, 1) == -1) {//id semafori, operazioni da eseguire, eseguita solo 1 operazione da relase operation
                        perror("Errore nel rilascio del semaforo");
                    }
                }
            }
        }
        else {
            printf("Attivatore: Nessun PID disponibile nella coda di messaggi.\n");
        }

        nanosleep(&req, &rem);
    
    }

    return 0;
}
