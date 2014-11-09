/* The Purpose of this project is to dynamically create space for records that are to be read into said memory.  
After a slight delay, enough to give us enough time to notice the change, 
the memory is then deallocated*/


#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <ctime>

#define TRUE 1
#define FALSE 0


/*This function takes in no variables.  The return variable is an unsigned integer that
represents how many records after the first will be read in.  It can return a minimum of 1
and a maximum of 6.  It could be any range, but I chose these to keep the clutter down*/
unsigned int getRandomRecordAmount();

/*This function takes in no variables. It's purpose is to randomly return an entry for us to start with.
It can return an entry number as high as 13000 and as low as 1.*/
unsigned int getStartingPoint();

/*This function exists to tell me how many entries can be found inside the text file. Seems like an essential feature
that would make it easy to implement a system where a user could input a random file with records on it.  Assuming
the followed the 65 line min/max requirement.*/
void getSizeOfFile(FILE *);

/*Good Programs always have good manners*/
void greetTA();

int _tmain(int argc, _TCHAR* argv[])
{
	int read_cycle;																								// Variable that controls how many times program will loop
	int records_to_record;																						// Variable that controls how many records to read after primary record
	int starting_index;																							// Where will begin reading from
	int byte_offset;																							// Amount of bytes we need to seek through to reach the starting line
	char * space_for_entry;																						// Variable for malloc
	errno_t error;
	FILE * record_stream;																						// File Stream created from the text file we are supplied
	
	error = fopen_s(&record_stream,"2004_21APR_L001178021.txt", "rb");											// Actually creating the file stream.
	
	if(error == NULL){																							// In this case the file was found and stream was opened.
		
		printf("The file 2004_21APR_L001178021.txt  was successfully opened!\n");
	
	}else{
		
		printf("The file 2004_21APR_L001178021.txt could not be located and/or opened.\n");						// We have failed to find the filepath.
		return 1;
		
	
	}
	getSizeOfFile(record_stream);																				// Finding the size of file
	
	srand (time(NULL));																							// Creating a "random" seed using system time.
	greetTA();
	
	for(read_cycle = 0; read_cycle < 50; read_cycle++){															// Preform following operations 50 times
		
		records_to_record = getRandomRecordAmount();
		space_for_entry = (char *)malloc(sizeof(char)*1500*records_to_record);									// Making room for a random number of entries.
		starting_index = getStartingPoint();
		
		byte_offset = starting_index*1298;																		// Bytes to offset for fseek function call
		
		fseek(record_stream, byte_offset, SEEK_SET);															// Gets us to the index we need
		fread(space_for_entry, sizeof(char), sizeof(char)*1298*records_to_record, record_stream);				// Reading in the entries
		
		space_for_entry[1298*records_to_record] = 0;                                       
		printf("\n\r%s\n\r",space_for_entry);

		Sleep(7000);																							// Wait 7 seconds
		free(space_for_entry);																					// I hate memory leaks
	}
	
	fclose(record_stream);

	return 0;
}

void getSizeOfFile(FILE* file_to_measure){
	unsigned int file_length;																					// Variable that will hold file size

	fseek(file_to_measure, 0L, SEEK_END);																		// Going to the end of the file
	file_length = ftell(file_to_measure)/65;																	// Asking for the current location of the file
	fseek(file_to_measure, 0L, SEEK_SET);																		// Returning to the beginning of the file
	
	printf("The file has %d entries! \n", file_length);

}

unsigned int getRandomRecordAmount(){																			// This function is designed to help me randomly choose the number of records I should read in.
	
	unsigned int rand_record = 0; 
	
	rand_record = rand() % 5 + 1;																			    // Generate random number between 1 and 5 so that it is not cluttered.  Could do any range.
	printf("\n\r~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\r");				// Making it look pretty
	printf("%d is the number of entries to read in after the first. \n", rand_record);

	return rand_record;																							// Return number of entries to record

}

unsigned int getStartingPoint(){																	            // This function is designed to help me randomly choose a starting point to begin reading in from.
	
	unsigned int starting_point;																				// Return value that will give me the record that I am going to start from

	starting_point = rand() % 13000 + 1;																	    // Only want up to 13000 entries and no less then 1.  End up with strange errors otherwise.
	printf("%d is the random starting entry to read records from. \n\r", starting_point);
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");						//Making it look pretty

		return starting_point;																					// Return starting point
}

void greetTA(){
printf("\n\r");
printf("Hello, and thank you for taking time to look at my project.\n\r");
printf("You will soon be bombarded by records.\n\r");
printf("Above each group is some additional information. \n\r");
printf("Press any key and hit enter to continue \n\r");
getchar();	   
}