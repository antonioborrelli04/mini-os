# mini-os (Mini Linux) Pthread

Mini Linux Pthread è un progetto didattico scritto in C con pthread che simula alcuni meccanismi fondamentali di un sistema operativo moderno, prendendo ispirazione dal modello Linux.

L'obiettivo non è costruire un vero kernel, perché evidentemente abbiamo già abbastanza modi per complicarci la vita, ma realizzare un simulatore user-space utile per studiare e comprendere concretamente i principali concetti di Sistemi Operativi.

Il progetto implementa progressivamente:

- tabella dei processi e PCB;
- stati dei processi: READY, RUNNING, BLOCKED, TERMINATED;
- scheduler simulato con pthread, mutex e condition variable;
- modello di memoria virtuale;
- traduzione Virtual Address → Physical Address;
- Page Table e TLB simulati;
- gestione di Page Fault;
- Demand Paging;
- algoritmi di sostituzione delle pagine: FIFO, LRU, Clock;
- Buddy System per l'allocazione dei frame fisici;
- zone di memoria in stile Linux;
- Page Cache e Page Frame Reclaim simulati.

Il progetto nasce come supporto pratico allo studio dell'esame di Sistemi Operativi, collegando teoria e codice C passo dopo passo.
