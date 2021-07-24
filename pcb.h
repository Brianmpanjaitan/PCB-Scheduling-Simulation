#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#define MAX_LEN 40

typedef struct p_Queue
{
	List *list;
}p_Queue;

typedef struct process
{
	int pid;
	int send_pid;
	int priority; // 0 - High, 1 - Medium, 2 - Low
	int state; // 0 - running, 1 - ready, 2 - blocked
	char msg[MAX_LEN];
}process;

typedef struct semaphore
{
	int val;
	int id;
	int check;
}semaphore;

bool search(List* prioQ, int search_pid);

void init(); // DONE

void create_p(int prio); // DONE

void fork_p(); // DONE

void kill_p(int search_pid); // DONE

void exit_p(); // DONE

void quantum_p(); // DONE

void send_p(int search_pid, char* msg_p); // DONE

void recv_p(); // DONE

void reply(int search_pid, char* msg_p); // DONE

void newsem(int semID, int semval); // DONE

void sem_p(int semID); // DONE

void sem_v(int semID); // DONE

void proc_info(int search_pid); // DONE

void total_info(); // DONE

int main();

