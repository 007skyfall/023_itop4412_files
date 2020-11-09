#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define USAGE_MESSAGE \
    "Usage:\n" \
    " [cmd1]:start, " \
        "to send value \n" \
	" [cmd2]:over , " \
        "to close process \n" \
		
#define DATA_PROTOCOL \
	"[device] [cmd] [data]...[end]\n"\
	"device=leds;[cmd1] 0 or 1,[cmd2]:0 or 1;end\n"\
	"device=buzzer;[cmd] 0 or 1;end\n"\
	"device=uart2;[data];end\n"\
	"device=console;[data];end\n"

char *console()
{
	int res=0;
	char *p;
	char data[100];
	char buffer[100];
	
	while(1){
		fprintf(stderr,USAGE_MESSAGE);
		memset(buffer, 0, 100);
		memset(data, 0, 100);
		
		scanf("%s",buffer);
		if(strcmp(buffer,"over")==0){
			strcat(data,buffer);
			p = data;
			return p;
		}
		else if(strcmp(buffer,"start")==0){
			fprintf(stderr,DATA_PROTOCOL);	
			
			res = 1;
			do{
				printf("Input send [data] or cmd:[end],length<=100 and no spacing\n");
				
				scanf("%s",buffer);
				if(strcmp(buffer,"end")==0)
					res = 0;
				
				strcat(data,buffer);
			}while(res);
						
			printf("The send data is %s\n",data);
			p = data;
			return p;
		}	
	}
}

int main(int argc, char **argv)
{
	pid_t pid;
	int sfp, nfp, num = 0;
	struct sockaddr_in s_add,c_add;
	int sin_size,recbyte;
	unsigned short portnum=0x8888;

	char buffer[100] = {0};

	printf("Hello,welcome to my server !\r\n");

	sfp = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == sfp)
	{
		printf("socket fail ! \r\n");
		return -1;
	}

	printf("socket ok !\r\n");

	bzero(&s_add,sizeof(struct sockaddr_in));
	s_add.sin_family=AF_INET;
	s_add.sin_addr.s_addr=htonl(INADDR_ANY);
	s_add.sin_port=htons(portnum);

	if(-1 == bind(sfp,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
	{
		printf("bind fail !\r\n");
		return -1;
	}

	printf("bind ok !\r\n");

	if(-1 == listen(sfp,5))
	{
		printf("listen fail !\r\n");
		return -1;
	}
	printf("listen ok\r\n");



	sin_size = sizeof(struct sockaddr_in);

	nfp = accept(sfp, (struct sockaddr *)(&c_add), &sin_size);
	if(-1 == nfp){
		printf("accept fail !\r\n");
		return -1;
	}

	printf("accept ok!\r\nServer start get connect from %#x : %#x\r\n",ntohl(c_add.sin_addr.s_addr), ntohs(c_add.sin_port));
	
	
	pid = fork();
	if(pid == -1){
		printf("fork failed\n");
		return 1;
	}
	else if(pid){
		while(1){
			scanf("%s",buffer);
			send(nfp, buffer, strlen(buffer), 0);
			usleep(500000);
		}
	}
	else{
		while(1){
			if(-1 == (recbyte = read(nfp, buffer, 1024)))
			{
				printf("read data fail !\r\n");
				return -1;
			}
			buffer[recbyte]='\0';
			printf("%s",buffer);
			memset(buffer, 0, recbyte);	
			usleep(500000);
		}
	}

	
over:
		close(nfp);
		close(sfp);
		return 0;			
}
