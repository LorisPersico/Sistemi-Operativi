#include "../headers/atomo.h"

#define printf(...)

int energia = 0;
int shmid;
int *shared_array;
int semid;
pid_t masterPid;

// Definizione dell'operazione del semaforo
struct sembuf acquire_operation = {0, -1, SEM_UNDO}; //Numero semaforo, operazione da eseguire, flag(annulla operazione se processo termina bruscamente)
struct sembuf release_operation = {0, 1, SEM_UNDO};

struct atomo atomop;

// Definizione della struttura del messaggio
struct msg_buffer {
    long msg_type; // Utilizzeremo il PID come msg_type
    pid_t pid;
};

// Funzione per generare un numero atomico casuale
int generate_atomic_number(int nAtomoP) {
    srand(time(NULL));
    return (rand() % nAtomoP);
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

int energy(int n1, int n2) {
    return (n1 * n2) - max(n1, n2);
}

// Funzione per la scissione dell'atomo
void scissione() {
    // Esegui la logica per la scissione
    printf("Atomo:%d con numero atomico:%d esegue la scissione.\n", atomop.pid, atomop.n_atom);
    // Processo padre genera numero atomico per il processo figlio
    int atomic_number_child = generate_atomic_number(atomop.n_atom);
    printf("Atomo:%d Numero atomico figlio: %d\n", atomop.pid, atomic_number_child);
    atomop.n_atom = atomop.n_atom - atomic_number_child;
    printf("Atomo:%d Numero atomico aggiornato: %d\n", atomop.pid, atomop.n_atom);

    energia = energy(atomop.n_atom, atomic_number_child);

     // Acquisisci il semaforo
    if (semop(semid, &acquire_operation, 1) == -1) {//id semafori, operazioni da eseguire, eseguita solo 1 operazione da acquire operation
        //perror("Errore nell'acquisizione del semaforo");
        exit(EXIT_FAILURE);
    }

    // Utilizza l'array in memoria condivisa come desiderato
    shared_array[2] = shared_array [2]+1; //Nuova scissione
    shared_array[3] = shared_array[3]+1;

    shared_array[4] = shared_array [4]+energia; //Energia liberata
    shared_array[5] = shared_array[5]+energia;

    // Rilascia il semaforo
    if (semop(semid, &release_operation, 1) == -1) {//id semafori, operazioni da eseguire, eseguita solo 1 operazione da release operation
        //perror("Errore nel rilascio del semaforo");
        exit(EXIT_FAILURE);
    }

    pid_t child_pid = fork();

    if (child_pid == -1) {
        kill(masterPid, SIGUSR1);
    } else if (child_pid == 0) { // Processo figlio (atomo)
        char atomic_number_str[10];
        snprintf(atomic_number_str, sizeof(atomic_number_str), "%d", atomic_number_child);
        execl("./bin/atomo", "atomo", atomic_number_str, NULL);//path, argomenti successivi da passare terminata da NULL
        kill(masterPid, SIGUSR1);
    }

    // Gestione dei processi figlio terminati
    int status;
    pid_t terminated_pid;
    while ((terminated_pid = waitpid(-1, &status, WNOHANG)) > 0) {//specifica quale figlio attendere -1 attende la terminazione di uno qualsiasi, 
                                                                  //puntatore in cui verrà salvato lo stato di uscita, opzioni WHOHANG significa 
                                                                  //eseguita come non bloccante cioè restituisce immediatamente se non ci sono processi figlio terminati(mentre 0 se deve aspettare)
        printf("Il processo figlio con PID %d è terminato\n", terminated_pid);
        // Aggiorna lo stato del processo figlio terminato, se necessario
    }
}

// Funzione per gestire i segnali
void handle_signal(int signal) {
    if (signal == SIGCHLD) {
        // Gestione del processo figlio terminato
        int status;
        pid_t terminated_pid;
        while ((terminated_pid = waitpid(-1, &status, WNOHANG)) > 0) {
            printf("Il processo figlio con PID %d è terminato\n", terminated_pid);
            // Aggiorna lo stato del processo figlio terminato, se necessario
        }
    }
}

void initialize_shm() {
    // Ottieni l'ID della memoria condivisa
    shmid = shmget(SHM_KEY, SIZE_STATISTICHE * sizeof(int), IPC_CREAT | 0666); //chiave univoca memoria condivisa, dimensione della memoria condivisa in queto caso 10*dimensione int(4)
    if (shmid == -1) {                                           //combinazione di flag di controllo, nella creazione(se non esiste viene creata) 
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

int main(int, char *argv[]) {

    initialize_shm();

    // Inizializzazione dell'atomo
    pid_t pid = getpid();
    atomop.pid = pid;
    atomop.n_atom = atoi(argv[1]);
    masterPid = atoi(argv[2]);

    printf("\nNumero atomico del padre ricevuto %d\n", atomop.n_atom);
    //signal(SIGCHLD, handle_signal);

    // Ottenimento dell'ID della coda di messaggi
    int msg_id_1 = msgget(MSG_KEY_1, IPC_CREAT | 0666);
    if (msg_id_1 == -1) {
        perror("Errore nella creazione della coda di messaggi");
        exit(EXIT_FAILURE);
    }

    // Invio del PID all'Attivatore tramite coda di messaggi
    struct {
        long msg_type;
        pid_t pid;
    } msg;

    msg.msg_type = 1; // Utilizziamo 1 come tipo di messaggio
    msg.pid = pid;

    if (msgsnd(msg_id_1, &msg, sizeof(msg.pid), 0) == -1) {//id coda di messaggi, puntatore in cui si trova messaggio da inviare, size del messaggio, flag del messaggio
                                                           //0 significa che l'invio del messaggio è bloccante, quindi il processo bloccato finchè non viene inviato
        perror("Errore nell'invio del PID all'Attivatore");
        exit(EXIT_FAILURE);
    }

    while (1) {

        // Attendi l'arrivo di segnali
        if (msgrcv(msg_id_1, &msg, sizeof(msg.msg_type), pid, 0) == -1) {//id coda di messaggi, puntatore in cui verrà salvato messaggio, size del messaggio flag del messaggio
                                                                         //tipo di messaggio da voler ricevere, flag 0 perchè bloccante
            //perror("Errore nella ricezione del messaggio dalla coda di messaggi");
        } else {
            if (atomop.n_atom <= MIN_N_ATOMICO) {
                printf("Atomo: Numero atomico minore o uguale al minimo consentito. Terminazione.\n");

                // Acquisisci il semaforo
                if (semop(semid, &acquire_operation, 1) == -1) {//id semafori, operazioni da eseguire, eseguita solo 1 operazione da acquire operation
                    perror("Errore nell'acquisizione del semaforo");
                    exit(EXIT_FAILURE);
                }

                shared_array[8] = shared_array [8]+1; //Nuova scoria
                shared_array[9] = shared_array[9]+1;

                // Rilascia il semaforo
                if (semop(semid, &release_operation, 1) == -1) {//id semafori, operazioni da eseguire, eseguita solo 1 operazione da relase operation
                    perror("Errore nel rilascio del semaforo");
                    exit(EXIT_FAILURE);
                }

                exit(EXIT_SUCCESS); 
            } else {
                msg.msg_type = 1; // Utilizziamo 1 come tipo di messaggio
                msg.pid = pid;

                if (msgsnd(msg_id_1, &msg, sizeof(msg.pid), 0) == -1) {//id coda di messaggi, puntatore in cui si trova messaggio da inviare, size del messaggio, flag del messaggio
                                                           //0 significa che l'invio del messaggio è bloccante, quindi il processo bloccato finchè non viene inviato
                    perror("Errore nell'invio del PID all'Attivatore");
                    exit(EXIT_FAILURE);
                }
                scissione();
            }
        }
    }
}
