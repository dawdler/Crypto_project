#include<stdio.h>
#include<math.h>
#include <arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<netinet/in.h>

#define N 3233 // N = 61*53 ... 

// Bank server private and public keys ....
const int bank_private = 2753;
const int bank_public = 17;

// structure card is used for storing card information ...
struct card {

	char name[20];
	int pin;
	int counter;  // current counter of the user, synchronized with bank server
	int public_key; // public key of the user..
};

// some users ...
struct card user[5] = { {"Navya", 1234, 0, 7},
						{"Rahul", 2134, 4, 47},
						{"Abhinav", 3214, 7, 41},
						{"Atit", 2321, 5, 0},//public key to be given ..
						{"Suraj", 1325, 9, 0} };//public key to be given .. 
 

// Key-response pair stored at the server ... (Response is generated with the module key_response.c .. and is stored at server.
int keypair[2][10] = { {1578, 2171, 3065, 2433, 2410, 1842, 3175, 2621, 3215, 1877},  
					   {1130, 3202, 1203, 1322, 2010, 2113, 1313, 1110, 3211, 2100} };


/*	function for both encryption and decryption ...
	Explanation given in RSA.c ...
*/
int edcrypt(int data, int key) {

	int i, response = 1;

	for(i = 1; i <= key; i++) 

		response = (response * (data%N)) % N;
	
	return response;
}


// function to find which user has logged in...
int authenticate(char name[], int pin) {

	int i;

	for(i = 0; i < 5; i++) 

		if(!strcmp(user[i].name, name)) 

			break;

	int size = sizeof(user)/sizeof(user[0]);

	// return (last.index + 1) if the name entered matches with no users...
	if(i == size || user[i].pin != pin)
		
		return size; // no of users... 

	return i;
}
		
int main() {

	int s, slen;
	
	struct sockaddr_in server, client;
	
	// socket creation ...
	s = socket(AF_INET, SOCK_STREAM, 0);
	
	// check whether socket created or not... 
	if(s == -1) { printf( "Error: Server not present!!!\n" ), exit(1); }
	
	server.sin_family = AF_INET;
	
	server.sin_port = 0;
	
	server.sin_addr.s_addr = INADDR_ANY;
	
	// bind socket to the structure server ...
	if(bind(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
	
		printf( "Connection refused!!!\n\n" );
		
		exit(1);
	}
	
	slen = sizeof(server);
	
	if(getsockname(s, (struct sockaddr *)&server, &slen) < 0)
	{
		printf( "Error reaching server...\n\n" );
		
		exit(1);
	}
	
	printf( "Connection port: %u\n", (unsigned)htons(server.sin_port) );
	
	// listen for incoming connection ....
	if(listen(s, 1) < 0) {
	
		printf( "No client to connect!!!\n\n" );
		
		exit(1);
	}
	
	int sockd, len;
	
	len = sizeof(client);
	
	// Accept the incoming connection ...
	if((sockd = accept(s, (struct sockaddr *)&client, &len)) == -1) {
	
		printf( "Can't connect to the client!!!\n\n" );
		
		exit(1);
	}
	
	while(1) {
	
		char buf[32], name[20], str[32];
		
		// Recieving name of the client ...
		if(recv(sockd, buf, sizeof(buf), 0) < 0) {
		
			printf( "Check your connection!!!\n" );

			exit(1);
		}	
		
		strcpy(name, buf);

		// Recieving PIN of the client ...
		if(recv(sockd, buf, sizeof(buf), 0) < 0) {
		
			printf( "Check your connection!!!\n" );

			exit(1);
		}

		// converting pin to integer ..  
		int pin = atoi(buf);
		
		/* Authenticate user, if not authorized close the connection to that client ...
		   store the counter value of that client in index.. */
		int index = authenticate(name, pin);

		// If the credential entered is incorrect then print incorrect credential ...
		if(index == 5) {

			char temp[] = "Incorrect credential";

			if(send(sockd, temp, sizeof(temp), 0) < 0) {

				printf( "Connection failed!!!\n" );
	
				exit(0);
			}

			continue;
		}

		// otherwise open his connection to the server for further processing ...
		else { 
			
			char temp[] = "Correct";

			if(send(sockd, temp, sizeof(temp), 0) < 0) {

				printf( "Connection failed!!!\n" );

				exit(0);
			}
		}

		// data to be sent is the key at that index(calculated above) ..
		int data = keypair[0][user[index].counter];

		// encrypted data to be sent .. 
		int en_data = edcrypt(data, user[index].public_key);
		
		// convert it to string ...
		char en_str[20];
		sprintf(en_str, "%d", en_data);	
		
		
		printf( "1. %d %d %s\n\n", data, en_data, en_str );


		// Send encrypted data if data sending failed ... exit showing error .. 
		if(send(sockd, en_str, sizeof(en_str), 0) < 0) {

			printf( "Connection failed!!!\n" );

			exit(0);
		}
		
		// Recieve generated response ..
		if(recv(sockd, buf, sizeof(buf), 0) < 0) {
		
			printf( "Connection failed!!!\n" );

			exit(1);
		}
		
		int en_response = atoi(buf);
		
		// decrypt recieved response ...
		int response = edcrypt(en_response, bank_private);
	
	
		printf( "2. %s %d %d\n\n", buf, en_response, response );

	
		// check for equality with the corresponding response at the server...
		if(response != keypair[1][user[index].counter]) {

			// Unauthorized card ... exit with error...
			printf( "Authorization failed!!!\n\n" );

			exit(0);
		}

		/* 	OTP generation and verification ...
			a user can enter maximum of 2 wrong attempts...
		*/
		int j;
				
		for(j = 0; j < 3; j++) {
	
			int otp = 100 + (rand() % 3133);

			time_t start, end;
	
			// encrypted otp...
			int en_otp = edcrypt(otp, user[index].public_key);

			sprintf(buf, "%d", en_otp);

			if(send(sockd, buf, sizeof(buf), 0) < 0) {
	
				printf( "OTP can't be delivered!!!\n\n" );		

				exit(0);
			}

			// Recieve the OTP entered by the user ..			
			if(recv(sockd, buf, sizeof(buf), 0) < 0) {

				printf( "Recieving of OTP failed!!!\n\n" );
		
				exit(0);
			}

			// If a timeout message comes from the client then expire this session ...
			if(!strcmp(buf, "timeout")) {

				char temp[] = "OTP Expired";
	
				// send some signal to client showing logout...
				if(send(sockd, temp, sizeof(temp), 0) < 0) {
	
					printf( "OTP can't be delivered!!!\n\n" );		

					exit(0);
				}				

				break;
			}

			// else decrypt the OTP received ...
			int en_otp_response = atoi(buf);
	
			int otp_response = edcrypt(en_otp_response, bank_private);

			// check for the match if not continue sending new OTP(maximum of 3 OTP)...
			if(otp_response != otp) {

				char temp[] = "OTP Incorrect";

				if(send(sockd, temp, sizeof(temp), 0) < 0) {
	
					printf( "OTP can't be delivered!!!\n\n" );		

					exit(0);
				}		
	
				continue;
			}
		
			char temp[] = "Successful";

			/* 	if perfect match, then send a "Successful" message to the client
				and increase the counter to both client and server ..
			*/
			if(send(sockd, temp, sizeof(temp), 0) < 0) {
	
				printf( "Check your connection!!!\n\n" );		

				exit(0);
			}

			user[index].counter = (user[index].counter+1)%10;

			break;
		}
	}
	
	// close the socket to reuse it..
	close(sockd);
	
	close(s);
	
	return 0;
}	
