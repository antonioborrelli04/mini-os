# ROADMAP — mini-os

Questa roadmap guida lo sviluppo di `mini-os`, un simulatore didattico in C per studiare Sistemi Operativi scrivendo codice verificabile fase per fase.

Obiettivo: ogni fase deve compilare, produrre un output osservabile e collegarsi a un concetto d'esame.

---

## Regola operativa

Per ogni fase:

1. Leggi l'obiettivo teorico.
2. Modifica solo i file indicati.
3. Scrivi una funzione alla volta.
4. Compila con `make`.
5. Esegui con `make run`.
6. Verifica l'output atteso.
7. Rispondi alle domande di controllo.
8. Solo dopo passa alla fase successiva.

Non saltare le verifiche. Il compilatore non perdona, anche se noi fingiamo di essere ottimisti.

---

## Stato iniziale della repository

La repository contiene già:

- `README.md`
- `Makefile`
- `src/main.c`

Il `main.c` attuale implementa già una simulazione monolitica con:

- process table;
- pthread per rappresentare processi simulati;
- mutex globale `kernel_lock`;
- condition variable per scheduler e processi;
- scheduling Round Robin semplificato;
- stati `READY`, `RUNNING`, `BLOCKED`, `TERMINATED`.

La prima fase non deve riscrivere tutto, ma trasformare questo codice in moduli più puliti.

---

# Fase 1 — Baseline compilabile

## Obiettivo

Verificare che la repository compili ed esegua il codice corrente.

## Comandi

```bash
make clean
make
make run
```

## Output atteso

Devono comparire log simili:

```txt
[SCHEDULER] dispath process 0
[PROCESS 0] executing instruction, remaining before = 5
[PROCESS 0] back to READY, remaining = 4
...
[SCHEDULER] all processes terminated
```

## Stop condition

Puoi passare alla fase successiva solo se:

- `make` termina senza errori;
- `make run` termina senza bloccarsi;
- sai spiegare il ruolo di `kernel_lock` e delle condition variable.

## Domande di controllo

1. Perché `process_run` usa `while` intorno a `pthread_cond_wait`?
2. Cosa succede se lo scheduler mette un processo in `RUNNING`?
3. Perché il processo segnala `scheduler_cond` dopo aver eseguito una istruzione?
4. Che differenza c'è tra un processo reale e il thread pthread usato qui per simularlo?

---

# Fase 2 — Separazione del modulo process

## Obiettivo

Estrarre dal `main.c` tutto ciò che riguarda il processo simulato.

## File da creare

```txt
include/process.h
src/process.c
```

## File da modificare

```txt
src/main.c
Makefile se necessario
```

## Contenuti di `process.h`

Deve contenere:

- enum `ProcessState`;
- struct `Process`;
- prototipo `const char* process_state_to_string(ProcessState state);`;
- prototipo `void process_init(Process* p, int pid, int instructions);`.

## Funzioni richieste

```c
const char* process_state_to_string(ProcessState state);
void process_init(Process* p, int pid, int instructions);
```

## Stop condition

Il progetto deve comportarsi come prima, ma la definizione del processo non deve più stare in `main.c`.

## Domande di controllo

1. Perché `ProcessState` deve stare in un header?
2. Perché `process_init` è meglio di inizializzare i campi direttamente nel `main`?
3. Quali campi della struct rappresentano il PCB semplificato?

---

# Fase 3 — Separazione dello scheduler

## Obiettivo

Estrarre lo scheduler dal `main.c`.

## File da creare

```txt
include/scheduler.h
src/scheduler.c
```

## Responsabilità del modulo

Lo scheduler deve gestire:

- scelta del prossimo processo ready;
- dispatch del processo;
- attesa del completamento del tick;
- avanzamento circolare dell'indice.

## Stop condition

`main.c` deve limitarsi a:

1. inizializzare i processi;
2. creare i thread processo;
3. creare il thread scheduler;
4. fare join e cleanup.

## Domande di controllo

1. Perché questo scheduler assomiglia a Round Robin?
2. Quale campo rappresenta il tempo residuo di esecuzione?
3. Dove avviene il context switch simulato?

---

# Fase 4 — Tick CPU esplicito

## Obiettivo

Rendere esplicito il concetto di CPU tick.

## Modifica

Sostituire la logica “esegui una istruzione” con una funzione:

```c
void process_execute_tick(Process* p);
```

## Stop condition

Ogni dispatch dello scheduler deve causare esattamente un tick del processo selezionato.

## Domande di controllo

1. Che cos'è un tick in questa simulazione?
2. Perché Round Robin lavora bene con l'idea di quantum/tick?
3. Cosa cambierebbe se il quantum fosse maggiore di 1?

---

# Fase 5 — Scheduler Round Robin con quantum

## Obiettivo

Generalizzare l'esecuzione da 1 tick fisso a `quantum` configurabile.

## Funzione

```c
void process_execute_quantum(Process* p, int quantum);
```

## Test

Usa:

```txt
MAX_PROCESSES = 4
INITIAL_INSTRUCTIONS = 5
QUANTUM = 2
```

## Stop condition

Un processo deve eseguire al massimo `QUANTUM` istruzioni prima di tornare READY, salvo terminazione.

## Domande di controllo

1. Cosa succede se il quantum è troppo piccolo?
2. Cosa succede se il quantum è troppo grande?
3. Round Robin è preemptive o non-preemptive?

---

# Fase 6 — Memoria virtuale v1

## Obiettivo

Aggiungere il calcolo:

```txt
virtual address -> page number + offset
```

## File da creare

```txt
include/page.h
include/mmu.h
src/mmu.c
```

## Costanti iniziali

```c
#define PAGE_SIZE 256
#define MAX_PAGES 16
#define MAX_FRAMES 8
```

## Funzioni

```c
int mmu_get_page_number(int virtual_address);
int mmu_get_offset(int virtual_address);
```

## Test minimo

Con `virtual_address = 515`:

```txt
page_number = 2
offset = 3
```

## Domande di controllo

1. Perché la divisione dà il numero di pagina?
2. Perché il modulo dà l'offset?
3. L'offset cambia nella traduzione VA -> PA?

---

# Fase 7 — Page table e traduzione MMU

## Obiettivo

Tradurre un indirizzo virtuale in indirizzo fisico.

## Struct

```c
typedef struct {
    int page_number;
    int frame_number;
    int present;
    int dirty;
    int referenced;
} PageTableEntry;

typedef struct {
    PageTableEntry entries[MAX_PAGES];
} PageTable;
```

## Formula

```txt
physical_address = frame_number * PAGE_SIZE + offset
```

## Funzione

```c
int mmu_translate(PageTable* pt, int virtual_address, int* physical_address);
```

## Stop condition

Con:

```txt
page 2 -> frame 7
virtual_address = 515
```

il risultato deve essere:

```txt
physical_address = 1795
```

## Domande di controllo

1. Che cosa contiene la page table?
2. Differenza tra pagina e frame?
3. Perché la formula usa il frame e non la pagina?

---

# Fase 8 — Page fault

## Obiettivo

Gestire pagine non presenti in memoria fisica.

## File da creare

```txt
include/memory.h
src/memory.c
```

## Funzioni

```c
void memory_init(PhysicalMemory* memory);
int memory_allocate_frame(PhysicalMemory* memory);
int handle_page_fault(Process* p, PhysicalMemory* memory, int page_number);
```

## Stop condition

Se una pagina ha `present == 0`, la MMU deve stampare un page fault e il sistema deve assegnare un frame libero.

## Domande di controllo

1. Un page fault è sempre un errore fatale?
2. Cosa fa il sistema operativo dopo un page fault valido?
3. Cosa succede se non ci sono frame liberi?

---

# Fase 9 — Lettura e scrittura memoria

## Obiettivo

Usare la memoria fisica simulata.

## Funzioni

```c
int memory_read(Process* p, PhysicalMemory* mem, int virtual_address, unsigned char* value);
int memory_write(Process* p, PhysicalMemory* mem, int virtual_address, unsigned char value);
```

## Stop condition

Scrivi un byte a un indirizzo virtuale e rileggilo correttamente.

## Domande di controllo

1. Perché anche `memory_write` passa dalla MMU?
2. Quando si imposta `dirty = 1`?
3. Una lettura può causare page fault?

---

# Fase 10 — Page replacement FIFO

## Obiettivo

Quando non ci sono frame liberi, scegliere una pagina vittima.

## Algoritmo

FIFO rimuove la pagina caricata da più tempo.

## Funzioni

```c
int select_victim_fifo(Process* p);
int replace_page_fifo(Process* p, PhysicalMemory* mem, int new_page);
```

## Domande di controllo

1. Perché serve page replacement?
2. FIFO considera l'uso recente della pagina?
3. Qual è il limite principale di FIFO?

---

# Fase 11 — Second Chance / Clock

## Obiettivo

Usare il bit `referenced` per evitare di rimuovere subito pagine usate recentemente.

## Regola

```txt
referenced == 1 -> referenced = 0 e la pagina riceve una seconda possibilità
referenced == 0 -> vittima
```

## Domande di controllo

1. A cosa serve `referenced`?
2. Perché Second Chance migliora FIFO?
3. Una pagina con `referenced = 1` è impossibile da rimuovere?

---

# Fase 12 — Swap simulato

## Obiettivo

Simulare memoria secondaria.

## File da creare

```txt
include/swap.h
src/swap.c
```

## Funzioni

```c
void swap_out(...);
void swap_in(...);
```

## Regola dirty bit

```txt
se dirty == 1, salva la pagina nello swap prima di rimuoverla
se dirty == 0, puoi scartarla
```

## Domande di controllo

1. Perché il dirty bit evita scritture inutili?
2. Differenza tra page fault e swap in?
3. Cosa succede se la pagina vittima è dirty?

---

# Fase 13 — IPC: message queue simulata

## Obiettivo

Far comunicare processi simulati tramite messaggi.

## File da creare

```txt
include/ipc.h
src/ipc.c
```

## Struct

```c
typedef struct {
    int sender_pid;
    int receiver_pid;
    char payload[128];
} Message;
```

## Domande di controllo

1. Comunicazione diretta e indiretta: differenza?
2. Send asincrona cosa significa?
3. Receive bloccante cosa significa?

---

# Fase 14 — Monitor produttore/consumatore

## Obiettivo

Implementare un monitor con mutex e condition variable.

## File da creare

```txt
include/monitor.h
src/monitor.c
```

## Regola fondamentale

Usare sempre `while`, non `if`, intorno a `pthread_cond_wait`.

## Domande di controllo

1. Perché `while` e non `if`?
2. Cosa fa `pthread_cond_wait` al mutex?
3. Quando segnali `not_empty`?
4. Quando segnali `not_full`?

---

# Fase 15 — Deadlock simulator

## Obiettivo

Simulare il classico deadlock tra due processi e due risorse.

## Scenario

```txt
P1 acquisisce R1
P2 acquisisce R2
P1 chiede R2 e si blocca
P2 chiede R1 e si blocca
```

## Domande di controllo

1. Quali sono le quattro condizioni del deadlock?
2. Deadlock e starvation sono la stessa cosa?
3. Come si rompe l'attesa circolare?

---

# Fase 16 — Algoritmo del banchiere

## Obiettivo

Implementare una versione semplificata di deadlock avoidance.

## Domande di controllo

1. Cos'è uno stato sicuro?
2. Cos'è una sequenza sicura?
3. Perché bisogna conoscere il claim massimo?

---

# Prompt per assistente vocale

Usa questo prompt all'inizio di ogni sessione:

```txt
Siamo nella fase X della roadmap mini-os.
Guidami passo passo.
Non scrivere tutto il codice subito.
Fammi prima spiegare il concetto teorico.
Poi guidami nella modifica di una struct o funzione alla volta.
Dopo ogni modifica fammi compilare e verificare.
Prima di passare alla fase successiva fammi domande di controllo.
```

## Prompt per debug

```txt
Questo è l'errore di compilazione o l'output ottenuto.
Spiegami cosa significa.
Aiutami a localizzare il problema senza riscrivere tutto il progetto.
```

---

# Criterio finale

La fase è completata solo se valgono tutte e tre le condizioni:

```txt
1. Il codice compila.
2. L'output è coerente con l'obiettivo.
3. Sai spiegare il concetto senza leggere il codice.
```
