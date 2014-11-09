/*
* File:   main.c
* Author: Ryan Yurkanin
*
* Created on October 28, 2014
*/

#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <sys/types.h>
#include <time.h>

#define TRUE 1
#define FALSE 0
#define PSLEEP_TIME 5000
#define CSLEEP_TIME 2000
#define BUFFER_LENGTH 30
#define NUM_PRODUCERS 8
#define NUM_CONSUMERS 10
#define NUM_SANDWICHES 100  //Welcome to Ryan's Sandwich Shop



typedef unsigned char sandwich;
typedef struct { 
	unsigned char sandwichtray[NUM_SANDWICHES];
	int in;
	int out;
	HANDLE full;
	HANDLE empty;
	HANDLE mutex;
} gandalf;

DWORD WINAPI producer(LPVOID);
DWORD WINAPI consumer(LPVOID);

gandalf the_white;
FILE* record;
int namegen = 0;

int main() {
	srand((unsigned int)time(NULL));
	DWORD threadref;
	int loopcontrol = 0;
	
	// Opening a log file
	record = fopen("lab3log.txt", "w");
	if(record = NULL){
		printf("ERROR OPENING FILE");
	}

	the_white.mutex = CreateMutex(NULL, FALSE, NULL);
	the_white.empty = CreateSemaphore(NULL, NUM_SANDWICHES, NUM_SANDWICHES, NULL);
	the_white.full  = CreateSemaphore(NULL, 0, NUM_SANDWICHES, NULL);

	//Making room for producers and consumers
	HANDLE Producers[NUM_PRODUCERS];
	HANDLE Consumers[NUM_CONSUMERS];

	for(loopcontrol = 0; loopcontrol < NUM_PRODUCERS; loopcontrol++){
		Producers[loopcontrol] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) producer, NULL, 0, &threadref);

		if( Producers[loopcontrol] == NULL ){
            printf("CreateThread error: %d\n", GetLastError());
            return 1;
        }
	}

	for(loopcontrol = 0; loopcontrol < NUM_CONSUMERS; loopcontrol++){
		Consumers[loopcontrol] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) consumer, NULL, 0, &threadref);

		if( Consumers[loopcontrol] == NULL ){
            printf("CreateThread error: %d\n", GetLastError());
            return 1;
        }
	}

	WaitForMultipleObjects(NUM_PRODUCERS, Producers, TRUE, INFINITE);
	WaitForMultipleObjects(NUM_CONSUMERS, Consumers, TRUE, INFINITE);

	return 0;
}

DWORD WINAPI consumer(LPVOID LpParam){
	record = fopen("lab3log.txt","w");
	if(record == NULL){
		printf("ERROR OPENING FILE\n");
	}

	int loopcontrol = 0;
	int name = namegen%20;
	namegen++;
	sandwich BLT;

	for(loopcontrol = 0; loopcontrol < NUM_SANDWICHES*NUM_PRODUCERS; loopcontrol++){
		Sleep(CSLEEP_TIME);

	//No sandwiches please wait.
	printf("Consumer #%d Says:  I'm going to look for a sandwich, this place sucks.\n", name);
	fprintf(record, "Consumer #%d Says:  I'm going to look for a sandwich, this place sucks.\n", name);
	WaitForSingleObject(&the_white.empty, INFINITE); // Gandalf will let you know when there is one ready.

	//There is a sandwich but another consumer is already eating it!
	printf("Consumer #%d Says:  I see a sandwich is available!  Checking to see if I alone can get it.\n", name);
	fprintf(record, "Consumer #%d Says:  I see a sandwich is available. Checking to see if I alone can get it.\n", name);
	WaitForSingleObject(&the_white.mutex, INFINITE);

	//Consumer is getting served!
	BLT = the_white.sandwichtray[the_white.out];
	printf("Consumer #%d Says:  Finally got a %d sandwich!  It's okay I guess.\n", name, BLT);
	fprintf(record, "Consumer #%d Says:  Finally got a %d sandwich!  It's okay I guess.\n", name, BLT);
	the_white.out = (the_white.out+1)%NUM_SANDWICHES;

	//You got what you wanted now let someone else order!
	printf("Consumer #%d Says:  I'm leaving the store! Somone else can get a sandwich now.\n", name);
	fprintf(record, "Consumer #%d Says:  I'm leaving the store! Someone else can get a sandwich now.\n", name);
	ReleaseMutex(&the_white.mutex);

	//Another sandwich down
	ReleaseSemaphore(&the_white.empty, 1, NULL);
}

	printf("Consumer #%d: Is full and gone. Nice.\n", name);
	fprintf(record, "Consumer #%d: Is full and gone. Nice.\n", name);
	return NULL;

}

DWORD WINAPI producer(LPVOID LPParam){
	record = fopen("lab3log.txt","w");
	if(record == NULL){
		printf("ERROR OPENING FILE\n");
	}

	int loopcontrol = 0;
	int name = namegen%30;
	namegen++;
	sandwich BLT;


	for(loopcontrol = 0; loopcontrol < NUM_SANDWICHES*NUM_CONSUMERS; loopcontrol++){
		Sleep(PSLEEP_TIME);

		//Figuring out what kind of sandwich to make
		BLT = (sandwich)(rand()%420);

		//If no one is eating your sandwiches then wait until someone does... and cry.
		printf("Producer #%d Says: Checking to see if I should make a sandwich!\n", name);
		fprintf(record, "Producer #%d Says: Checking to see if I should make a sandwich!\n", name);
		WaitForSingleObject(&the_white.full, INFINITE);

		//Another cook is putting a sandwich out on the the tray!
		printf("Producer #%d Says:  I see a spot for a sandwich, checking if someone is already filling it!\n", name);
		fprintf(record, "Producer #%d Says:  I see a spot for a sandwich, checking if someone s already filling it!\n", name);
		WaitForSingleObject(&the_white.mutex, INFINITE);

		//I see a spot and noone is doing anything so I'll make a sandwich!
		the_white.sandwichtray[the_white.in] = BLT;
		the_white.out = the_white.in;
		the_white.in = (the_white.in + 1)%NUM_SANDWICHES;
		printf("Producer #%d Says:  Made a %d sandwich! Putting it on the tray now!\n", name, BLT);
		fprintf(record, "Producer #%d Says:  Made a %d sandwich! Putting it on the tray now!\n", name, BLT);

		//Let someone else get to the tray.
		printf("Producer #%d Says:  I did my job.  Time to go for a smoke break. Be back in 30 minutes.\n", name);
		fprintf(record, "Producer #%d Says:  I did my job.  Time to go for a smoke break. Be back in 30 minutes.\n", name);
		ReleaseMutex(&the_white.mutex);

		//Sandwich is now on the tray.
		ReleaseSemaphore(&the_white.full, 1, NULL);
	}
	printf("Producer #%d: Has just quit his job.  What a shame.\n", name);
	fprintf(record, "Producer #%d: Has just quit his job.  What a shame.\n", name);

	return NULL;
}