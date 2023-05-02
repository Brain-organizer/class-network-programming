#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <bits/sigaction.h>
#include <sys/wait.h>


int filepid;


int calculate(char, int*, char*);
//int priority(char);
int operate(int, int, char);
void proc_rm(int sig){
	int status;
	pid_t id=waitpid(-1, &status, WNOHANG);
	if(WIFEXITED(status)&&id!=filepid){
		printf("removed proc id: %d \n", id);
	}
}

int main(int argc, char* argv[]){
	int sockfd, cSockfd;
	char opCount, operators[128];
	char buf[4096];
	int operands[128], result;
	struct sockaddr_in servaddr, cliaddr;
	pid_t pid;
	socklen_t len;
	struct sigaction act;
	int pipefd[2];
	int byte;

	if(argc<2){
		printf("usage:./server localPort\n");
		return -1;
	}

	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
			perror("socket creation failed");
			return -1;
	}

	if(pipe(pipefd) == -1){
		perror("pipe error");
		return -1;
	}
	
	if((pid = fork()) == -1){
		perror("fork error");
		return -1;
	}
	else if(pid == 0){
		int logfd = creat("log.txt",0664);

		while(1){
			
			byte = read(pipefd[0],buf,4096);

			if(buf[0]!='!'){
				write(logfd,buf,byte);
			}
			else{
				close(logfd);
				return 0;
			}
		}
	}
	filepid = pid;


	act.sa_handler = proc_rm;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	sigaction(SIGCHLD, &act, 0);

	int enable = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[1]));
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("bind failed");
		return -1;
	}

	if(listen(sockfd, 5) < 0) {
		perror("socket failed");
		return -1;
	}

	while(1){

		len = sizeof(struct sockaddr_in); 

		while((cSockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len)) < 0){
			
		}

		printf("new client connected..\n");

		if((pid = fork()) == -1){
			perror("fork error");
			return -1;
		}

		if(pid == 0){
			close(sockfd);
			
			memset(operands, 0, sizeof(operands));
			memset(operators, 0, sizeof(operators));
		
			read(cSockfd, &opCount, sizeof(char));
			if (opCount<=0){
				close(cSockfd);
				printf("Save file(%d)\n",(int)opCount);
				buf[0] = '!';
				write(pipefd[1],buf,1);
				return 0;
			}

			for(int i=0; i<opCount; i++){
				read(cSockfd, operands+i, sizeof(int));
			}

			for(int i=0; i<opCount-1; i++){
				read(cSockfd, operators+i, sizeof(char));
			}

			
			byte = sprintf(buf,"%d: ",getpid());

			for(int i=0; i<opCount-1; i++){
				byte += sprintf(buf+byte,"%d",operands[i]);
				
				buf[byte] = operators[i];
				byte += 1;
			}

			byte += sprintf(buf+byte, "%d", operands[opCount-1]);
			
			result = calculate(opCount, operands, operators);

			byte += sprintf(buf+byte, "=%d\n", result);
			
			write(pipefd[1],buf,byte);
			write(1,buf,byte);
			write(cSockfd, &result, sizeof(result));

			return 0;
		}
		else{
			close(cSockfd);
		}
	}
	close(sockfd);
	return 0;
}

int calculate(char opcount, int* operands, char *operators){
	int result = operands[0];
	
	for(int i = 1; i<opcount; i++)
		result = operate(result, operands[i], operators[i-1]);
	
	return result;
}
/*
int calculate(char opcount, int* operands, char *operators){
	int oprtri = 0, oprtrj = 1, oprnd1 = operands[0] , oprnd2 = operands[1], oprndi = 2;
	//oprnd1: 첫번째 피연산자
	//oprnd2: 두번째 피연산자
	//oprndi: 세번째 피연산자의 인덱스
	
	//oprtri: 첫번째 연산자의 인덱스
	//oprtrj: 두번째 연산자의 인덱스

	while(oprndi<opcount){
		if (priority(operators[oprtri]) <= priority(operators[oprtrj])){
			oprnd1 = operate(oprnd1,oprnd2,operators[oprtri]);
			oprnd2 = operands[oprndi];
			oprndi++;

			oprtri = oprtrj;
			oprtrj++;
		} else{
			oprnd2 = operate(oprnd2, operands[oprndi], operators[oprtrj]);
			oprndi++;

			oprtrj++;
		}
	}
	
	return operate(oprnd1, oprnd2, operators[oprtri]);
}

int priority(char operator){
	switch(operator){
		case '*':
			return 1;
		case '+':
		case '-':
			return 2;
	}
}
*/
int operate(int n, int m, char opr){
	switch(opr){
		case '*':
			return n*m;
		case '+':
			return n+m;
		case '-':
			return n-m;
	}
}
