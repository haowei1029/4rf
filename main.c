#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#ifdef __unix__
# include <unistd.h>
#elif defined _WIN32
# include <windows.h>
#define sleep(x) Sleep(1000 * (x))
#endif
#include <stdio.h>
#include <stdlib.h>
//#include "fun.h"
#include <string.h>
#pragma comment(lib,"x86/pthreadVC2.lib")
int mode=1;
int cycle=1;
int x=0;
int active=0;
int rate;	//rate of monitoring
int current=0;
// add Mutex for data modifying
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;


int tempscan(int x);

void clrscr();	//clear screen function

struct Device{
	char name[33];
	int tempup;	//upper temperature threshold 
	int templow;//lower temperature 
	/*
	others can be added here in the future 
	*/
};
struct Device list[10];	//create an array for the device

struct para{
	int device;
	int fault;
	int live;
};

void printstatus(int a,int b, int c);
char readstring();
int readnumber();


void* UI(void* data) {
  // char *str = (char*) data; // 取得輸入資料
	struct para *result=(struct para*)data;	//input data
//	for(int i = 0;i < 5;++i) {
	while (1){
		clrscr();
		printf("Current progress: ");
		if (mode==1){
			printf("Cycles %d checking %d\n",cycle,current);
		}else{
			printf("Pausing\n");
		}
		for (int j=0;j<active;j++){
			
			printstatus(result[j].device,result[j].fault,result[j].live);			
			result[j].live++;
		}
//		printf("hi %d\nlive %d ago\n",result[i].device,result[i].fault); 	//Output in CMD every second
		
		sleep(1);
	}
	pthread_exit(NULL); 	//exit the thread
}

void* read(void* data){
	struct para *result=(struct para*)data;	//input data
	while (1){
		for (int i=0;i<active;i++){
			current=result[i].device;
	//			temp=tempscan(number[i]);
			int temp=tempscan(result[i].device);	//read the temperature data from device
//			printf("%d %d %d \n",temp,list[result[i].device-1].templow,list[result[i].device-1].tempup);
			
			//check if the temp is in the range
			
			pthread_mutex_lock( &mutex1 ); // Mutex lock
			if (temp>=list[result[i].device-1].templow && temp<=list[result[i].device-1].tempup){	
				result[i].fault=1;
			}else{
				result[i].fault=0;
			}
			result[i].live=0;
			pthread_mutex_unlock( &mutex1 ); // Mutex unlock
			
	//			clrscr();
//			printf("tempeture is %d\n",temp);
			
			sleep(rate);
			
		}	
		cycle++;
	}	
	
	pthread_exit(NULL);
}

int main()
{
	
	//create variables
//	int x;	//how many devices
	int i;	
	char buffer[100];
//	int active;	//
	int temp;
//	int c;
//	int currenttemp[10];
//	int live[10];
//	struct Device list[10];	//create an array for the device
	
	struct para result[10];
	for (i=0;i<10;i++){
		result[i].fault=-1;
		
	}
	for (i=0;i<10;i++){
		result[i].live=0;
		
	}
//	char buffer[33];
	pthread_t t; 	//first thread UI
	pthread_t t2; 	//second thread monitoring
	while (1){
		printf("Loading configurations...\n");
		FILE *fp=fopen("device.txt", "r");//open the txt file that contains the data of the devices
		if(NULL == fp)	//check if the file is exising
		{
			printf("the device file is not exist\n");
		}
		// else {
			// fseek (fp, 0, SEEK_END);	//check if the file is empty
			// c = ftell(fp);
		// }
		// if (0 == c) {
			// printf("file is empty\n");
		// }
		
		// if(NULL = fgetc(fp)){
			// printf("the device file is empty\n");
		// }
		fscanf_s(fp,"%d",&x);	//how many devices we r entering
		if (x>10){
			printf("no more than 10 device\n");
		}
		printf("There are %d devices details loaded\n" , x);

		
		for (i=0;i<x;i++){	//load the config for each device
			fscanf(fp,"%s",&list[i].name); 
			fscanf_s(fp,"%d",&list[i].tempup);
			fscanf_s(fp,"%d",&list[i].templow);
			printf("Device %d is: %s with upper temperature limit of %d and lower temperature limit of %d \n" , i , list[i].name, list[i].tempup,list[i].templow);
			
		}
		fclose(fp);
		
		while(1){
			printf("Please confirm the configurations. Enter Y to continue or N to reload configurations:");
			scanf_s("%s",&buffer);
//			strcpy(buffer, readstring());
			if (buffer[0]=='Y'){
				break;
				break;
				
			}else if (buffer[0]=='N'){
				break;
				continue;
				
			}else{
				printf("Invalid. Please enter again\n");
				continue;
			}
			
		}
	}		
	
	printf("Please enter the amount of device that would like to monitor:");
	scanf_s("%d",&active);
	while (active>x){
		printf("Invalid. Please enter the number again.\n");
		printf("Please enter the amount of device that would like to monitor:");
		scanf_s("%d",&active);
	}
	
	int number[10];
	for (i=0;i<active;i++){
		printf("Please enter the number # of device that would like to monitor and press enter:");
		
		scanf_s("%d",&temp);
		while (temp>x){
			printf("Invalid. Please enter the number again.\n");
			printf("Please enter the number # of device that would like to monitor and press enter:");
			scanf_s("%d",&temp);
		}
//		number[i]=temp;
		result[i].device=temp;
	}
	printf("Please enter the rate of monitoring(by seconds):");
	scanf_s("%d",&rate);

	
//	tempscan(3);
	
	
	pthread_create(&t, NULL, UI, &result); 	//create thread to continuously print out the monitoring result
	pthread_create(&t2,NULL, read, &result);	//create thread to continuously check the temperature data
	while (1){
		for (i=0;i<active;i++){
//			temp=tempscan(number[i]);
//			temp=tempscan(result[i].device);	//read the temperature data from device
//			if (temp>=list[i])
//			clrscr();
//			printf("tempeture is %d\n",temp);
			
			sleep(rate);
		}

	}
	
	pthread_join(t, NULL); 		//wait for thread 1
	pthread_join(t2, NULL); 	//wait for thread 2
	
	
	
	printf("Hello World!");
	return 0;
}

int tempscan(int n){
	FILE *fp=fopen("temp.txt", "r");
	int i=0;
	int c;
	for (i=0; i<n; i++){
		fscanf_s(fp,"%d",&c);
	}
	// printf("temp is %d\n",c);
	fclose(fp);
	return c;
}

void clrscr(){
    system("@cls||clear");
}

void printstatus(int a,int b, int c){
	printf("device %d is %d live %d\n",a,b,c);
	
}


char readstring(){
	char temp;
	scanf_s("%s",&temp);
	return temp;
}
int readnumber(){
	int temp;
	scanf_s("%d",&active);
	return temp;
}