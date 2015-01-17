#include<stdio.h>
#include<math.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<signal.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define N 3233 // N = 61*53 ..

// bank server public key ..
const int bank_public = 17;

// time_flag for timer .. set to 1 when time expires...
int time_flag = 0;

/* 	clients public-private key pair ... 
	
	Navya   => (7, 4903)
	Rahul   => (47, 863)
	Abhinav => (41, 761)
*/

struct bank_clients {

	char name[20];
	int public;	
	int private;
	int counter;
};	

struct bank_clients user[3] = { {"Navya", 7, 4903, 0}, 
								{"Rahul", 47, 863, 4},
								{"Abhinav", 41, 761, 7} };	

int keys[10] = {1578, 2171, 3065, 2433, 2410, 1842, 3175, 2621, 3215, 1877};

// handler function for time expire ... 
void handler_time(int signum) {

	time_flag = 1;

	//exit(0);
}

// function to generate response of the key ...
int generate_response(int data, int counter) {

	int i, response = 0;

	for(i = 3; i >= 0; i--) {

		int k = data/((int)pow(10, i));

		response = response*10 + (k+counter)%4;
	
		data = (data % (int)pow(10, i));
	}

	return response;
}

// function for both encryption and decryption ...
int edcrypt(int data, int key) {

	int i, response = 1;

	for(i = 1; i <= key; i++) 

		response = (response * (data%N)) % N;
	
	return response;
}

int main(int argc, char *argv[]) {

	int s;
	
	unsigned short port;
	
	struct sockaddr_in server;

	struct sigaction obj;

	obj.sa_handler = handler_time;

	if(argc != 3) {
	
		printf( "\nusage: a.out Hostname Port\n" );
		
		exit(1);
	}
	
	s = socket(AF_INET, SOCK_STREAM, 0);
	
	port = (unsigned short)atoi(argv[2]);
	
	if(s == -1) { 
		
		printf( "Error: Client not present!!!\n" );
		
		exit(2);
	}
	
	server.sin_family = AF_INET;
	
	server.sin_port = htons(port);
	
	server.sin_addr.s_addr = inet_addr(argv[1]);
	
	if((connect(s,(struct sockaddr *)&server, sizeof(server))) < 0) {
	
		printf( "Can't connect to the server!!!\n" );
		
		exit(1);
	}
	
	sigaction(SIGALRM, &obj, NULL);

	while(1) {
	
		char str[30], buf[32], name[20];
	
		// Entering username...
		printf( "Enter your name: " );
		scanf( "%s", name );
		
		if(send(s, name, sizeof(name), 0) < 0) {
		
			printf( "Check your connection!!!\n" );
	
			exit(1);
		}
		
		// Entering password for card authentication .. 
		printf( "Enter your PIN: " );
		scanf( "%s", str );
		
		strcpy(buf, str);
		if(send(s, buf, sizeof(buf), 0) < 0) {
		
			printf( "Check your connection!!!\n" );
	
			exit(1);
		}
	
		if(recv(s, buf, sizeof(buf), 0) < 0) {

			printf( "Check your connection!!!\n" );

			exit(1);
		}

		printf( "buf -> %s\n\n", buf );

		if(!strcmp(buf, "Incorrect credential")) {
		
			printf( "\n\t\t****Incorrect Credentials****\n\n" );

			continue;
		}

		// Internal process for key-response generation ... 
		int index;		
		for(index = 0; index < 5; index++) 
			
			if(!strcmp(user[index].name, name)) 	break;
				
		if(recv(s, buf, sizeof(buf), 0) < 0) {
		
			printf( "Recieving failed!\n" );
			
			exit(1);
		}	
		
		int rec_key = atoi(buf);


		printf( "1. %s %d ", buf, rec_key );


		rec_key = edcrypt(rec_key, user[index].private);


		printf( "%d\n\n", rec_key );		


		// generate response ...
		int key_response = generate_response(rec_key, user[index].counter);


		printf( "2. %d ", key_response );

	
		// encrypt response with public key of bank server..
		key_response = edcrypt(key_response, bank_public);


		printf( "%d ", key_response );


		sprintf(buf, "%d", key_response);
		
		
		printf( "%s\n\n", buf );		


		// send the encrypted response to the server ...
		if(send(s, buf, sizeof(buf), 0) < 0) {
		
			printf( "Check your connection!!!\n" );
	
			exit(1);
		}

		// 2-step verification with OTP ... 
		int i;
		
		for(i = 0; i < 3; i++) {

			if(recv(s, buf, sizeof(buf), 0) < 0) {

				printf( "OTP error: can't connect to server!!!\n\n" );
	
				exit(0);
			}
	
			int en_otp = atoi(buf);
		
			int otp = edcrypt(en_otp, user[index].private);
			
			printf( "OTP for transaction PayU India is:%d.\nDo not share it with anyone\n\n", otp );

			// set an alarm for 60 seconds...
			alarm(60);

			printf( "Enter password sent to 87xxxxxx12: " );

			int otp1;
			scanf( "%d", &otp1 );

			// Response OTP ...
			if(time_flag == 0) {

				alarm(0);

				int otp_response = edcrypt(otp1, bank_public);

				sprintf(buf, "%d", otp_response);
			}

			// if alarm raises exception flag is set to 1 .. then send server a "timeout" message ..
			else {

				char str[] = "timeout";

				strcpy(buf, str);
			}

			// reset time_flag to 0 ..
			time_flag = 0;

			if(send(s, buf, sizeof(buf), 0) < 0) {
	
				printf( "OTP response failed!!!\n\n" );		
		
				exit(0);
			}

			// status of the OTP recieved at the server ...
			if(recv(s, buf, sizeof(buf), 0) < 0) {

				printf( "Check your connection!!!\n\n" );

				exit(0);
			}

			if(!strcmp(buf, "OTP Expired")) {
	
				printf( "\nYour session expired!!!\n\nLogin again to continue...\n\n" );
		
				break;
			}

			else if(!strcmp(buf, "OTP Incorrect")) {
			
				if(i == 2) {

					printf( "\n\t\t*****Transaction Unsuccessful!!!*****\n\n" );

					break;
				}

				printf( "Incorrect OTP entered.. try again.\n\n" ); 

				continue;
			}

			// If successful transaction then increase counter of both client and server ...
			printf( "\n\t\t*****Transaction Successful!!!*****\n\n" );
			user[index].counter = (user[index].counter+1)%10;

			break;
		}
	}

	// close the socket to reuse..
	close(s);
	
	return 0;
}	
