// Program to generate response of key encryption at client side ..

#include<stdio.h>
#include<math.h>

/*	function to generate response ...
	
	To generate response we just took every place digit, add it to the current counter value and took mod with 4.

	ex.  2879 and counter value -> 7
	
		1000th place digit become => (2+7)%4 = 1
		100th place digit become => (8+7)%4 = 3
		10th place digit become => (7+7)%4 = 2
		1st place digit become => (9+7)%4 = 0

	so the response generated would be-> 1320

	now this response is sent to the server for matching...
*/

int generate_response(int data, int counter) {

	int i, response = 0;

	for(i = 3; i >= 0; i--) {

		int k = data/((int)pow(10, i));

		response = response*10 + (k+counter)%4;
	
		data = (data % (int)pow(10, i));
	}

	return response;
}

int keypair[] =  {1578, 2171, 3065, 2433, 2410, 1842, 3175, 2621, 3215, 1877};  

int main() {

	int i;

	for(i = 0; i < 10; i++) 

		printf( "%d -> %d\n", keypair[i], generate_response(keypair[i], i) );	

	return 0;
}
