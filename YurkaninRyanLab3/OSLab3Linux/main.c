/*
 * main.c
 *
 *  Created on: Oct 8, 2014
 *      Author: Ryan Yurkanin
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <pthread.h>

#define PSLEEP_TIME 40000
#define CSLEEP_TIME 5000
#define BUFFER_LENGTH 30
#define NUM_PRODUCERS 8
#define NUM_CONSUMERS 10
#define NUM_SANDWICHES 100  //Welcome to Ryan's Sandwich Shop

void* consumer();
void* producer();

typedef unsigned char sandwich;
typedef struct {
	unsigned char sandwichtray[NUM_SANDWICHES];
	int out;
	int in;
	sem_t full;
	sem_t empty;
	sem_t mutex;
} Gandalf;  //You shall not pass... unless certain conditions are met.

Gandalf the_white;

FILE* record;

int main(){
	record = fopen("lab3log.txt","w");
	if(record == NULL){
		printf("ERROR OPENING FILE\n");
	}

	int loopcontrol = 0;
	int* threadref = 0;  //To be used with thread_create
	pthread_t producerid[NUM_PRODUCERS];
	pthread_t consumerid[NUM_CONSUMERS];

	//Prepare the semaphores for the opening of the shop.  Otherwise Gandalf would be very confused.
	sem_init(&the_white.full, 0, 0); //When this hits zero do not produce more sandwiches.
	sem_init(&the_white.empty, 0, NUM_SANDWICHES); //When this hits NUM_SANDWICHES please stop eating
	sem_init(&the_white.mutex, 0, 1); //We are humans not savage beasts.  One at a time please.
	srand(time(NULL)); //What could POSSIBLY go wrong?

	//Create the sandwich-makers and customers

	for(loopcontrol = 0; loopcontrol < NUM_PRODUCERS; loopcontrol++){
		threadref = (int*)malloc(sizeof(int));  //Name tags.
		*threadref = loopcontrol; //Cooks need names.  Numerical Names.
		pthread_create(producerid+loopcontrol, NULL, producer, (void*)threadref);
	}

	for(loopcontrol = 0; loopcontrol < NUM_CONSUMERS; loopcontrol++){
		threadref = (int*)malloc(sizeof(int));  //Name tags.
		*threadref = loopcontrol; //Customers need names.  Numerical Names.
		pthread_create(consumerid+loopcontrol, NULL, consumer, (void*)threadref);
	}

	//Cleaning up the mess that has been made... by ending life as they know it.
	for(loopcontrol = 0; loopcontrol < NUM_PRODUCERS; loopcontrol++){
		pthread_join(*(producerid+loopcontrol), NULL);
	}

	for(loopcontrol = 0; loopcontrol < NUM_CONSUMERS; loopcontrol++){
		pthread_join(*(consumerid+loopcontrol), NULL);
	}

	// CLOOOOOSSSSING TIME!
	return 1;
}

void* consumer(void* nametag){
	record = fopen("lab3log.txt","w");
	if(record == NULL){
		printf("ERROR OPENING FILE\n");
	}

	int loopcontrol = 0;
	int name = *((int*)nametag);
	sandwich BLT;

	for(loopcontrol = 0; loopcontrol < NUM_SANDWICHES*NUM_PRODUCERS; loopcontrol++){
		usleep(CSLEEP_TIME);

	//No sandwiches please wait.
	printf("Consumer #%d Says:  I'm waiting for a sandwich to be made!  This place is so slow!\n", name);
	fprintf(record, "Consumer #%d Says:  I'm waiting for a sandwich to be made!  This place is so slow!\n", name);
	sem_wait(&the_white.full); // Gandalf will let you know when there is one ready.

	//There is a sandwich but another consumer is already eating it!
	printf("Consumer #%d Says:  I see a sandwich is available!  I'm going to try to get it!\n", name);
	fprintf(record, "Consumer #%d Says:  I see a sandwich is available. I'm going to try to get it!!\n", name);
	sem_wait(&the_white.mutex);

	//Consumer is getting served!
	BLT = the_white.sandwichtray[the_white.out];
	printf("Consumer #%d Says:  Finally got a %d sandwich!  It's okay I guess.\n", name, BLT);
	fprintf(record, "Consumer #%d Says:  Finally got a %d sandwich!  It's okay I guess.\n", name, BLT);
	the_white.out = (the_white.out+1)%NUM_SANDWICHES;

	//You got what you wanted now let someone else order!
	printf("Consumer #%d Says:  I'm leaving the store!\n", name);
	fprintf(record, "Consumer #%d Says:  I'm leaving the store!\n", name);
	sem_post(&the_white.mutex);

	//Another sandwich down
	sem_post(&the_white.empty);
}

	printf("Consumer #%d: Is full and gone. Nice.\n", name);
	fprintf(record, "Consumer #%d: Is full and gone. Nice.\n", name);
	return NULL;

}

void* producer(void* nametag){
	record = fopen("lab3log.txt","w");
	if(record == NULL){
		printf("ERROR OPENING FILE\n");
	}

	int loopcontrol = 0;
	int name = *((int*)nametag);
	sandwich BLT;


	for(loopcontrol = 0; loopcontrol < NUM_SANDWICHES*NUM_CONSUMERS; loopcontrol++){
		usleep(PSLEEP_TIME);

		//Figuring out what kind of sandwich to make
		BLT = (sandwich)(rand()%420);

		//If no one is eating your sandwiches then wait until someone does... and cry.
		printf("Producer #%d Says: Waiting for someone to eat one of our sandwiches.\n", name);
		fprintf(record, "Producer #%d Says: Waiting for someone to eat one of our sandwiches.\n", name);
		sem_wait(&the_white.empty);

		//Another cook is putting a sandwich out on the the tray!
		printf("Producer #%d Says:  I see a spot for a sandwich, Going to try to fill it!\n", name);
		fprintf(record, "Producer #%d Says:  I see a spot for a sandwich, Going to try to fill it!\n", name);
		sem_wait(&the_white.mutex);

		//I see a spot and noone is doing anything so I'll make a sandwich!
		the_white.sandwichtray[the_white.in] = BLT;
		the_white.out = the_white.in;
		the_white.in = (the_white.in + 1)%NUM_SANDWICHES;
		printf("Producer #%d Says:  Made a %d sandwich! Putting it on the tray now!\n", name, BLT);
		fprintf(record, "Producer #%d Says:  Made a %d sandwich! Putting it on the tray now!\n", name, BLT);

		//Let someone else get to the tray.
		printf("Producer #%d Says:  I did my job.  Time to go for a smoke break. Be back in 30 minutes.\n", name);
		fprintf(record, "Producer #%d Says:  I did my job.  Time to go for a smoke break. Be back in 30 minutes.\n", name);
		sem_post(&the_white.mutex);

		//Sandwich is now on the tray.
		sem_post(&the_white.full);
	}
	printf("Producer #%d: Has just quit his job.  What a shame.\n", name);
	fprintf(record, "Producer #%d: Has just quit his job.  What a shame.\n", name);

	return NULL;
}




