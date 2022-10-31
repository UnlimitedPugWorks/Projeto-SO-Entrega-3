#include "linkedlist.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "lib/commandlinereader.h"
#include "lib/timer.h"
#include <errno.h>
/* DEFINED CONSTANTS*/
#define PATH "./CircuitRouter-SeqSolver/CircuitRouter-SeqSolver"
#define THIS "CircuitRouter-AdvanceShell"
#define BUFFERSIZE 150
#define MAXPARAMS 4
#define EXITCOMMAND "exit"
#define RUNCOMMAND "run"
#define FILENAME "CircuitRouter-SeqSolver"
#define FILENAMESIZE 25
#define NOMAXCHILDREN -1
#define PIPE_FAILURE -1
/* Functions*/
int activeProcess(processlist);
int runSeqSolver(char**, int);
void prompt(int);
void closeProcess(processlist, int);
