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
#include <sys/select.h>
#include <sys/time.h>
#include <sys/uio.h>


int calculate(char, int*, char*);
//int priority(char);
int operate(int, int, char);
void proc_rm(int sig);

void parentproc();
void childproc();
void actbymode(struct iovec *vec, int cScokfd, int *ptocpipe, int *ctoppipe);

int main(int argc, char* argv[]){
	
	
	pid_t pid;
	
	struct sigaction act;
	int pipeptoc[2];
	int pipectop[2];
	int byte;

	if(argc<2){
		printf("usage:./server localPort\n");
		return -1;
	}

	if(pipe(pipeptoc) == -1 || pipe(pipectop) == -1){
		perror("pipe error");
		return -1;
	}
	

	act.sa_handler = proc_rm;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	sigaction(SIGCHLD, &act, 0);

	if(fork()){
		parentproc(argv[1], pipeptoc, pipectop);
	}
	else{
		childproc(pipeptoc, pipectop);
	}

	return 0;
}

void childproc(int *pipeptoc, int *pipectop){
	char buf[4096];
	char log[100][100];
	char id[5];
	int byte;
	int lognum = 0;

	while(1){
		byte = read(pipeptoc[0],buf,4096);
		buf[byte] = '\0';

		if(buf[0] == 's'){
			strcpy(log[lognum++], buf+1);
		}
		else if(buf[0] == 'l'){
			strcpy(id, buf+1);
			byte = 0;
			
			for(int i = 0; i < lognum; i++){
				if(strncmp(id,log[i],4) == 0){
					byte += sprintf(buf+byte, "%s", log[i]);
				}
			}

			if(!byte){
				byte += sprintf(buf, "Not exist\n");
			}
		
			write(pipectop[1],buf,byte);
		}
		else if(buf[0] == 'q'){
			return;
		}
	}
}

void parentproc(char* port, int *ptocpipe, int *ctoppipe){
	int sockfd, cSockfd, fd_max, fd_num;
	fd_set reads, cpy_reads;
	struct timeval timeout;
	struct sockaddr_in servaddr, cliaddr;
	struct iovec vec[3];
	char mode[100], id[100], opbuf[1024];
	socklen_t len;
	int vec_len;


	vec[0].iov_base = mode;
	vec[1].iov_base = id;
	vec[2].iov_base = opbuf;
	vec[0].iov_len = 5;
	vec[1].iov_len = 5;
	vec[2].iov_len = 1024;


	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
			perror("socket creation failed");
			return;
	}

	int enable = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(port));
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("bind failed");
		return;
	}

	if(listen(sockfd, 5) < 0) {
		perror("socket failed");
		return;
	}

	//set file descriptor set
	FD_ZERO(&reads);
	FD_SET(sockfd, &reads);
	fd_max = sockfd;

	while(1){

		cpy_reads = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		if((fd_num=select(fd_max+1, &cpy_reads, 0, 0, &timeout)) == -1){
			break;
		}
		if(fd_num == 0){
			continue;
		}

		for(int i=0; i<fd_max+1; i++){
			if(FD_ISSET(i, &cpy_reads)){
				if(i==sockfd){
					len = sizeof(cliaddr);
					cSockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len);
					FD_SET(cSockfd, &reads);
					if(fd_max<cSockfd){
						fd_max = cSockfd;
					}
					printf("connected client: %d \n", cSockfd);
				}
				else{
					memset(mode,0,sizeof(mode));
					memset(id, 0, sizeof(id));
					memset(opbuf,0,sizeof(opbuf));

					vec_len = readv(i, vec, 3);
					if(vec_len == 0){
						FD_CLR(i, &reads);
						close(i);
						printf("closed client: %d \n", i);
					}
					else{
						actbymode(vec, i, ptocpipe, ctoppipe);
					}
				}
			}
		}
	}
	close(sockfd);
}

void actbymode(struct iovec *vec, int cSockfd, int *ptocpipe, int *ctoppipe){
	char opCount, operators[128];
	char buf[4096];
	int operands[128], result;
	int byte = 0;

	if(strcmp(vec[0].iov_base, "save") == 0){
		printf("save to %s\n", (char*)vec[1].iov_base);

		memset(operands, 0, sizeof(operands));
		memset(operators, 0, sizeof(operators));
	
		opCount = ((char *)vec[2].iov_base)[0];

		for(int i=0; i<opCount; i++){
			operands[i] = ((int *)(vec[2].iov_base+1))[i];
		}

		for(int i=0; i<opCount-1; i++){
			operators[i] = ((char*)(vec[2].iov_base+1+4*opCount))[i];
		}

		byte = sprintf(buf+byte,"s");
		byte += sprintf(buf+byte,"%s: ",(char*)vec[1].iov_base);

		for(int i=0; i<opCount-1; i++){
			byte += sprintf(buf+byte,"%d",operands[i]);
			
			buf[byte] = operators[i];
			byte += 1;
		}

		byte += sprintf(buf+byte, "%d", operands[opCount-1]);
		
		result = calculate(opCount, operands, operators);

		byte += sprintf(buf+byte, "=%d\n", result);
			
		write(ptocpipe[1],buf,byte);
		write(cSockfd, &result, sizeof(result));
	}
	else if(strcmp(vec[0].iov_base, "load") == 0){
		printf("load from %s\n", (char*)vec[1].iov_base);
		
		byte = sprintf(buf+byte,"l");
		byte += sprintf(buf+byte, "%s", (char*)vec[1].iov_base);
		write(ptocpipe[1],buf,byte);

		byte = read(ctoppipe[0],buf,4096);
		write(cSockfd, buf, byte);
	}
	else if(strcmp(vec[0].iov_base, "quit") == 0){
		printf("quit\n");

		byte = sprintf(buf+byte, "q");
		write(ptocpipe[1],buf,byte);
	}
}

int calculate(char opcount, int* operands, char *operators){
	int result = operands[0];
	
	for(int i = 1; i<opcount; i++)
		result = operate(result, operands[i], operators[i-1]);
	
	return result;
}

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

void proc_rm(int sig){
	int status;
	pid_t id=waitpid(-1, &status, WNOHANG);
	if(WIFEXITED(status)){
		printf("removed proc id: %d \n", id);
	}

	exit(0);
}