
#ifndef _SCHEDULER_H
#define _SCHEDULER_H

int find_next_ready_process(int start_index);
void* scheduler_loop(void* arg);

#endif