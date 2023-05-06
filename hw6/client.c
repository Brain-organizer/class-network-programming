#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/uio.h>

int getopinfo(char *buf);
int connecttoservertcp(char* port, char* ip);
void actbymode(int sockfd);

int main(int argc, char* argv[]){
	int sockfd;

	if(argc < 3) {
		printf("usage:./client remoteAddress remotePort\n");
		return -1;
	}

	sockfd = connecttoservertcp(argv[1], argv[2]);


	actbymode(sockfd);

	close(sockfd);
	return 0;
}	

void actbymode(int sockfd){
	int opcount, result, len;
	char mode[100], id[100], opbuf[1024];
	char loaddata[2048];
	struct iovec vec[3];

	vec[0].iov_base = mode;
	vec[1].iov_base = id;
	vec[2].iov_base = opbuf;
	vec[0].iov_len = 5;
	vec[1].iov_len = 0;
	vec[2].iov_len = 0;

	printf("Mode: ");
	scanf("%s", mode);

	if(strcmp(mode, "save") == 0){
		printf("ID: ");
		scanf("%s", id);
		if(strlen(id) != 4){
			printf("Error: ID length must be 4\n");
			return;
		}
		vec[1].iov_len = 5;
		
		opcount = getopinfo(opbuf);

		vec[2].iov_len = (opcount*4)*2 - 4 + 1;

		writev(sockfd, vec, 3);
		read(sockfd, &result, sizeof(result));

		printf("Operation result: %d\n", result);
	}
	else if(strcmp(mode, "load") == 0){
		printf("ID: ");
		scanf("%s", id);
		if(strlen(id) != 4){
			printf("Error: ID length must be 4\n");
			return;
		}
		vec[1].iov_len = 5;

		writev(sockfd, vec, 3);
		len = read(sockfd, loaddata, 2048);

		if(len == 0){
			return;
		}
		loaddata[len] = '\0';

		printf("%s", loaddata);
	}
	else if(strcmp(mode, "quit") == 0){
		writev(sockfd, vec, 3);
	}
	else{
		printf("supported mode: save load quit\n");
	}
}

int getopinfo(char *buf){
	int opcount;
	
	printf("Operand count: ");
	scanf("%d", &opcount);
	buf[0] = (char)opcount;
	
	if (buf[0] <= 0){
		printf("Overflow will happen(%d)\n", buf[0]);
		exit(0);
	}

	for(int i=0; i<opcount; i++) {
		printf("Operand %d: ", i);
		scanf("%d", (int*)&buf[i*4+1]);
	}

	for(int i=0; i<opcount-1; i++){
		printf("Operator %d: ", i);
		scanf(" %c", &buf[1+opcount*4+i]);
	}

	return opcount;
}

int connecttoservertcp(char* port, char* ip){
	int sockfd;
	struct sockaddr_in servaddr;
	
	if((sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		perror("socket creation failed");
		exit(-1);
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(port));
	servaddr.sin_addr.s_addr = inet_addr(ip);
	
	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ){
		perror("connect error");
		exit(-1);
	}

	return sockfd;
}
