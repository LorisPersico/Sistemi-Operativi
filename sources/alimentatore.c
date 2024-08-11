#include "../headers/alimentatore.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

pid_t masterPid;

char* generate_atomic_number(unsigned int *seed) {
    static char atomic_number_str[10];
    sprintf(atomic_number_str, "%d", rand_r(seed) % MAX_N_ATOMICO); // Usa rand_r() con il seme
    return atomic_number_str;
}

int main(int, char *argv[]) 
{    
    srand(time(NULL));
    struct timespec req, rem;
    req.tv_sec = STEP_ALIMENTATORE_SEC;
    req.tv_nsec = STEP_ALIMENTATORE_NSEC;     
    masterPid = atoi(argv[1]);

    char alimPid_str[20];
    snprintf(alimPid_str, sizeof(alimPid_str), "%d", getpid());

    while(1)
    {
        // Aggiunta di 3 atomi alla simulazione ogni STEP_ATTIVATORE
        nanosleep(&req, &rem);

        for (int i = 0; i < N_NUOVI_ATOMI; ++i) 
        {
            pid_t pid = fork();
            if (pid == -1) {
                perror("Errore nella fork");
                kill(masterPid, SIGUSR1);
            } else if (pid == 0) { // Processo figlio (atomo)
                unsigned int seed = time(NULL) ^ getpid(); // Genera un seme unico per ogni processo figlio
                char *args[] = {"./bin/atomo", generate_atomic_number(&seed), alimPid_str, NULL};
                printf("ALIMENTATORE AGGIUNTO NUOVO ATOMOOO");
                execve(args[0], args, NULL); //path, argomenti passati, variabii d'ambiente
                perror("Errore nell'esecuzione di execl");
                kill(masterPid, SIGUSR1);
            }
        }
                // Gestione dei processi figlio terminati
        int status;
        pid_t child_pid;
        while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0) {
            if (WIFEXITED(status)) {//macro che indica il modo in cui è terminato il processo figlio, se 0 terminato in maniera anormale, magari tramite segnale
                //printf("Processo figlio con PID %d terminato correttamente\n", child_pid);
            } else {
                //printf("Processo figlio con PID %d terminato in modo anomalo\n", child_pid);
            }
        }
    }

}
