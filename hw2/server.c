#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int calculate(char, int*, char*);
//int priority(char);
int operate(int, int, char);

int main(int argc, char* argv[]){
	int sockfd, cSockfd;
	char opCount, operators[128];
	int operands[128], result;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t len = 0; //초기화 해줘야 메모리 할당됨.

	if(argc<2){
		printf("usage:./server localPort\n");
		return -1;
	}

	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
			perror("socket creation failed");
			return -1;
	}

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
		if((cSockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len)) < 0){
			perror("accept error");
			return -1;
		}

		memset(operands, 0, sizeof(operands));
		memset(operators, 0, sizeof(operators));
	
		read(cSockfd, &opCount, sizeof(char));
		if (opCount<=0){
			close(cSockfd);
			
			printf("Server close(%d)\n", opCount);
			break;
		}
		printf("Operand count: %d\n", opCount);

		for(int i=0; i<opCount; i++){
			read(cSockfd, operands+i, sizeof(int));
			printf("Operand %d: %d\n", i, operands[i]);
		}

		for(int i=0; i<opCount-1; i++){
			read(cSockfd, operators+i, sizeof(char));
			printf("Operator %d: %c\n", i, operators[i]);
		}

		result = calculate(opCount, operands, operators);
		
		printf("Operation result: %d\n", result);
		write(cSockfd, &result, sizeof(result));
		
		close(cSockfd);
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
