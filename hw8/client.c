#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/uio.h>
	
#define BUF_SIZE 1024
#define NAME_SIZE 5
	
void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);
	
char name[NAME_SIZE];
char msg[BUF_SIZE];
	
int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void * thread_return;
	if(argc!=4) {
		printf("Usage : %s <port> <IP> <name>\n", argv[0]);
		exit(1);
	}
	
    if(strlen(argv[3]) != 4){
        printf("ID have to be 4\n");
        return 0;
    }
	sprintf(name, "%s", argv[3]);
	sock=socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[2]);
	serv_addr.sin_port=htons(atoi(argv[1]));
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
	
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	close(sock);  
	return 0;
}
	
void * send_msg(void * arg)   // send thread main
{
	int sock=*((int*)arg);
    struct iovec vec[2];
    int opCount;

    vec[0].iov_base = name;
    vec[0].iov_len = 4;
    vec[1].iov_base = msg;
    vec[1].iov_len = BUF_SIZE;
	while(1) 
	{
		scanf("%d", &opCount);
        msg[0] = (char)opCount;

        if(msg[0]<=0){
            printf("Overflow Number(%d) - Closed client\n", opCount);
            exit(0);
        }

		for(int i=0; i<opCount; i++) {
            scanf("%d", (int*)&msg[i*4+1]);
        }

        for(int i=0; i<opCount-1; i++){
            scanf(" %c", &msg[1+opCount*4+i]);
        }

        writev(sock, vec, 2);
	}
	return NULL;
}
	
void * recv_msg(void * arg)   // read thread main
{
	int sock=*((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE];
	int str_len;
	while(1)
	{
		str_len=read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
		if(str_len==-1) 
			return (void*)-1;
		name_msg[str_len]=0;
		fputs(name_msg, stdout);
	}
	return NULL;
}

	
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}