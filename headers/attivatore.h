
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <time.h>

#define MSG_KEY_1 1234
#define STEP_ATTIVATORE_SEC 0
#define STEP_ATTIVATORE_NSEC 5000000
#define SHM_KEY 4444
#define SIZE_STATISTICHE 10

// Definizione della struttura del messaggio
struct msg_buffer {
    long msg_type; // Utilizzeremo il PID come msg_type
    pid_t pid;
};

// Funzione per inizializzare la memoria condivisa
extern void initialize_shm();

