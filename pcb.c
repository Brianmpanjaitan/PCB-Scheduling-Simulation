#include "list.h"
#include "pcb.h"
#define MAX_LEN 40

int flag = 1;
char input[MAX_LEN];
int pidvalue = 1000;

// STATE: 0 - running, 1 - ready, 2 - blocked
// PRIORITY: 0 - High, 1 - Medium, 2 - Low
// strcpy() function was from overiq.com/c-programming-101/the-strcpy-function-in-c/

List* prioQ0;
List* prioQ1;
List* prioQ2;
List* send_q;
List* recv_q;
List* blocked_sem0;
List* blocked_sem1;
List* blocked_sem2;
List* blocked_sem3;
List* blocked_sem4;

struct process* p;
struct process* initial;
struct process* current;

struct semaphore* sem0;
struct semaphore* sem1;
struct semaphore* sem2;
struct semaphore* sem3;
struct semaphore* sem4;

// Bool function to compare items
bool compare(void* pItem, void* comparisonArg)
{
	if (pItem != comparisonArg)
	{
		return 0;
	}
	else 
	{
		return 1;
	}
}

bool search(List* prioQ, int search_pid)
{
	if(List_count(prioQ) > 0)
	{
		while(List_curr(prioQ) != NULL)
		{
			if( ((struct process*) List_curr(prioQ))->pid == search_pid ) // Check if its in prioQ
			{
				return 1;
			}
			List_next(prioQ);		
		}
	}
	return 0;
}


void init()
{
	prioQ0 = List_create();
	prioQ1 = List_create();
	prioQ2 = List_create();
	send_q = List_create();
	recv_q = List_create();
	
	blocked_sem0 = List_create();
	blocked_sem1 = List_create();
	blocked_sem2 = List_create();
	blocked_sem3 = List_create();
	blocked_sem4 = List_create();

	sem0 = (struct semaphore*)malloc(sizeof(semaphore));
	sem1 = (struct semaphore*)malloc(sizeof(semaphore));
	sem2 = (struct semaphore*)malloc(sizeof(semaphore));
	sem3 = (struct semaphore*)malloc(sizeof(semaphore));
	sem4 = (struct semaphore*)malloc(sizeof(semaphore));
	
    struct process* p = (struct process*)malloc(sizeof(process));
	current = (struct process*)malloc(sizeof(process));
	initial = (struct process*)malloc(sizeof(process));
	
    p->priority = 0;
    p->pid = pidvalue;
    p->state = 0;

    initial->priority = p->priority;
    initial->pid = p->pid;
    initial->state = 0;

    current->priority = initial->priority;
    current->pid = initial->pid;
    current->state = 0;

	pidvalue++;
	printf("initialized\n");
}

void create_p(int prio)
{
    struct process* p = (struct process*)malloc(sizeof(process));
    p->priority = prio;
    p->state = 1;
	p->pid = pidvalue;
	printf("PID of the created process is: %d\n", p->pid);
    
    if(current->pid == 1000)
    {
    	p->state = 0;
    	current = p;
    }
    
	if(prio == 0)
	{
		List_append(prioQ0, p);
	}
	else if(prio == 1)
	{
		List_append(prioQ1, p);
	}
	else if(prio == 2)
	{
		List_append(prioQ2, p);
	}
    pidvalue++;
}

void fork_p()
{
    //Check if current is initial process
    if(current->pid == 1000)
    {
    	printf("Fork Failed: Current process is the initial process %d\n", current->pid);
    	return;
    }
    
    struct process* copied_p = (struct process*)malloc(sizeof(copied_p));
    copied_p->pid = pidvalue;
    copied_p->priority = current->priority;
    copied_p->state = 1; // Put on ready Q
    strcpy(copied_p->msg, current->msg);
    
    if(copied_p->priority == 0)
    {
    	List_append(prioQ0, copied_p);
    }
    else if(copied_p->priority == 1)
    {
    	List_append(prioQ1, copied_p);
    }
    else if(copied_p->priority == 2)
    {
    	List_append(prioQ2, copied_p);
    }
	printf("PID of the forked process: %d\n", copied_p->pid);
    pidvalue++;
}

void kill_p(int search_pid)
{
	// Check if we are killing the current process
	// If so, run exit_p()
	if(search_pid == current->pid)
	{
		exit_p();
		return;
	}

	List_first(prioQ0);
	if(search(prioQ0, search_pid))
	{
		printf("Found and killed process in prioQ0\n");
		List_remove(prioQ0);
		return;	
	}
	
	List_first(prioQ1);
	if(search(prioQ1, search_pid))
	{
		printf("Found and killed process in prioQ1\n");
		List_remove(prioQ0);
		return;	
	}
	
	List_first(prioQ2);
	if(search(prioQ2, search_pid))
	{
		printf("Found and killed process in prioQ2\n");
		List_remove(prioQ2);
		return;	
	}
	
	List_first(send_q);
	if(search(send_q, search_pid))
	{
		printf("Found and killed process in prioQ2\n");
		List_remove(send_q);
		return;	
	}
	
	List_first(recv_q);
	if(search(recv_q, search_pid))
	{
		printf("Found and killed process in prioQ2\n");
		List_remove(recv_q);
		return;	
	}
	printf("Process not found\n");
}

void exit_p()
{
	struct process* p = (struct process*)malloc(sizeof(process)); // temporary value
	int kill_pid = current->pid;
	List_first(prioQ0);
	if(search(prioQ0, kill_pid))// Current process is in high Q
	{
		p = List_remove(prioQ0);
		// Check if any of the Q's are empty
		// If not, make new current to the head and run it
		if(List_count(prioQ0) > 0) // Check if High is empty
		{
			((struct process*) List_first(prioQ0))->state = 0;
			current = List_first(prioQ0);
		}
		else if(List_count(prioQ1) > 0) // Check if Medium is empty
		{
			((struct process*) List_first(prioQ1))->state = 0;
			 current = List_first(prioQ1);
		}
		else if(List_count(prioQ2) > 0) // Check if Low is empty
		{
			((struct process*) List_first(prioQ2))->state = 0;
			current = List_first(prioQ2);
		}
		printf("Current process PID %d killed\n", p->pid);
		printf("Process PID %d in priority Q%d has control of the CPU\n", current->pid, current->priority);
		return;
	}
	
	List_first(prioQ1);
	if(search(prioQ1, kill_pid)) // Current process is in medium Q
	{
		p = List_remove(prioQ1);
		if(List_count(prioQ0) > 0) // Check if High is empty
		{
			((struct process*) List_first(prioQ0))->state = 0;
			current = List_first(prioQ0);
		}
		else if(List_count(prioQ1) > 0) // Check if Medium is empty
		{
			((struct process*) List_first(prioQ1))->state = 0;
			current = List_first(prioQ1);
		}
		else if(List_count(prioQ2) > 0) // Check if Low is empty
		{
			((struct process*) List_first(prioQ2))->state = 0;
			current = List_first(prioQ2);
		}
		printf("Current process PID %d killed\n", p->pid);
		printf("Process PID %d in priority Q%d has control of the CPU\n", current->pid, current->priority);
		return;
	}
	
	List_first(prioQ2);
	if(search(prioQ2, kill_pid)) // Current process is in low Q
	{
		p = List_remove(prioQ2);
		if(List_count(prioQ0) > 0) // Check if High is empty
		{
			((struct process*) List_first(prioQ0))->state = 0;
			current = List_first(prioQ0);
		}
		else if(List_count(prioQ1) > 0) // Check if Medium is empty
		{
			((struct process*) List_first(prioQ1))->state = 0;
			current = List_first(prioQ1);
		}
		else if(List_count(prioQ2) > 0) // Check if Low is empty
		{
			((struct process*) List_first(prioQ2))->state = 0;
			current = List_first(prioQ2);
		}
		printf("Current process PID %d killed\n", p->pid);
		printf("Process PID %d in priority Q%d has control of the CPU\n", current->pid, current->priority);
		return;
	}
	printf("No current process on any Q\n");
}

void quantum_p()
{
	if(current->pid == 1000)
	{
		printf("Quantum failed: Only one process");
		return;
	}
	struct process* p = (struct process*)malloc(sizeof(process)); // temporary value
	int curr_pid = current->pid;
	
	List_first(prioQ0);
	if(search(prioQ0, curr_pid)) // Current process is in Q0
	{
		// Move process to back of Q
		((struct process*) List_curr(prioQ0))->state = 1; // Ready
		p = List_remove(prioQ0); // p now holds the ex current process
		
		// Run the next available process. If prioQ0 empty, check prioQ1, and so on
		if(List_count(prioQ0) > 0)
		{
			((struct process*) List_first(prioQ0))->state = 0; // Running
			current = List_first(prioQ0);
		}
		else if(List_count(prioQ1) > 0) 
		{
			((struct process*) List_first(prioQ1))->state = 0; // Running
			current = List_first(prioQ1);
		}
		else if(List_count(prioQ2) > 0)
		{
			((struct process*) List_first(prioQ2))->state = 0; // Running
			current = List_first(prioQ2);
		}
		List_append(prioQ0, p); // Add to back of the queue
		printf("Process PID %d gave up control of the CPU for Process PID %d\n", p->pid, current->pid);
		return;
	}
	
	List_first(prioQ1);
	if(search(prioQ1, curr_pid)) // Current process is in Q1
	{
		// Move process to back of Q
		((struct process*) List_first(prioQ0))->state = 1; // Ready
		p = List_remove(prioQ1); // p now holds the ex current process

		// Run the next available process. If prioQ0 empty, check prioQ1, and so on
		if(List_count(prioQ0) > 0)
		{
			((struct process*) List_first(prioQ0))->state = 0; // Running
			current = List_first(prioQ0);
		}
		else if(List_count(prioQ1) > 0) 
		{
			((struct process*) List_first(prioQ1))->state = 0; // Running
			current = List_first(prioQ1);
		}
		else if(List_count(prioQ2) > 0)
		{
			((struct process*) List_first(prioQ2))->state = 0; // Running
			current = List_first(prioQ2);
		}
		List_append(prioQ1, p); // Add to back of the queue
		printf("Process PID %d gave up control of the CPU for Process PID %d\n", p->pid, current->pid);
		return;
	}
	
	List_first(prioQ2);
	if(search(prioQ2, curr_pid)) // Current process is in Q2
	{
		// Move process to back of Q
		((struct process*) List_first(prioQ0))->state = 1; // Ready
		p = List_remove(prioQ2); // p now holds the ex current process

		// Run the next available process. If prioQ0 empty, check prioQ1, and so on
		if(List_count(prioQ0) > 0)
		{
			((struct process*) List_first(prioQ0))->state = 0; // Running
			current = List_first(prioQ0);
		}
		else if(List_count(prioQ1) > 0) 
		{
			((struct process*) List_first(prioQ1))->state = 0; // Running
			current = List_first(prioQ1);
		}
		else if(List_count(prioQ2) > 0)
		{
			((struct process*) List_first(prioQ2))->state = 0; // Running
			current = List_first(prioQ2);
		}
		List_append(prioQ2, p); // Add to back of the queue
		printf("Process PID %d gave up control of the CPU for Process PID %d\n", p->pid, current->pid);
		return;
	}
	printf("Quantum Failed\n");
}

void send_p(int search_pid, char* msg_p)
{
	struct process* p = (struct process*)malloc(sizeof(process)); // temporary value
	int curr_pid = current->pid;
	// Search for the process
	List_first(prioQ0);
	if(search(prioQ0, search_pid))
	{
		strcpy(((struct process*) List_curr(prioQ0))->msg, msg_p);
		((struct process*) List_curr(prioQ0))->send_pid = current->pid;
		
		// Find current (sender) pid and remove it from its queue
		List_first(prioQ0);
		if(search(prioQ0, current->pid)) // in Q0
		{
			((struct process*) List_curr(prioQ0))->state = 2; // blocked
			p = List_remove(prioQ0);
		}
		
		List_first(prioQ1);
		if(search(prioQ1, current->pid)) // in Q1
		{
			((struct process*) List_curr(prioQ1))->state = 2; // blocked
			p = List_remove(prioQ1);
		}
		
		List_first(prioQ2);
		if(search(prioQ2, current->pid)) // in Q2
		{
			((struct process*) List_curr(prioQ2))->state = 2; // blocked
			p = List_remove(prioQ2);
		}
		
		// Place sender on blocked Q and make next available process current
		List_append(send_q, p);
		if(List_count(prioQ0) > 0)
		{
			((struct process*) List_first(prioQ0))->state = 0; // Running
			current = List_first(prioQ0);
		}
		else if(List_count(prioQ1) > 0) 
		{
			((struct process*) List_first(prioQ1))->state = 0; // Running
			current = List_first(prioQ1);
		}
		else if(List_count(prioQ2) > 0)
		{
			((struct process*) List_first(prioQ2))->state = 0; // Running
			current = List_first(prioQ2);
		}
		printf("Message succesfully sent, current process is PID %d\n", current->pid);
		return;
	}
	
	List_first(prioQ1);
	if(search(prioQ1, search_pid))
	{
		strcpy(((struct process*) List_curr(prioQ1))->msg, msg_p);
		((struct process*) List_curr(prioQ1))->send_pid = current->pid;
		
		// Find current (sender) pid
		List_first(prioQ0);
		if(search(prioQ0, current->pid))
		{
			((struct process*) List_curr(prioQ0))->state = 2;
			p = List_remove(prioQ0);
		}
		
		List_first(prioQ1);
		if(search(prioQ1, current->pid))
		{
			((struct process*) List_curr(prioQ1))->state = 2;
			p = List_remove(prioQ1);
		}
		
		List_first(prioQ2);
		if(search(prioQ2, current->pid))
		{
			((struct process*) List_curr(prioQ2))->state = 2;
			p = List_remove(prioQ2);
		}
		
		// Place sender on blocked Q and make next available process current
		List_append(send_q, p);
		if(List_count(prioQ0) > 0)
		{
			((struct process*) List_first(prioQ0))->state = 0; // Running
			current = List_first(prioQ0);
		}
		else if(List_count(prioQ1) > 0) 
		{
			((struct process*) List_first(prioQ1))->state = 0; // Running
			current = List_first(prioQ1);
		}
		else if(List_count(prioQ2) > 0)
		{
			((struct process*) List_first(prioQ2))->state = 0; // Running
			current = List_first(prioQ2);
		}
		printf("Message succesfully sent, current process is PID %d\n", current->pid);
		return;
	}
	
	List_first(prioQ2);
	if(search(prioQ2, search_pid))
	{
		strcpy(((struct process*) List_curr(prioQ2))->msg, msg_p);
		((struct process*) List_curr(prioQ2))->send_pid = current->pid;
		
		// Find current (sender) pid and remove it from its queue
		List_first(prioQ0);
		if(search(prioQ0, current->pid)) // in Q0
		{
			((struct process*) List_curr(prioQ0))->state = 2; // blocked
			p = List_remove(prioQ0);
		}
		
		List_first(prioQ1);
		if(search(prioQ1, current->pid)) // in Q1
		{
			((struct process*) List_curr(prioQ1))->state = 2; // blocked
			p = List_remove(prioQ1);
		}
		
		List_first(prioQ2);
		if(search(prioQ2, current->pid)) // in Q2
		{
			((struct process*) List_curr(prioQ2))->state = 2; // blocked
			p = List_remove(prioQ2);
		}
		
		// Place sender on blocked Q and make next available process current
		List_append(send_q, p);
		if(List_count(prioQ0) > 0)
		{
			((struct process*) List_first(prioQ0))->state = 0; // Running
			current = List_first(prioQ0);
		}
		else if(List_count(prioQ1) > 0) 
		{
			((struct process*) List_first(prioQ1))->state = 0; // Running
			current = List_first(prioQ1);
		}
		else if(List_count(prioQ2) > 0)
		{
			((struct process*) List_first(prioQ2))->state = 0; // Running
			current = List_first(prioQ2);
		}
		printf("Message succesfully sent, current process is PID %d\n", current->pid);
		return;
	}
	printf("Message send failed\n");
}

void recv_p()
{
	if(strlen(current->msg) > 0) // There is a message
	{
		printf("Process PID %d sent: %s\n", current->send_pid, current->msg);
		memset(current->msg, 0, sizeof(current->msg)); // Empty message
		strcpy(current->msg, "");
		return;
	}
	
	struct process* p = (struct process*)malloc(sizeof(process)); // temporary value
	// Block until one arrives
	p = current;
	List_append(recv_q, p);
	if(List_count(prioQ0) > 0)
	{
		current = List_first(prioQ0);
	}
	else if(List_count(prioQ1) > 0) 
	{
		current = List_first(prioQ1);
	}
	else if(List_count(prioQ2) > 0)
	{
		current = List_first(prioQ2);
	}
	printf("No message, current process is PID %d\n", current->pid);
}

void reply(int search_pid, char* msg_p)
{
	struct process* p = (struct process*)malloc(sizeof(process)); // temporary value
	List_first(send_q);
	if(search(send_q, search_pid))
	{
		strncpy(((struct process*) List_curr(send_q))->msg, msg_p, (sizeof(char) * MAX_LEN));
		
		// Remove from blocked Q
		((struct process*) List_curr(send_q))->send_pid = current->pid;
		p = List_remove(send_q);
		p->state = 1;
		
		// Add back to priority Q
		if(p->priority == 0)
		{
			List_append(prioQ0, p);
		}
		else if(p->priority == 1)
		{
			List_append(prioQ1, p);
		}
		else if(p->priority == 2)
		{
			List_append(prioQ2, p);
		}
		printf("Reply delivered to process PID %d\n", p->pid);
		return;
	}
	printf("PID not in sender's blocked Q\n");
}

void newsem(int semID, int semval)
{
	if(semID == 0)
	{
		if(sem0->check == 0) // Has not been initialized before
		{
			sem0->val = semval;
			sem0->id = semID;
			sem0->check = 1;
			printf("Semaphore 0 has been initialized with ID %d and value %d\n", sem0->id, sem0->val);
		}
		else
		{
			printf("ERROR: semaphore 0 has already been initialized\n");
		}
	}
	else if(semID == 1)
	{
		if(sem1->check == 0) // Has not been initialized before
		{
			sem1->val = semval;
			sem1->id = semID;
			sem1->check = 1;
			printf("Semaphore 1 has been initialized with ID %d and value %d\n", sem1->id, sem1->val);
		}
		else
		{
			printf("ERROR: semaphore 1 has already been initialized\n");
		}
	}
	else if(semID == 2)
	{
		if(sem2->check == 0) // Has not been initialized before
		{
			sem2->val = semval;
			sem2->id = semID;
			sem2->check = 1;
			printf("Semaphore 2 has been initialized with ID %d and value %d\n", sem2->id, sem2->val);
		}
		else
		{
			printf("ERROR: semaphore 2 has already been initialized\n");
		}
	}
	else if(semID == 3)
	{
		if(sem3->check == 0) // Has not been initialized before
		{
			sem3->val = semval;
			sem3->id = semID;
			sem3->check = 1;
			printf("Semaphore 3 has been initialized with ID %d and value %d\n", sem3->id, sem3->val);
		}
		else
		{
			printf("ERROR: semaphore 3 has already been initialized\n");
		}
	}
	else if(semID == 4)
	{
		if(sem4->check == 0) // Has not been initialized before
		{
			sem4->val = semval;
			sem4->id = semID;
			sem4->check = 1;
			printf("Semaphore 4 has been initialized with ID %d and value %d\n", sem4->id, sem4->val);
		}
		else
		{
			printf("ERROR: semaphore 4 has already been initialized\n");
		}
	}
}

void sem_p(int semID)
{
	struct process* p = (struct process*)malloc(sizeof(process)); // temporary value
	int curr_pid = current->pid;
	if(semID == 0) // Semaphore 0
	{
		if(sem0->check == 1) // Check if sem has been initialized
		{
			if(sem0->val <= 0)
			{
				List_first(prioQ0);
				if(search(prioQ0, curr_pid))
				{
					((struct process*) List_curr(prioQ0))->state = 2; // blocked
					p = List_remove(prioQ0);
				}
				List_first(prioQ1);
				if(search(prioQ1, curr_pid))
				{
					((struct process*) List_curr(prioQ1))->state = 2; // blocked
					p = List_remove(prioQ1);
				}
				List_first(prioQ2);
				if(search(prioQ2, curr_pid))
				{
					((struct process*) List_curr(prioQ2))->state = 2; // blocked
					p = List_remove(prioQ2);
				}
				
				List_append(blocked_sem0, p);
				if(List_count(prioQ0) > 0)
				{
					((struct process*) List_first(prioQ0))->state = 0; // Running
					current = List_first(prioQ0);
				}
				else if(List_count(prioQ1) > 0) 
				{
					((struct process*) List_first(prioQ1))->state = 0; // Running
					current = List_first(prioQ1);
				}
				else if(List_count(prioQ2) > 0)
				{
					((struct process*) List_first(prioQ2))->state = 0; // Running
					current = List_first(prioQ2);
				}
				printf("Process Blocked\n");
				return;
			}
			printf("Semaphore P success\n");
			sem0->val--;
			return;
		}
		printf("Semaphore 0 has not been initialized\n");
		return;
	}
	
	if(semID == 1) // Semaphore 1
	{
		if(sem1->check == 1)
		{
			if(sem1->val <= 0)
			{
				List_first(prioQ0);
				if(search(prioQ0, curr_pid))
				{
					((struct process*) List_curr(prioQ0))->state = 2; // blocked
					p = List_remove(prioQ0);
				}
				List_first(prioQ1);
				if(search(prioQ1, curr_pid))
				{
					((struct process*) List_curr(prioQ1))->state = 2; // blocked
					p = List_remove(prioQ1);
				}
				List_first(prioQ2);
				if(search(prioQ2, curr_pid))
				{
					((struct process*) List_curr(prioQ2))->state = 2; // blocked
					p = List_remove(prioQ2);
				}
				
				List_append(blocked_sem1, p);
				if(List_count(prioQ0) > 0)
				{
					((struct process*) List_first(prioQ0))->state = 0; // Running
					current = List_first(prioQ0);
				}
				else if(List_count(prioQ1) > 0) 
				{
					((struct process*) List_first(prioQ1))->state = 0; // Running
					current = List_first(prioQ1);
				}
				else if(List_count(prioQ2) > 0)
				{
					((struct process*) List_first(prioQ2))->state = 0; // Running
					current = List_first(prioQ2);
				}
				printf("Process Blocked\n");
				return;
			}
			printf("Semaphore P success\n");
			sem1->val--;
			return;
			}
		printf("Semaphore 1 has not been initialized\n");
		return;
	}
	
	if(semID == 2) // Semaphore 2
	{
		if(sem2->check == 1)
		{
			if(sem2->val <= 0)
			{
				List_first(prioQ0);
				if(search(prioQ0, curr_pid))
				{
					((struct process*) List_curr(prioQ0))->state = 2; // blocked
					p = List_remove(prioQ0);
				}
				List_first(prioQ1);
				if(search(prioQ1, curr_pid))
				{
					((struct process*) List_curr(prioQ1))->state = 2; // blocked
					p = List_remove(prioQ1);
				}
				List_first(prioQ2);
				if(search(prioQ2, curr_pid))
				{
					((struct process*) List_curr(prioQ2))->state = 2; // blocked
					p = List_remove(prioQ2);
				}
				
				List_append(blocked_sem2, p);
				if(List_count(prioQ0) > 0)
				{
					((struct process*) List_first(prioQ0))->state = 0; // Running
					current = List_first(prioQ0);
				}
				else if(List_count(prioQ1) > 0) 
				{
					((struct process*) List_first(prioQ1))->state = 0; // Running
					current = List_first(prioQ1);
				}
				else if(List_count(prioQ2) > 0)
				{
					((struct process*) List_first(prioQ2))->state = 0; // Running
					current = List_first(prioQ2);
				}
				printf("Process Blocked\n");
				return;
			}
			printf("Semaphore P success\n");
			sem2->val--;
			return;
			}
		printf("Semaphore 2 has not been initialized\n");
		return;
	}
	
	if(semID == 3) // Semaphore 3
	{
		if(sem3->check == 1)
		{
			if(sem3->val <= 0)
			{
				List_first(prioQ0);
				if(search(prioQ0, curr_pid))
				{
					((struct process*) List_curr(prioQ0))->state = 2; // blocked
					p = List_remove(prioQ0);
				}
				List_first(prioQ1);
				if(search(prioQ1, curr_pid))
				{
					((struct process*) List_curr(prioQ1))->state = 2; // blocked
					p = List_remove(prioQ1);
				}
				List_first(prioQ2);
				if(search(prioQ2, curr_pid))
				{
					((struct process*) List_curr(prioQ2))->state = 2; // blocked
					p = List_remove(prioQ2);
				}
				
				List_append(blocked_sem3, p);
				if(List_count(prioQ0) > 0)
				{
					((struct process*) List_first(prioQ0))->state = 0; // Running
					current = List_first(prioQ0);
				}
				else if(List_count(prioQ1) > 0) 
				{
					((struct process*) List_first(prioQ1))->state = 0; // Running
					current = List_first(prioQ1);
				}
				else if(List_count(prioQ2) > 0)
				{
					((struct process*) List_first(prioQ2))->state = 0; // Running
					current = List_first(prioQ2);
				}
				printf("Process Blocked\n");
				return;
			}
			printf("Semaphore P success\n");
			sem3->val--;
			return;
		}
		printf("Semaphore 3 has not been initialized\n");
		return;
	}
	
	if(semID == 4) // Semaphore 4
	{
		if(sem4->check == 1);
		{
			if(sem4->val <= 0)
			{
				List_first(prioQ0);
				if(search(prioQ0, curr_pid))
				{
					((struct process*) List_curr(prioQ0))->state = 2; // blocked
					p = List_remove(prioQ0);
				}
				List_first(prioQ1);
				if(search(prioQ1, curr_pid))
				{
					((struct process*) List_curr(prioQ1))->state = 2; // blocked
					p = List_remove(prioQ1);
				}
				List_first(prioQ2);
				if(search(prioQ2, curr_pid))
				{
					((struct process*) List_curr(prioQ2))->state = 2; // blocked
					p = List_remove(prioQ2);
				}
				
				List_append(blocked_sem4, p);
				if(List_count(prioQ0) > 0)
				{
					((struct process*) List_first(prioQ0))->state = 0; // Running
					current = List_first(prioQ0);
				}
				else if(List_count(prioQ1) > 0) 
				{
					((struct process*) List_first(prioQ1))->state = 0; // Running
					current = List_first(prioQ1);
				}
				else if(List_count(prioQ2) > 0)
				{
					((struct process*) List_first(prioQ2))->state = 0; // Running
					current = List_first(prioQ2);
				}
				printf("Process PID %d Blocked\n", p->pid);
				return;
			}
			printf("Semaphore P success\n");
			sem4->val--;
			return;
		}
		printf("Semaphore 4 has not been initialized\n");
		return;
	}
	printf("Semaphore ID given has not been created\n");
}

void sem_v(int semID)
{
	struct process* p = (struct process*)malloc(sizeof(process)); // temporary value
	int curr_pid = current->pid;
	if(semID == 0)
	{
		if(sem0->check == 1) // Has been initialized
		{
			if(sem0->val > 0)
			{
				// Remove off blocked semaphore Q
				List_first(blocked_sem0);
				p = List_remove(blocked_sem0);
				p->state = 1;
				
				// Append process to respective priority Q
				if(p->priority == 0) 
				{
					List_append(prioQ0, p);
				}
				if(p->priority == 1) 
				{
					List_append(prioQ1, p);
				}
				if(p->priority == 2) 
				{
					List_append(prioQ2, p);
				}
				printf("Process PID %d removed and added to priority Q%d\n", p->pid, p->priority);
				return;
			}
			printf("Semaphore V success\n");
			sem0->val++;
			return;
		}
		printf("Semaphore 0 has not been initialized\n");
		return;
	}
	if(semID == 1)
	{
		if(sem1->check == 1)
		{
			if(sem1->val > 0)
			{
				// Remove off blocked semaphore Q
				List_first(blocked_sem1);
				p = List_remove(blocked_sem1);
				p->state = 1;
				
				// Append process to respective priority Q
				if(p->priority == 0) 
				{
					List_append(prioQ0, p);
				}
				if(p->priority == 1) 
				{
					List_append(prioQ1, p);
				}
				if(p->priority == 2) 
				{
					List_append(prioQ2, p);
				}
				printf("Process PID %d removed and added to priority Q%d\n", p->pid, p->priority);
				return;
			}
			printf("Semaphore V success\n");
			sem1->val++;
			return;
		}
		printf("Semaphore 1 has not been initialized\n");
		return;
	}
	if(semID == 2)
	{
		if(sem2->check == 1)
		{
			if(sem2->val > 0)
			{
				// Remove off blocked semaphore Q
				List_first(blocked_sem2);
				p = List_remove(blocked_sem2);
				p->state = 1;
				
				// Append process to respective priority Q
				if(p->priority == 0) 
				{
					List_append(prioQ0, p);
				}
				if(p->priority == 1) 
				{
					List_append(prioQ1, p);
				}
				if(p->priority == 2) 
				{
					List_append(prioQ2, p);
				}
				printf("Process PID %d removed and added to priority Q%d\n", p->pid, p->priority);
				return;
			}
			printf("Semaphore V success\n");
			sem2->val++;
			return;
		}
		printf("Semaphore 2 has not been initialized\n");
		return;
	}
	if(semID == 3)
	{
		if(sem3->check == 1)
		{
			if(sem3->val > 0)
			{
				// Remove off blocked semaphore Q
				List_first(blocked_sem3);
				p = List_remove(blocked_sem3);
				p->state = 1;
				
				// Append process to respective priority Q
				if(p->priority == 0) 
				{
					List_append(prioQ0, p);
				}
				if(p->priority == 1) 
				{
					List_append(prioQ1, p);
				}
				if(p->priority == 2) 
				{
					List_append(prioQ2, p);
				}
				printf("Process PID %d removed and added to priority Q%d\n", p->pid, p->priority);
				return;
			}
			printf("Semaphore V success\n");
			sem3->val++;
			return;
		}
		printf("Semaphore 3 has not been initialized\n");
		return;
	}
	if(semID == 4)
	{
		if(sem4->check == 1)
		{
			if(sem4->val > 0)
			{
				// Remove off blocked semaphore Q
				List_first(blocked_sem4);
				p = List_remove(blocked_sem4);
				p->state = 1;
				
				// Append process to respective priority Q
				if(p->priority == 0) 
				{
					List_append(prioQ0, p);
				}
				if(p->priority == 1) 
				{
					List_append(prioQ1, p);
				}
				if(p->priority == 2) 
				{
					List_append(prioQ2, p);
				}
				printf("Process PID %d removed and added to priority Q%d\n", p->pid, p->priority);
				return;
			}
			printf("Semaphore V success\n");
			sem4->val++;
			return;
		}
		printf("Semaphore 4 has not been initialized\n");
		return;
	}
	printf("Semaphore ID given has not been created\n");
}

void proc_info(int search_pid)
{
	// Search prioQ0 if process is in there
	List_first(prioQ0);
	if(search(prioQ0, search_pid))
	{
		printf("The process ID: %d\n", ((struct process*) List_curr(prioQ0))->pid);
		printf("The process' priority: %d\n", ((struct process*) List_curr(prioQ0))->priority);
		printf("The process' state: %d\n", ((struct process*) List_curr(prioQ0))->state);
		printf("The process' message: %s\n\n", ((struct process*) List_curr(prioQ0))->msg);
		return;
	}
	// Search prioQ1 if process is in there
	List_first(prioQ1);
	if(search(prioQ1, search_pid))
	{
		printf("The process ID: %d\n", ((struct process*) List_curr(prioQ1))->pid);
		printf("The process' priority: %d\n", ((struct process*) List_curr(prioQ1))->priority);
		printf("The process' state: %d\n", ((struct process*) List_curr(prioQ1))->state);
		printf("The process' message: %s\n\n", ((struct process*) List_curr(prioQ1))->msg);
		return;
	}
	// Search prioQ2 if process is in there
	List_first(prioQ2);
	if(search(prioQ2, search_pid))
	{
		printf("The process ID: %d\n", ((struct process*) List_curr(prioQ2))->pid);
		printf("The process' priority: %d\n", ((struct process*) List_curr(prioQ2))->priority);
		printf("The process' state: %d\n", ((struct process*) List_curr(prioQ2))->state);
		printf("The process' message: %s\n\n", ((struct process*) List_curr(prioQ2))->msg);
		return;
	}
	
	// Search send_q if process is in there
	List_first(send_q);
	if(search(send_q, search_pid))
	{
		printf("The process ID: %d\n", ((struct process*) List_curr(send_q))->pid);
		printf("The process' priority: %d\n", ((struct process*) List_curr(send_q))->priority);
		printf("The process' state: %d\n", ((struct process*) List_curr(send_q))->state);
		printf("The process' message: %s\n\n", ((struct process*) List_curr(send_q))->msg);
		return;
	}
	// Search recv_q if process is in there
	List_first(recv_q);
	if(search(recv_q, search_pid))
	{
		printf("The process ID: %d\n", ((struct process*) List_curr(recv_q))->pid);
		printf("The process' priority: %d\n", ((struct process*) List_curr(recv_q))->priority);
		printf("The process' state: %d\n", ((struct process*) List_curr(recv_q))->state);
		printf("The process' message: %s\n\n", ((struct process*) List_curr(recv_q))->msg);
		return;
	}
	
	// Search blocked_sem0 if process is in there
	List_first(blocked_sem0);
	if(search(blocked_sem0, search_pid))
	{
		printf("The process ID: %d\n", ((struct process*) List_curr(blocked_sem0))->pid);
		printf("The process' priority: %d\n", ((struct process*) List_curr(blocked_sem0))->priority);
		printf("The process' state: %d\n", ((struct process*) List_curr(blocked_sem0))->state);
		printf("The process' message: %s\n\n", ((struct process*) List_curr(blocked_sem0))->msg);
		return;
	}
	// Search blocked_sem1 if process is in there
	List_first(blocked_sem1);
	if(search(blocked_sem1, search_pid))
	{
		printf("The process ID: %d\n", ((struct process*) List_curr(blocked_sem1))->pid);
		printf("The process' priority: %d\n", ((struct process*) List_curr(blocked_sem1))->priority);
		printf("The process' state: %d\n", ((struct process*) List_curr(blocked_sem1))->state);
		printf("The process' message: %s\n\n", ((struct process*) List_curr(blocked_sem1))->msg);
		return;
	}
	// Search blocked_sem2 if process is in there
	List_first(blocked_sem2);
	if(search(blocked_sem2, search_pid))
	{
		printf("The process ID: %d\n", ((struct process*) List_curr(blocked_sem2))->pid);
		printf("The process' priority: %d\n", ((struct process*) List_curr(blocked_sem2))->priority);
		printf("The process' state: %d\n", ((struct process*) List_curr(blocked_sem2))->state);
		printf("The process' message: %s\n\n", ((struct process*) List_curr(blocked_sem2))->msg);
		return;
	}
	// Search blocked_sem3 if process is in there
	List_first(blocked_sem3);
	if(search(blocked_sem3, search_pid))
	{
		printf("The process ID: %d\n", ((struct process*) List_curr(blocked_sem3))->pid);
		printf("The process' priority: %d\n", ((struct process*) List_curr(blocked_sem3))->priority);
		printf("The process' state: %d\n", ((struct process*) List_curr(blocked_sem3))->state);
		printf("The process' message: %s\n\n", ((struct process*) List_curr(blocked_sem3))->msg);
		return;
	}
	// Search blocked_sem4 if process is in there
	List_first(blocked_sem4);
	if(search(blocked_sem4, search_pid))
	{
		printf("The process ID: %d\n", ((struct process*) List_curr(blocked_sem4))->pid);
		printf("The process' priority: %d\n", ((struct process*) List_curr(blocked_sem4))->priority);
		printf("The process' state: %d\n", ((struct process*) List_curr(blocked_sem4))->state);
		printf("The process' message: %s\n\n", ((struct process*) List_curr(blocked_sem4))->msg);
		return;
	}
	
	printf("Process was not found\n");
}

void total_info() // Print out all the info
{
	printf("High Priority Queue (Queue 0)\n");
	List_first(prioQ0);
	if(List_count(prioQ0) > 0)
	{
		while(List_curr(prioQ0) != NULL)
		{
			printf("The process ID: %d\n", ((struct process*) List_curr(prioQ0))->pid);
			printf("The process' priority: %d\n", ((struct process*) List_curr(prioQ0))->priority);
			printf("The process' state: %d\n", ((struct process*) List_curr(prioQ0))->state);
			printf("The process' message: %s\n\n", ((struct process*) List_curr(prioQ0))->msg);
			List_next(prioQ0);
		}
	}
	else
	{
		printf("EMPTY\n\n");
	}

	printf("Medium Priority Queue (Queue 1)\n");
	List_first(prioQ1);
	if(List_curr(prioQ1) > 0)
	{
		while(List_curr(prioQ1) != NULL)
		{
			printf("The process ID: %d\n", ((struct process*) List_curr(prioQ1))->pid);
			printf("The process' priority: %d\n", ((struct process*) List_curr(prioQ1))->priority);
			printf("The process' state: %d\n", ((struct process*) List_curr(prioQ1))->state);
			printf("The process' message: %s\n\n", ((struct process*) List_curr(prioQ1))->msg);
			List_next(prioQ1);
		}
	}
	else
	{
		printf("EMPTY\n\n");
	}

	printf("Low Priority Queue (Queue 2)\n");
	List_first(prioQ2);
	if(List_curr(prioQ2) > 0)
	{
		while(List_curr(prioQ2) != NULL)
		{
			printf("The process ID: %d\n", ((struct process*) List_curr(prioQ2))->pid);
			printf("The process' priority: %d\n", ((struct process*) List_curr(prioQ2))->priority);
			printf("The process' state: %d\n", ((struct process*) List_curr(prioQ2))->state);
			printf("The process' message: %s\n\n", ((struct process*) List_curr(prioQ2))->msg);
			List_next(prioQ2);
		}
	}
	else
	{
		printf("EMPTY\n\n");
	}
	
	// Sender Queue
	printf("Sender's Blocked Queue\n");
	List_first(send_q);
	List_first(send_q);
	if(List_curr(send_q) > 0)
	{
		while(List_curr(send_q) != NULL)
		{
			printf("The process ID: %d\n", ((struct process*) List_curr(send_q))->pid);
			printf("The process' priority: %d\n", ((struct process*) List_curr(send_q))->priority);
			printf("The process' state: %d\n", ((struct process*) List_curr(send_q))->state);
			printf("The process' message: %s\n\n", ((struct process*) List_curr(send_q))->msg);
			List_next(send_q);
		}
	}
	else
	{
		printf("EMPTY\n\n");
	}
	
	// Receiver Queue
	printf("Receiver's Blocked Queue\n");
	List_first(recv_q);
	List_first(recv_q);
	if(List_curr(recv_q) > 0)
	{
		while(List_curr(recv_q) != NULL)
		{
			printf("The process ID: %d\n", ((struct process*) List_curr(recv_q))->pid);
			printf("The process' priority: %d\n", ((struct process*) List_curr(recv_q))->priority);
			printf("The process' state: %d\n", ((struct process*) List_curr(recv_q))->state);
			printf("The process' message: %s\n\n", ((struct process*) List_curr(recv_q))->msg);
			List_next(recv_q);
		}
	}
	else
	{
		printf("EMPTY\n\n");
	}
	
	// Current process
	printf("Current Process:\n");
	printf("Process' ID: %d\n", current->pid);
	printf("Process' priority: %d\n", current->priority);
	printf("Process' state: %d\n", current->state);
	printf("Process' message: %s\n\n", current->msg);

	// Semaphore 0
	printf("Semaphore 0\n");
	if(sem0->check == 1)
	{
		printf("Semaphore ID: %d\n", sem0->id);
		printf("Semaphore Value: %d\n", sem0->val);
	}
	else
	{
		printf("EMPTY\n");
	}
	printf("Blocked Queue\n");
	List_first(blocked_sem0);
	if(List_count(blocked_sem0) > 0)
	{
		while(List_curr(blocked_sem0) != NULL)
		{
			printf("The process ID: %d\n", ((struct process*) List_curr(blocked_sem0))->pid);
			printf("The process' priority: %d\n", ((struct process*) List_curr(blocked_sem0))->priority);
			printf("The process' state: %d\n", ((struct process*) List_curr(blocked_sem0))->state);
			printf("The process' message: %s\n\n", ((struct process*) List_curr(blocked_sem0))->msg);
			List_next(blocked_sem0);
		}
	}
	else
	{
		printf("EMPTY\n\n");
	}
	
	// Semaphore 1
	printf("Semaphore 1\n");
	if(sem1->check == 1)
	{
		printf("Semaphore ID: %d\n", sem1->id);
		printf("Semaphore Value: %d\n", sem1->val);
	}
	else
	{
		printf("EMPTY\n");
	}
	printf("Blocked Queue\n");
	List_first(blocked_sem1);
	if(List_count(blocked_sem1) > 0)
	{
		while(List_curr(blocked_sem0) != NULL)
		{
			printf("The process ID: %d\n", ((struct process*) List_curr(blocked_sem1))->pid);
			printf("The process' priority: %d\n", ((struct process*) List_curr(blocked_sem1))->priority);
			printf("The process' state: %d\n", ((struct process*) List_curr(blocked_sem1))->state);
			printf("The process' message: %s\n\n", ((struct process*) List_curr(blocked_sem1))->msg);
			List_next(blocked_sem1);
		}
	}
	else
	{
		printf("EMPTY\n\n");
	}

	// Semaphore 2
	printf("Semaphore 2\n");
	if(sem2->check == 1)
	{
		printf("Semaphore ID: %d\n", sem2->id);
		printf("Semaphore Value: %d\n", sem2->val);
	}
	else
	{
		printf("EMPTY\n");
	}
	printf("Blocked Queue\n");
	List_first(blocked_sem2);
	if(List_count(blocked_sem2) > 0)
	{
		while(List_curr(blocked_sem2) != NULL)
		{
			printf("The process ID: %d\n", ((struct process*) List_curr(blocked_sem2))->pid);
			printf("The process' priority: %d\n", ((struct process*) List_curr(blocked_sem2))->priority);
			printf("The process' state: %d\n", ((struct process*) List_curr(blocked_sem2))->state);
			printf("The process' message: %s\n\n", ((struct process*) List_curr(blocked_sem2))->msg);
			List_next(blocked_sem2);
		}
	}
	else
	{
		printf("EMPTY\n\n");
	}

	// Semaphore 3
	printf("Semaphore 3\n");
	if(sem3->check == 1)
	{
		printf("Semaphore ID: %d\n", sem3->id);
		printf("Semaphore Value: %d\n", sem3->val);
	}
	else
	{
		printf("EMPTY\n");
	}
	printf("Blocked Queue\n");
	List_first(blocked_sem3);
	if(List_count(blocked_sem3) > 0)
	{
		while(List_curr(blocked_sem3) != NULL)
		{
			printf("The process ID: %d\n", ((struct process*) List_curr(blocked_sem3))->pid);
			printf("The process' priority: %d\n", ((struct process*) List_curr(blocked_sem3))->priority);
			printf("The process' state: %d\n", ((struct process*) List_curr(blocked_sem3))->state);
			printf("The process' message: %s\n\n", ((struct process*) List_curr(blocked_sem3))->msg);
			List_next(blocked_sem3);
		}
	}
	else
	{
		printf("EMPTY\n\n");
	}

	// Semaphore 4
	printf("Semaphore 4\n");
	if(sem4->check == 1)
	{
		printf("Semaphore ID: %d\n", sem4->id);
		printf("Semaphore Value: %d\n", sem4->val);
	}
	else
	{
		printf("EMPTY\n");
	}
	printf("Blocked Queue\n");
	List_first(blocked_sem4);
	if(List_count(blocked_sem4) > 0)
	{
		while(List_curr(blocked_sem4) != NULL)
		{
			printf("The process ID: %d\n", ((struct process*) List_curr(blocked_sem4))->pid);
			printf("The process' priority: %d\n", ((struct process*) List_curr(blocked_sem4))->priority);
			printf("The process' state: %d\n", ((struct process*) List_curr(blocked_sem4))->state);
			printf("The process' message: %s\n\n", ((struct process*) List_curr(blocked_sem4))->msg);
			List_next(blocked_sem4);
		}
	}
	else
	{
		printf("EMPTY\n\n");
	}
}

int main()
{
	printf("Starting...\n");
	init();        
	        
	while(flag)
	{
        fgets(input, MAX_LEN, stdin);
		if(strcmp(input, "C\n") == 0) //Create
		{
			int prio;
			printf("Give a priority 0-2...\n");
			scanf("%d", &prio);
			if(prio < 3 && prio >= 0)
			{
				create_p(prio);
			}
			else
			{
				printf("Not a valid priority...\n");
			}
		}
		else if(strcmp(input, "F\n") == 0) //Fork
		{
			fork_p();
		}
		else if(strcmp(input, "K\n") == 0) //Kill
		{
			int kill_pid;
			printf("Enter a 4 digit PID to kill...\n");
			scanf("%d", &kill_pid);
			if(kill_pid >= 1000 && kill_pid <= 9999)
			{
				kill_p(kill_pid);
			}
			else
			{
				printf("Not a valid PID...\n");
			}
		}
		else if(strcmp(input, "E\n") == 0) //Exit
		{
			exit_p();
		}
		else if(strcmp(input, "Q\n") == 0) //Quantum
		{
			quantum_p();
		}
		else if(strcmp(input, "S\n") == 0) //Send
		{
			int send_pid;
			char msg_p[MAX_LEN];
			
			printf("Enter a 4 digit PID to send to...\n");
			scanf("%d", &send_pid);
			printf("Enter a message to send...\n");
			scanf("%s", &msg_p);
			send_p(send_pid, msg_p);
		}
		else if(strcmp(input, "R\n") == 0) //Receive
		{
			recv_p();
		}
		else if(strcmp(input, "Y\n") == 0) //Reply
		{
			int send_pid;
			char msg_p[MAX_LEN];
			
			printf("Enter a 4 digit PID to send to...\n");
			scanf("%d", &send_pid);
			printf("Enter a message to reply with...\n");
			scanf("%s", &msg_p);
			reply(send_pid, msg_p);
		}
		else if(strcmp(input, "N\n") == 0) //New Semaphore
		{
			int semID;
			printf("Enter an ID value from 0-4...\n");
			scanf("%d", &semID);
			
			if(semID <= 4 && semID >= 0) // Valid ID
			{
				int semval;
				printf("Enter a value...\n");
				scanf("%d", &semval);
				if(semval >= 0)
				{
					newsem(semID, semval);
				}
				else
				{
					printf("Semaphore cannot take a negative value...\n");
				}
			}
			else
			{
				printf("Invalid ID...\n");
			}	
		}
		else if(strcmp(input, "P\n") == 0) //Semaphore P
		{
			int semID;
			printf("Enter a semaphore ID...\n");
			scanf("%d", &semID);
			if(semID <= 4 && semID >= 0) // Valid ID
			{
				sem_p(semID);
			}
			else
			{
				printf("Invalid ID...\n");
			}
		}
		else if(strcmp(input, "V\n") == 0) //Semaphore V
		{
			int semID;
			printf("Enter a semaphore ID...\n");
			scanf("%d", &semID);
			if(semID <= 4 && semID >= 0) // Valid ID
			{
				sem_v(semID);
			}
			else
			{
				printf("Invalid ID...\n");
			}
		}
		else if(strcmp(input, "I\n") == 0) //Procinfo
		{
			int findpid;
			printf("Enter a 4 digit PID...\n");
			scanf("%d", &findpid);
			proc_info(findpid);
		}
		else if(strcmp(input, "T\n") == 0) //Totalinfo
		{
			printf("Totalinfo: \n");
			total_info();
		}
		else if(strcmp(input, "!\n") == 0) //End
		{
			printf("Bye\n");
			flag = 0;
			break;
		}
	}
	return 0;
}
