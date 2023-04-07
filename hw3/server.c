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
	int sockfd;
	char opCount, operators[128], buf[1024];
	int operands[128], result;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t len; //초기화 해줘야 메모리 할당됨.

	if(argc<2){
		printf("usage:./server localPort\n");
		return -1;
	}

	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
			perror("socket creation failed");
			return -1;
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[1]));
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("bind failed");
		return -1;
	}

	while(1){
		memset(operands, 0, sizeof(operands));
		memset(operators, 0, sizeof(operators));
		memset(buf, 0, sizeof(buf));

		len = sizeof(cliaddr);
		if (recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&cliaddr, &len)==-1){
			perror("receive failed");
			return -1;
		}
		opCount = buf[0];

		if (opCount<=0){	
			printf("Server close(%d)\n", opCount);
			break;
		}

		printf("Operand count: %d\n", opCount);

		for(int i=0; i<opCount; i++){
			operands[i] =((int*)(buf+1))[i];
			printf("Operand %d: %d\n", i, operands[i]);
		}

		for(int i=0; i<opCount-1; i++){
			operators[i] = (buf+1+4*opCount)[i];
			printf("Operator %d: %c\n", i, operators[i]);
		}

		result = calculate(opCount, operands, operators);
		
		printf("Operation result: %d\n", result);
		if(sendto(sockfd, &result, sizeof(result), 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr)) == -1){
			perror("send failed");
			return -1;
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
