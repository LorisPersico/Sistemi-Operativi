#include "../headers/master.h"

int ENERGY_DEMAND;    // Quantità di energia prelevata ogni secondo
int MAX_N_ATOMICO; 
int SIM_DURATION;  // Durata massima della simulazione in secondi
int ENERGY_EXPLODE_THRESHOLD;  // Energia limite per l'esplosione 

int shmid;
int *shared_array;
int semid;
int msg_id_1;

pid_t attivatore_pid;
pid_t atomo_pid;
pid_t alimentatore_pid;

pid_t arrayAtomi[N_ATOMI_INIT];

// Definizione dell'operazione del semaforo
struct sembuf acquire_operation = {0, -1, SEM_UNDO}; //Numero semaforo, operazione da eseguire, flag(annulla operazione se processo termina bruscamente)
struct sembuf release_operation = {0, 1, SEM_UNDO};

// Funzione per generare un numero atomico casuale
char* generate_atomic_number(unsigned int *seed) {
    static char atomic_number_str[10];
    sprintf(atomic_number_str, "%d", rand_r(seed) % MAX_N_ATOMICO); // Usa rand_r() con il seme
    return atomic_number_str;
}

// Funzione per verificare se è stato raggiunto il tempo massimo di simulazione
bool is_simulation_over() {
    static int elapsed_time = 0;
    return (++elapsed_time >= SIM_DURATION);
}

// Funzione per controllare se l'energia liberata supera la soglia di esplosione
bool is_explode_threshold_reached(int energy_released) {
    return (energy_released >= ENERGY_EXPLODE_THRESHOLD);
}

// Funzione per controllare se c'è blackout (prelievo di energia maggiore di quella disponibile)
bool is_blackout(int energy_available, int energy_consumed) {
    return (energy_consumed > energy_available);
}

// Funzione per la pulizia delle risorse IPC
void cleanup() {
    // Elimina la memoria condivisa
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {//id memoria condivisa, operazione da eseguire(rimozione),
        perror("Errore durante la rimozione della memoria condivisa");// puntatore a struttura utilizzata per operazioni di controllo, in questo caso null
    }

    // Elimina il semaforo
    if (semctl(semid, 0, IPC_RMID) == -1) {//id semaforo, indice del semaforo nel set, operazione da eseguire(rimozione)
        perror("Errore durante la rimozione del semaforo");
    }
        // Rimuovere la coda di messaggi
    if (msgctl(msg_id_1, IPC_RMID, NULL) == -1) {//id coda messaggi, operazione da eseguire(rimozione), 
        perror("Errore nella rimozione della coda di messaggi");//puntatore a struttura utilizzata per operazioni di controllo, in questo caso null
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i<N_ATOMI_INIT;  i++)
    {
        kill(arrayAtomi[i], SIGKILL);
    }
    kill(attivatore_pid, SIGKILL);
    kill(alimentatore_pid, SIGKILL);

    // Termina il programma
    exit(EXIT_SUCCESS);
}


// Funzione di gestione del segnale SIGINT
void sigint_handler() {
    printf("\nRicevuto segnale SIGINT. Pulizia delle risorse...\n");
    cleanup();
}
// Funzione di gestione del segnale SIGUSR1
void sigusr1_handler() {
    printf("\nLa simulazione è terminata per fallimento di una fork\n");
    cleanup();
}

void initialize_master(char* scelta)
{
    FILE *file;

    file = fopen(scelta, "r");
    printf("%s",scelta);
    // Verifica se il file è stato aperto correttamente
    if (file == NULL) {
        printf("Impossibile aprire il file.\n");
        exit(EXIT_FAILURE);
    }

    fscanf(file, " %d %d %d %d", &ENERGY_DEMAND, &MAX_N_ATOMICO, &SIM_DURATION, &ENERGY_EXPLODE_THRESHOLD);

    // Chiudi il file
    fclose(file);


    // Registra il gestore per il segnale SIGINT
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("Errore nell'impostare il gestore per SIGINT");
        exit(EXIT_FAILURE);
    }
    
    // Installa il gestore per SIGUSR1
    if (signal(SIGUSR1, sigusr1_handler) == SIG_ERR) {
        perror("Errore nell'impostare il gestore per SIGUSR1");
        exit(EXIT_FAILURE);
    }

    // Ottieni l'ID della memoria condivisa
    shmid = shmget(SHM_KEY, SIZE_STATISTICHE * sizeof(int), IPC_CREAT | 0666);//chiave univoca memoria condivisa, dimensione della memoria condivisa in queto caso 10*dimensione int(4)
    if (shmid == -1) {                                                        //combinazione di flag di controllo, nella creazione(se non esiste viene creata) 
        perror("Errore nella creazione della memoria condivisa");             //in or bit a bit con diritti di utilizzo in questo caso chiunque può eseguire/leggere/modificare.
        exit(EXIT_FAILURE);
    }

    // Attacca la memoria condivisa al processo
    shared_array = (int *)shmat(shmid, NULL, 0); //collega array a memoria condivisa, in questo caso ritorna un array di interi dalla memoria shmid 
    if (shared_array == (int *)(-1)) {           //non viene specificato l'indirizzo in cui collegare il segmento di memoria(perchè null),
        perror("Errore nell'attaccare la memoria condivisa");//nessun flag aggiuntivo da considerare(perchè 0)
        exit(EXIT_FAILURE);
    }

    // Ottieni l'ID del semaforo
    semid = semget(SHM_KEY, 1, IPC_CREAT | 0666); //chiave identificazione insieme semafori, semafori presenti nell'insieme in questo caso 1, classici flag
    if (semid == -1) {
        perror("Errore nella creazione del semaforo");
        exit(EXIT_FAILURE);
    }

    // Inizializza il valore del semaforo
    if (semctl(semid, 0, SETVAL, 1) == -1) { //imposta il valore del primo semaforo di semid, 0=indice semaforo, SETVAL= indica l'operazione di settare il valore, 1=valore da impostare
        perror("Errore nell'inizializzazione del valore del semaforo");
        exit(EXIT_FAILURE);
    }

   // Acquisisci il semaforo
    if (semop(semid, &acquire_operation, 1) == -1) {//id semafori, operazioni da eseguire, eseguita solo 1 operazione da acquire operation
        perror("Errore nell'acquisizione del semaforo");
        exit(EXIT_FAILURE);
    }

    // Inizializza l'array 
    for (int i = 0; i < SIZE_STATISTICHE; i++) {
        shared_array[i] = 0;
    }

    // Rilascia il semaforo
    if (semop(semid, &release_operation, 1) == -1) {//id semafori, operazioni da eseguire, eseguita solo 1 operazione da relase operation
        perror("Errore nel rilascio del semaforo");
        exit(EXIT_FAILURE);
    }
}


int main() {
    int scelta;

    printf("Seleziona configurazione:\n");
    printf("1. timeout\n");
    printf("2. explode\n");
    printf("3. blackout\n");
    printf("4. standard\n");
    printf("Scelta: ");

    scanf("%d", &scelta);

    switch (scelta) {
    case 1:
        initialize_master("configurazioni/confTimeout.txt");
        break;
    case 2:
        initialize_master("configurazioni/confExplode.txt");
        break;
    case 3:
        initialize_master("configurazioni/confBlackout.txt");
        break;
    case 4:
        initialize_master("configurazioni/confWorking.txt");
        break;
    default:
        printf("Scelta non valida. Seleziona un numero tra 1 e 4.\n");
        break;
    }

    printf("\033[2J\033[H");


    pid_t masterPid = getpid();
    char masterPid_str[20];
    snprintf(masterPid_str, sizeof(masterPid_str), "%d", masterPid);

    char *argsAttivatore[] = {"./bin/attivatore", NULL};
    char *argsAlimentatore[] = {"./bin/alimentatore", masterPid_str,NULL};

    // Ottenimento dell'ID della coda di messaggi
    msg_id_1 = msgget(MSG_KEY_1, IPC_CREAT | 0666);
    if (msg_id_1 == -1) {
        perror("Errore nella creazione della coda di messaggi");
        exit(EXIT_FAILURE);
    }
    srand(time(NULL));

    // Creazione dei processi atomi iniziali
    for (int i = 0; i < N_ATOMI_INIT; ++i) {
        pid_t atomo_pid = fork();
        arrayAtomi[i] = atomo_pid;
        if (atomo_pid == -1) {
            perror("Errore nella fork");
            exit(EXIT_FAILURE);
        } else if (atomo_pid == 0) { // Processo figlio (atomo)
            unsigned int seed = time(NULL) ^ getpid(); // Genera un seme unico per ogni processo figlio
            char *argsAtomo[] = {"./bin/atomo", generate_atomic_number(&seed), masterPid_str, NULL}; //null perchè indica fine degli argomenti
            execve(argsAtomo[0], argsAtomo, NULL); //esegue un nuovo processo avente path, altri valori passati tramite argsAtomo, 
                                                   //variabili d'ambiente se null eredita quelle del processo chiamante
            perror("Errore nell'esecuzione di execl");
            exit(EXIT_FAILURE);
        }
    }

    // Creazione del processo attivatore
    pid_t attivatore_pid = fork();
    if (attivatore_pid == -1) {
        perror("Errore nella fork");
        exit(EXIT_FAILURE);
    } else if (attivatore_pid == 0) { // Processo figlio (attivatore)
        execve(argsAttivatore[0], argsAttivatore, NULL);//esegue un nuovo processo avente path, altri valori passati tramite argsAttivatore,
                                                        // variabili d'ambiente se null eredita quelle del processo chiamante
        perror("Errore nell'esecuzione di execl");
        exit(EXIT_FAILURE);
    }

    // Creazione del processo alimentatore
    pid_t alimentatore_pid = fork();
    if (alimentatore_pid == -1) {
        perror("Errore nella fork");
        exit(EXIT_FAILURE);
    } else if (alimentatore_pid == 0) { // Processo figlio (alimentatore)
        execve(argsAlimentatore[0], argsAlimentatore, NULL);
        perror("Errore nell'esecuzione di execl");
        exit(EXIT_FAILURE);
    }

    sleep(1);
    // Avvio della simulazione
    while (true) 
    {
        
        // Acquisisci il semaforo
        if (semop(semid, &acquire_operation, 1) == -1) {
            perror("Errore nell'acquisizione del semaforo");
        }

        if (is_simulation_over()) {
            printf("\nTIMEOUT\n\nLa simulazione è terminata per raggiungimento della durata massima.\n");
            cleanup();
        }

        // Verifica se è stato raggiunto il limite di energia per l'esplosione
        if (is_explode_threshold_reached(shared_array[4])) {
            printf("\nEXPLODE\n\nLa simulazione è terminata per raggiungimento del limite di energia per l'esplosione.\n");
            cleanup();
        }

        // Verifica se c'è blackout (prelievo di energia maggiore di quella disponibile)
        if (is_blackout(shared_array[4] - shared_array[6], ENERGY_DEMAND)) {
            printf("\nBLACKOUT\n\nLa simulazione è terminata per blackout (prelievo di energia maggiore di quella disponibile).\n");
            cleanup();
        }

        // Utilizza l'array in memoria condivisa come desiderato
        printf("\nAttivazioni: %d (tot) %d (sec) \
                \nScissioni: %d (tot) %d (sec) \
                \nEnergia prodotta: %d (tot) %d (sec) \
                \nEnergia consumata: %d (tot) %d (sec) \
                \nScorie prodotte: %d (tot) %d (sec)",
                shared_array[0], shared_array[1], shared_array[2], shared_array[3],
                shared_array[4], shared_array[5], shared_array[6], shared_array[7],
                shared_array[8], shared_array[9]);

        printf(SEPARATORE);
        
        shared_array[1] = shared_array[3] = shared_array[5] = shared_array[7] = shared_array[9] = 0;

        // Rilascia il semaforo
        if (semop(semid, &release_operation, 1) == -1) {
            //perror("Errore nel rilascio del semaforo");
            exit(EXIT_FAILURE);
        }

        // Gestione dei processi figlio terminati
        int status;
        pid_t child_pid;
        while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0) { //specifica quale figlio attendere -1 attende la terminazione di uno qualsiasi, 
                                                                  //puntatore in cui verrà salvato lo stato di uscita, opzioni WHOHANG significa 
                                                                  //eseguita come non bloccante cioè restituisce immediatamente se non ci sono processi figlio terminati(mentre 0 se deve aspettare)
            if (WIFEXITED(status)) { //macro che indica il modo in cui è terminato il processo figlio, se 0 terminato in maniera anormale, magari tramite segnale
                //printf("Processo figlio con PID %d terminato correttamente\n", child_pid);
            } else {
                //printf("Processo figlio con PID %d terminato in modo anomalo\n", child_pid);
            }
        }

        sleep(1);
        
        // Acquisisci il semaforo
        if (semop(semid, &acquire_operation, 1) == -1) {
            perror("Errore nell'acquisizione del semaforo");
            exit(EXIT_FAILURE);
        }
        shared_array[4] = shared_array [4] - ENERGY_DEMAND; //Preleva energia totale
        shared_array[5] = shared_array[5] - ENERGY_DEMAND;  //Preleva energia parziale

        shared_array[6] = shared_array [6] + ENERGY_DEMAND; //Energia Prelevata totale
        shared_array[7] = shared_array[7] + ENERGY_DEMAND;  //Energia Prelevata parziale
                // Rilascia il semaforo
        if (semop(semid, &release_operation, 1) == -1) {
            //perror("Errore nel rilascio del semaforo");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
