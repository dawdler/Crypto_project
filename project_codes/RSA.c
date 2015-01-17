#include<stdio.h>
#define N 3233

/* 	function for both encryption and decryption ...
	
	formula for encryption is ..
	
		cipher text = pow(plain text, public_key) % N

	and decryption is..
	
		plain text = pow(cipher text, private key) % N
*/

int edcrypt(int data, int key) {

	int i, response = 1;

	for(i = 1; i <= key; i++) 

		response = (response * (data%N)) % N;
	
	return response;
}		

int main() {

	int key1, key2, data;

	printf( "Enter key-pair:\n" );

	scanf( "%d %d", &key1, &key2 );	
	
	printf( "\nEnter the number to be transmitted: " );

	scanf( "%d", &data );

	int enc_data = edcrypt(data, key1);

	printf( "\nTransmitting encrypted data: %d\n\n", enc_data );

	int dec_data = edcrypt(enc_data, key2);

	printf( "Recieving decrypted data: %d\n\n", dec_data );
	
	return 0;
}
