#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> 
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <termios.h>
#define K 0.0625
int main(int argc , char **argv){
	int fd,i=5;
	char count = 5;
	unsigned int tmp;
	float res=0;
	char *hello_node = "/dev/ds18b20";
	if((fd = open(hello_node,O_RDWR|O_NOCTTY))<0){
		printf("APP open %s failed",hello_node);
	}
	else{ 
                 printf("open ds18b20 success \n" );					
	} 
	while(i--){
		read(fd, &tmp , sizeof(tmp));
		printf("read");
	        sleep(1);
	}
	res=tmp*K;
	printf("the currently temperature is %f\n",res); 		
    close(fd);
        return 0;  
}	
	




