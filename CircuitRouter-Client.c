/*Usar mkstemp para criar nomes de pipes*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#define BUFFERSIZE 120
#define MAXPARAMS 5

int fserv, fclient;
char* clientname;

void disconectfunc(){
	char *message = "Nao foi possivel mandar a mensagem, o cliente foi desconectado do serivdor\n";
	write(1, message, strlen(message)+1);
}

void controlcfunc(){
	char *message = "\nO client foi desligado pelo ctrl+c.\n";
	write(1,message, strlen(message)+1);
	close(fclient);
	unlink(clientname);
	close(fserv);
	exit(0);
}

struct sigaction createSimplesignal(void* func){
	struct sigaction handler;
	handler.sa_handler = func;
	handler.sa_flags = SA_RESTART;
	sigemptyset(&handler.sa_mask);
	return handler;
}

int main(int argc, char** argv){
	struct sigaction controlChandler = createSimplesignal(controlcfunc);
	struct sigaction sigpipeHandler = createSimplesignal(disconectfunc);
	if( argc != 2){
		printf("%s\n","É necessario um e apenas um argumento");
		exit(-1);
	}
	clientname = (char*) malloc(sizeof(char)*32);
	char template[] = "/tmp/XXXXXX";
	char* name = "client";
	char* directory = mkdtemp(template);
	sprintf(clientname,"%s/%s.pipe",directory, name);
	printf("O nome do pipe do cliente é %s\n", clientname);
	char* serverpipename = argv[1];
	/*char* serverpipename = (char*)malloc((strlen(argv[1])+6)*sizeof(char));
	sprintf(serverpipename, "%s.pipe", argv[1]);*/
	char *returnmessage = (char*)malloc(BUFFERSIZE* sizeof(char));
	char *message = (char*) malloc(BUFFERSIZE*sizeof(char));
	printf("Vai agora tentar connectar-se ao %s.\n", serverpipename);
	if ((fserv = open (serverpipename,O_WRONLY)) < 0) {
		printf("%s\n", "O ficheiro não existe");
		exit (-1);
	}
	unlink(clientname);
	if(mkfifo(clientname, 0777) < 0){
		exit(-1);
	}
	printf("Connectado com sucesso!\n");
	char* truemessage = (char*) malloc(BUFFERSIZE*sizeof(char));
	/*signal(SIGPIPE, disconectfunc);*/
	sigaction(SIGPIPE,&sigpipeHandler, NULL);
	sigaction(SIGINT,&controlChandler, NULL);
	while(1){
		read(0, message, BUFFERSIZE);
		sprintf(truemessage, "%s|%s",strtok(message, "\n"),clientname);
		printf("Vai tentar mandar a mensagem : %s\n", truemessage);
		write(fserv, truemessage, BUFFERSIZE);
		if(errno != EPIPE){ 
			printf("Sucesso! A espera da resposta do commando...\n");
			fclient = open(clientname, O_RDONLY);
			read(fclient,returnmessage, BUFFERSIZE);
			printf("%s\n", returnmessage);
			close(fclient);
		}
		else{
			break;
		}
	}
	unlink(clientname);
	close(fserv);
	return 0;
}
