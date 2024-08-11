Progetto SO 2023/24 Reazione a catena
Descrizione<a name="_page2_x56.69_y251.43"></a> del progetto: versione minima (voto max 24 su 30)

Si intende simulare una reazione a catena. A tal fine sono presenti i seguenti processi:

- un processo master che gestisce la simulazione e mantiene delle statistiche
- processi atomo che si scindono in altri processi atomo, generando energia
- un processo attivatore
- un processo alimentazione che aggiunge nuovi atomi
1. Processo<a name="_page2_x56.69_y384.83"></a> master

Il processo master

- gestisce la simulazione, inizializza le strutture, crea processi figlio
- ogni secondo:
- stampa lo stato corrente della simulazione e le statistiche
- preleva una quantit`a ENERGYDEMANDdi energia.![](Aspose.Words.86582a17-653f-49cf-a560-d4af286c4533.001.png)

All’inizio della simulazione, il processo master

- crea NATOMIINIT processi atomo![](Aspose.Words.86582a17-653f-49cf-a560-d4af286c4533.002.png)![](Aspose.Words.86582a17-653f-49cf-a560-d4af286c4533.003.png)
- crea il processo attivatore
- crea il processo alimentazione
- avvia la simulazione. La simulazione dovr`a partire solamente quando tutti i processi sopra citati sono stati creati e hanno eseguito le rispettive istruzioni di inizializzazione
2. Processo<a name="_page2_x56.69_y618.40"></a> atomo

Ogni processo atomo `e dotato di un numero atomico casuale compreso tra 1 e NATOMMAX. Il numero atomico `e un’informazione![](Aspose.Words.86582a17-653f-49cf-a560-d4af286c4533.004.png)![](Aspose.Words.86582a17-653f-49cf-a560-d4af286c4533.005.png) privata del processo che viene assegnata dal processo padre dopo la sua creazione (la funzione

di scelta del numero atomico `e demandata al programmatore. Pu`o essere estratto un semplice numero casuale o utilizzata qualche altra distribuzione di probabilit`a).

La scissione dell’atomo `e simulata con una fork.

- Un processo atomo padre effettua una fork di un nuovo atomo.
- La somma dei numeri atomici di padre e figlio dopo la scissione `e uguale a quello del padre prima della scissione.
- Quando avviene la scissione, viene incrementata l’ energia liberata nelle statistiche mantenute dal master. La quantit`a di energia liberata dipende dai numeri atomici dei due atomi dopo la scissione, secondo la seguente funzione

energy(n1,n2) = n1n2 − max(n1,n2),

con n1 e n2 uguali ai numeri atomici dei due atomi dopo la fissione. Si osservi che l’energia liberata `e massima quando avviene una scissione in atomi con ugual numero atomico, mentre vale 0 quando n1 o n2 vale 1.

- La scissione `e comunicata dal processo attivatore . Tale meccanismo di comunicazione `e scelto a discrezione dello sviluppatore (si possono pensare a soluzioni alternative con pipe, code di messaggi, semafori, o segnali).
- Quando un atomo con numero atomico minore o uguale a MINNATOMICOriceve il comando di scissione, esso termina![](Aspose.Words.86582a17-653f-49cf-a560-d4af286c4533.006.png) e viene conteggiato nelle statistiche fra le scorie.
3. Processo<a name="_page3_x56.69_y272.28"></a> attivatore

Ogni STEPATTIVATORE, sulla base di proprie politiche (a discrezione degli sviluppatori), il processo attivatore comunica![](Aspose.Words.86582a17-653f-49cf-a560-d4af286c4533.007.png) a uno o piu` atomi la necessit`a di effettuare una scissione.

Nota bene: l’attivatore NON crea nuovi atomi, ma ordina esclusivamente ad atomi presenti di scindersi. Saranno gli atomi che a loro volta ne creeranno di nuovi.

4. Processo<a name="_page3_x56.69_y354.42"></a> alimentazione

Ogni STEPALIMENTAZIONEnanosecondi, il processo alimentazione immette nuovo combustibile, ovvero crea NNUOVIATOMI atomi.![](Aspose.Words.86582a17-653f-49cf-a560-d4af286c4533.008.png)![](Aspose.Words.86582a17-653f-49cf-a560-d4af286c4533.009.png)![](Aspose.Words.86582a17-653f-49cf-a560-d4af286c4533.010.png)

5. Terminazione

<a name="_page3_x56.69_y410.71"></a>La simulazione termina in una delle seguenti circorstanze:

timeout raggiungimento della durata impostata SIMDURATION![](Aspose.Words.86582a17-653f-49cf-a560-d4af286c4533.011.png)

explode energia liberata (al netto di quella consumata da master) maggiore del valore ENERGYEXPLODETHRESHOLD blackout![](Aspose.Words.86582a17-653f-49cf-a560-d4af286c4533.012.png)![](Aspose.Words.86582a17-653f-49cf-a560-d4af286c4533.013.png) prelievo di una quantit`a di energia maggiore di quella disponibile. La quantit`a di energia disponibile `e

definita come la quantit`a di energia prodotta dalla scissione degli atomi meno la quantit`a di energia prelevata dal processo master.

meltdown fallimento delle fork di uno qualunque dei processi

Il gruppo di studenti deve produrre configurazioni che siano in grado di generare la terminazione in ognuno dei casi sopra descritti.

Al termine della simulazione, l’output del programma deve riportare anche la causa di terminazione.

6  Descrizione<a name="_page3_x56.69_y607.45"></a> del progetto: versione “normal” (max 30)

Nella versione completa del progetto, `e presente anche un processo inibitore che controlla reazione attraverso i seguenti due meccanismi:

- assorbe parte della quantit`a di energia prodotta dalla scissione dell’atomo diminuendo la quantit`a di energia che viene liberata
- limita il numero di scissioni agendo sull’operazione di scissione rendendola probabilistica (ad esempio decidendo se la scissione debba avvenire o meno oppure trasformando in scoria uno degli atomi prodotti dopo la scissione)

Il meccanismo di assorbimento e quello di limitazione delle scissioni sono scelti dal programmatore e devono essere basati su qualche criterio adattivo.

La presenza o meno del processo inibitore deve poter essere scelta a run-time, all’inizio della simulazione. Nel caso in cui il processo inibitore sia attivo, ci si aspetta che la terminazione per “explode” e “meltdown” non avvenga.

Inoltre, l’utente deve poter fermare (e far ripartire) il processo inibitore piu` volte da terminale attraverso un meccanismo a scelta del programmatore.

7  Configurazione

<a name="_page4_x56.69_y171.56"></a>Tutti i parametri di configurazione sono letti a tempo di esecuzione, da file o da variabili di ambiente. Quindi, un cambiamento dei parametri non deve determinare una nuova compilazione dei sorgenti (non `e consentito inserire i parametri uno alla volta da terminale una volta avviata la simulazione).

8  Linee<a name="_page4_x56.69_y250.62"></a> guida per la valutazione

Ogni secondo il sistema deve produrre una stampa in cui sono elencati:

- numero di attivazioni occorse ad opera del processo attivatore (totali e relative all’ultimo secondo),
- numero di scissioni (totali e relative all’ultimo secondo);
- quantit`a di energia prodotta (totale e relativa all’ultimo secondo);
- quantit`a di energia consumata (totale e relativa all’ultimo secondo);
- quantit`a di scorie prodotte (totale e relativa all’ultimo secondo);
- (per la versione “normal”) quantit`a di energia assorbita dal processo inibitore
- (per la versione “normal”) log delle operazioni di bilanciamento condotte dal processo inibitore .
9  Requisiti<a name="_page4_x56.69_y441.54"></a> implementativi

Il progetto (sia in versione “minimal” che “normal”) deve

- evitare l’attesa attiva
- utilizzare almeno memoria condivisa, semafori e un meccanismo di comunicazione fra processi a scelta fra code di messaggi o pipe,
- essere realizzato sfruttando le tecniche di divisione in moduli del codice (per esempio, i vari processi devono essere lanciati da eseguibili diversi con execve(...)),
- essere compilato mediante l’utilizzo dell’utility make
- massimizzare il grado di concorrenza fra processi
- deallocare le risorse IPC che sono state allocate dai processi al termine del gioco
- essere compilato con almeno le seguenti opzioni di compilazione:

gcc -Wvla -Wextra -Werror

- poter eseguire correttamente su una macchina (virtuale o fisica) che presenta parallelismo (due o piu` proces- sori).

Per i motivi introdotti a lezione, ricordarsi di definire la macro GNUSOURCEo compilare il progetto con il flag -D![](Aspose.Words.86582a17-653f-49cf-a560-d4af286c4533.014.png)![](Aspose.Words.86582a17-653f-49cf-a560-d4af286c4533.015.png)~~ GNUSOURCE.![](Aspose.Words.86582a17-653f-49cf-a560-d4af286c4533.016.png)
6
