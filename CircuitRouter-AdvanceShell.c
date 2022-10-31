#include "CircuitRouter-AdvanceShell.h"
processlist PIDlist = NULL;
int runningProcesses = 0;
struct sigaction childHandler;
int lab_mode = 0;

static void parseArgs(long argc, char* const argv[]){
	long opt = 0;
	opterr = 0;
	optind = 0;
	while((opt = getopt(argc, argv, "l"))!= -1){
		switch(opt){
			case 'l':
				lab_mode = 1;
				optind = 2;
				break;
			default:
				break;
		}
	}
}

void child_receiver(){
	sigaction(SIGCHLD,&childHandler, NULL); 
	int status, pid;
	processlist current;
	while((pid = waitpid(-1,&status, WNOHANG)) > 0){ /*Enquanto houver zombies*/
		current = PIDlist; /*Procura os zombies*/
		while(current != NULL){ 
			if((current->PID == pid) && (WIFEXITED(status))){
				closeProcess(current, status);
				break;
			}
		current = current->next;
		}
	}
}

int main(int argc, char** argv){
	parseArgs(argc, argv);
	int fserv, fclient;
	fd_set rfds;
	char* directory;
	char* progname = strtok(argv[0], "/.");
	char* pipename;
	if(lab_mode){
		directory = "/tmp/";
		pipename = (char*)malloc((strlen(argv[0]) + strlen(directory) + 7)*sizeof(char));
		sprintf(pipename, "%s%s.pipe", directory, progname);
	}
	else{
		pipename = (char*)malloc((strlen(argv[0]) + 7)*sizeof(char));
		sprintf(pipename, "%s.pipe", progname);
	}
	int MAXCHILDREN = NOMAXCHILDREN, numargs, Pid, currentPid, status, i = 0;
	processlist current;
	char* errormessage = "Command not supported.";
	char **line = (char**)malloc(MAXPARAMS*sizeof(char*));
	char *buffer = (char*)malloc(BUFFERSIZE* sizeof(char));
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGCHLD);
	unlink(pipename);
	if(argc > 1){
		if((optind == 2) && (argc == 3)){
			MAXCHILDREN = atoi(argv[optind]);
		}
		else if((optind == 1) && (argc >=1)){
			MAXCHILDREN = atoi(argv[1]);
		}
		else if(argc > 3){
			printf("Este programa só suporta 2 argumentos no maximo.\n");
			printf("A flag -l, para ativar o lab mode que permite o programa correr nos PC de Laboratorio\n");
			printf("E o argumento opcional MAXCHILDREN.\n");
			exit(EXIT_FAILURE);
		}
	}

	if(mkfifo(pipename, 0777) < 0){
		perror("Error: ");
		exit(EXIT_FAILURE);
	}
	printf("The server's pipe is:%s\n", pipename);

	prompt(MAXCHILDREN); /*Shows the beginning message to the user*/

	if((fserv = open(pipename, O_RDWR)) < 0){
		exit(EXIT_FAILURE);
	}

	childHandler.sa_handler = child_receiver;
	sigemptyset(&childHandler.sa_mask);
	childHandler.sa_flags = (SA_RESTART  | SA_NOCLDSTOP);
	sigaction(SIGCHLD,&childHandler, NULL); 
	while(1){
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		FD_SET(fserv, &rfds);
		do{
			errno = 0;
			select(fserv+1,&rfds, NULL, NULL, NULL);
		}while(errno == EINTR);
		if(FD_ISSET(fserv, &rfds)){
			readLineArguments(line, MAXPARAMS, buffer, BUFFERSIZE, fserv);
			if(strcmp(line[1],"run")!=0){
				do{
					errno = 0;
					fclient = open(line[0], O_WRONLY);
				}while(errno == EINTR);
				do{
					errno = 0;
					write(fclient, errormessage, strlen(errormessage)+1);
				}while(errno == EINTR);
				do{
					errno = 0;
					close(fclient);
				}while(errno == EINTR);
			}
			else{
				sigprocmask(SIG_BLOCK,&set, NULL);
				Pid = runSeqSolver(line, fserv);
				PIDlist = insertEnd(PIDlist, Pid);
				sigprocmask(SIG_UNBLOCK, &set, NULL);
			}		
		}
		else if(FD_ISSET(0, &rfds)){
			numargs = readLineArguments(line, MAXPARAMS, buffer, BUFFERSIZE, 0); /*Reads the commands on the line*/
			if ((numargs == 1) && (strcmp(line[0],EXITCOMMAND) == 0)){/*If it detects only one arguments and it's exit, then it exits the program*/
				break;
			}
			else if((numargs == 2) && (strcmp(line[0],RUNCOMMAND) == 0)){ /*If it detects two arguments and the first argument is run, then it goes into run*/
				if ((MAXCHILDREN == NOMAXCHILDREN) || ((MAXCHILDREN != NOMAXCHILDREN) && runningProcesses < MAXCHILDREN)){
					sigprocmask(SIG_BLOCK, &set, NULL);
					Pid = runSeqSolver(line, 0);
					PIDlist = insertEnd(PIDlist, Pid);
					sigprocmask(SIG_UNBLOCK, &set, NULL);
				}	
				else{
					printf("%s\n%s\n","MAXCHILDREN reached", "Processing..."); /*Shows the use that MAXCHILDREN proceses have been used*/
					sigprocmask(SIG_BLOCK,&set,NULL);
					Pid = wait(&status);
					if(Pid == -1){
						perror("Error:");
					}
					current = PIDlist;
					while(current != NULL){
						if((current->PID == Pid) && (WIFEXITED(status))){
							closeProcess(current, status);
						}
						current = current->next;
					}
					printf("%s\n", "Processed! Processes will now be launched"); /*Runs the process*/
					Pid = runSeqSolver(line, 0);
					PIDlist = insertEnd(PIDlist, Pid);
					sigprocmask(SIG_UNBLOCK,&set,NULL);
				}
			}
			else{
				printf("%s\n","Command not recognized");
			}
		}

	}
	sigprocmask(SIG_BLOCK,&set,NULL);
	current = PIDlist;
	while(current != NULL){
		currentPid = current->PID;
		if (current->finished !=FINISHED){ /*If a process is not finished, wait for it*/
			waitpid(currentPid, &status,0);
			closeProcess(current, status);
		}
		printf("CHILD EXITED(PID=%d; return %s; %d s)\n", currentPid, (current->finishedstatus==0)?"OK":"NOK", (int) TIMER_DIFF_SECONDS(current->begin, current->end)) ; /*If that status is equal to 0, it exited in an okay form, otherwise it ended*/
		current = current->next;
	}
	freelist(PIDlist); /*Frees the PIDLIST*/
	free(buffer); /*Frees the buffer*/
	for(i = 0; i < MAXPARAMS;i++){ /*Frees the components of line*/
		if(line[i] != NULL){
			free(line[i]);
		}
	}
	close(fserv);
	unlink(pipename);
	free(line); /*Frees line*/
	free(pipename);
	printf("%s\n", "END.");
	return 0;
}

int runSeqSolver(char** args, int fds){
	int SeqPid = fork();
	if(SeqPid == 0){ /*Child Process*/
		if(fds == 0){	
			execl(PATH, FILENAME, args[1], NULL);/*Executes the program*/
		}
		else{
			execl(PATH, FILENAME, args[2], args[0], NULL);
		}
	}
	if(SeqPid == -1){
		printf("Erro a realizar o fork. O programa sera encerrado.\n");
		exit(EXIT_FAILURE);
	}
	runningProcesses++;
	return SeqPid;
}

void prompt(int MAXCHILDREN){ /*Shows the beginning message*/
	printf("Welcome to CircuitRouter-AdvanceShell.\n");
	printf("Lab Mode: %s\n", (lab_mode)? "ON" : "OFF");
	if (MAXCHILDREN == NOMAXCHILDREN){
		printf("MAXCHILDREN: Unlimited\n");
	}
	else{
		printf("MAXCHILDREN: %d\n", MAXCHILDREN);
	}
}

void closeProcess(processlist current, int status){
	TIMER_READ(current->end);
	current->finished = FINISHED;
	current->finishedstatus = WEXITSTATUS(status);
	runningProcesses--;
}