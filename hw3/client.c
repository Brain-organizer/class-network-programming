#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>

int main(int argc, char* argv[]){
	int sockfd;
	int opcount, operands[128], result=0;
	char operators[128];
	char buf[1024];
	struct sockaddr_in servaddr, cliaddr;
	socklen_t len = sizeof(struct sockaddr_in);

	if(argc < 3) {
		printf("usage:./client remoteAddress remotePort\n");
		return -1;
	}
	
	if((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
		perror("socket creation failed");
		return -1;
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[1]));
	servaddr.sin_addr.s_addr = inet_addr(argv[2]);


	printf("Operand count: ");
	scanf("%d", &opcount);
	buf[0] = (char)opcount;
	
	if (buf[0] <= 0){
		sendto(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&servaddr, len);
		close(sockfd);
		return 0;
	}

	for(int i=0; i<opcount; i++) {
		printf("Operand %d: ", i);
		scanf("%d", (int*)&buf[i*4+1]);
	}

	for(int i=0; i<opcount-1; i++){
		printf("Operator %d: ", i);
		scanf(" %c", &buf[1+opcount*4+i]);
	}

	if(sendto(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&servaddr, len)==-1){
		perror("send failed");
		return -1;
	}

	if(recvfrom(sockfd, &result, sizeof(result), 0, (struct sockaddr *)&servaddr, &len) == -1){
		perror("recieve failed");
		return -1;
	}

	printf("Operation result: %d\n", result);

	close(sockfd);
	return 0;
}	
