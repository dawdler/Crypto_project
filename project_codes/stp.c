// to check short time password working ....

#include<stdio.h>
#include<signal.h>
int flag = 0;

// signal handler for SIGALRM which is called when time expires..
void handler(int signum) {

	flag = 1;

	printf( "handler flag=> %d\n", flag );

	//exit(1);
}	

int main() {

	struct sigaction obj;

	obj.sa_handler = handler;

	sigaction(SIGALRM, &obj, NULL); 

	while(1) {

		// Sets alarm for 10 sec. i.e. raises SIGALRM after 10 sec.
		alarm(10);
	
		int num;
	
		printf( "enter num: " );

		scanf( "%d", &num );

		// if no input within 10 seconds came from STDIN .. then flag is 1 ..
		if(flag == 1) {

			// we don't have to break the connection .. just repeat the process from start..
				
			printf( "\ntime out!!\n" );

			continue;
		}	
	
		// to reset alarm ... value 0 is for a kind of close the alarm..
		alarm(0);

		// set flag back to 0 if it is made 1 .. 
		flag = 0;

		printf( "%d, flag-> %d\n", num, flag );
	}

	return 0;
}
