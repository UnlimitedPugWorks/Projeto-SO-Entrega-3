#include <signal.h>
#include <sys/time.h>
#include "lib/timer.h"

typedef struct node{
	/*int PID;
	int finished;
	int finishedstatus;
	int finishedPid;*/
	volatile sig_atomic_t PID;
	volatile sig_atomic_t finished;
	volatile sig_atomic_t finishedstatus;
	/*volatile sig_atomic_t finishedPid;*/
	struct node* next;
	/*unsigned long time;*/
	/*volatile sig_atomic_t time;*/
	TIMER_T begin;
	TIMER_T end;
}*processlist;

#define NOTFINISHED 0
#define FINISHED 1
#define NOEXITSTATUS -1

processlist insertEnd(processlist head, int i);
void freelist(processlist head);

