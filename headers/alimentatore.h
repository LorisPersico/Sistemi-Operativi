
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#define MAX_N_ATOMICO 100
#define STEP_ALIMENTATORE_SEC 0
#define STEP_ALIMENTATORE_NSEC 500000000 // 0.5 sec
#define N_NUOVI_ATOMI 2

// Prototipi delle funzioni
char* generate_atomic_number(unsigned int *seed);

