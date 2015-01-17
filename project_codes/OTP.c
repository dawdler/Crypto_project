// NOT very important ....

#include<stdio.h>
#include<time.h>

// generation of OTP ... random number generation range from 1000 to 9999

int generate_otp() {
	
	return (1000 + (rand()%9000));
}

int main() {

	int i;

	time_t start, end;

	time(&start);	

	for(i = 0; i < 4; i++) 

		printf( "OTP%d: %d\n", i+1, generate_otp() );

	int num;

	scanf( "%d", &num );	

	time(&end);

	printf( "\ntime taken:- %lf\n\n", difftime(end, start));

	return 0;
}	
