
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>

#define MSG_KEY_1 1234
#define MIN_N_ATOMICO 5
#define SHM_KEY 4444
#define SIZE_STATISTICHE 10

struct atomo {
    pid_t pid;
    int n_atom;
};

// Funzione per generare un numero atomico casuale
int generate_atomic_number(int nAtomoP);

// Funzione per la scissione dell'atomo
void scissione();

// Funzione per gestire i segnali
void handle_signal(int signal);

// Funzione per inizializzare la memoria condivisa
extern void initialize_shm();

