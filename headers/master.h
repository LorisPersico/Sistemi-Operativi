#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

#define MSG_KEY_1 1234 
#define N_ATOMI_INIT 5     // Numero iniziale di atomi
#define SHM_KEY 4444
#define SIZE_STATISTICHE 10
#define SEPARATORE "\n--------------------------------"

extern int shmid;
extern int *shared_array;
extern int semid;
extern int msg_id_1;

extern pid_t attivatore_pid;
extern pid_t atomo_pid;
extern pid_t alimentatore_pid;

extern pid_t arrayAtomi[N_ATOMI_INIT];

// Definizione dell'operazione del semaforo
extern struct sembuf acquire_operation;
extern struct sembuf release_operation;

// Funzione per generare un numero atomico casuale
char* generate_atomic_number(unsigned int *seed);

// Funzione per verificare se è stato raggiunto il tempo massimo di simulazione
bool is_simulation_over();

// Funzione per controllare se l'energia liberata supera la soglia di esplosione
bool is_explode_threshold_reached(int energy_released);

// Funzione per controllare se c'è blackout (prelievo di energia maggiore di quella disponibile)
bool is_blackout(int energy_available, int energy_consumed);

// Funzione per la pulizia delle risorse IPC
void cleanup();

// Funzione di gestione del segnale SIGINT
void sigint_handler();

// Funzione di gestione del segnale SIGUSR1
void sigusr1_handler();

void initialize_master();
