//parses the input file into Process and event
#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

// Structure to hold process information.
typedef struct {
	char pid[10];
	char state[20];
	char prevState[20];
} Process;

int main()
{
	int i;
	int j;
	int k;
	char* rch;
	char str[200];
	char LineInFile[40][300];
	int lineP, lineQ;
	char* sch;
	char tokenizedLine[10][10];
	char diskQueue[20][10];
	int diskQueueCount = 0;
	char printerQueue[20][10];
	int printerQueueCount = 0;
	char keyboardQueue[20][10];
	int keyboardQueueCount = 0;
	int processCount = 0;
	Process processes[20];

	FILE* fp1;
	FILE* fp2;
	fp1 = fopen("inp1.txt", "r");			//open the original input file
	fp2 = fopen("inp1_parsed.txt", "w");	//output the Process ID and event to another file. 
	//You can store in variables instead of printing to file

	lineP = 0;
	i = 0;
	j = 0;
	k = 0;

	printf("Started parsing...\n");

	//copy first line of the original file to the new file
	if (fgets(str, sizeof(str), fp1) != NULL) {
		// Tokenize using space and newline as delimiters.
		char* token = strtok(str, " ");
		while (token != NULL && processCount < 20) {
			
			if (strcmp(token, "end") == 0)
				break;

			strcpy(processes[processCount].pid, token);

			// Get the process state.
			token = strtok(NULL, " ");
			if (token == NULL || strcmp(token, "end") == 0)
				break;

			strcpy(processes[processCount].state, token);
			strcpy(processes[processCount].prevState, token);

			processCount++;

			// Get the next token
			token = strtok(NULL, " ");
		}

		// Print to fp2 the processes initial states
		for (int i = 0; i < processCount; i++) {
			fprintf(fp2, "%s %s ", processes[i].pid, processes[i].state);
		}
	}
	fprintf(fp2, "\n");

	//parse each remaining line in to Process event
	//while loop with fgets reads each line
	while (fgets(str, sizeof(str), fp1) != NULL)
	{
		// Print line to tokenize
		fprintf(fp2, "\n%s", str);

		lineP = 0;
		rch = strtok(str, ":;.");					// use strtok to break up the line by : or . or ; This would separate each line into the different events
		while (rch != NULL)
		{
			strcpy(LineInFile[lineP], rch);			//copy the events into an array of strings
			lineP++;								//keep track of how many events are in that line
			rch = strtok(NULL, ":;.");				//needed for strtok to continue in the while loop
		}

		//for each event (e.g. Time slice for P7 expires) pull out process number and event
		for (i = 1; i < lineP - 1; i++)
		{
			lineQ = 0;
			sch = strtok(LineInFile[i], " ");
			while (sch != NULL)
			{
				strcpy(tokenizedLine[lineQ], sch);		//use strtok to break up each line into separate words and put the words in the array of strings
				lineQ++;								//count number of valid elements
				sch = strtok(NULL, " ");
			}

			//tokenizedLine has the event separated by spaces (e.g. Time slice for P7 expires)
			if (strcmp(tokenizedLine[1], "requests") == 0)						//Process requests an I/O device
			{
				for (j = 0; j < processCount; j++) {							// Ready -> Blocked
					if (strcmp(processes[j].pid, tokenizedLine[0]) == 0) {
						strcpy(processes[j].state, "Blocked");

						// Handle Disk, Printer, and Keyboard Queues (Adding)
						if (strcmp(tokenizedLine[3], "disk") == 0) {
							strcpy(diskQueue[diskQueueCount++], tokenizedLine[0]);
						}
						else if (strcmp(tokenizedLine[3], "printer") == 0) {
							strcpy(printerQueue[printerQueueCount++], tokenizedLine[0]);
						}
						else {
							strcpy(keyboardQueue[keyboardQueueCount++], tokenizedLine[0]);
						}
					}
				}
			}
			else if ((strcmp(tokenizedLine[2], "dispatched") == 0))				//Process is dispatched
			{
				for (j = 0; j < processCount; j++) {							// Ready -> Running
					if (strcmp(processes[j].pid, tokenizedLine[0]) == 0) {
						strcpy(processes[j].state, "Running");
					}
				}
			}
			else if (strcmp(tokenizedLine[0], "Time") == 0)						//Process has timed-out
			{
				for (j = 0; j < processCount; j++) {							// Running -> Ready
					if (strcmp(processes[j].pid, tokenizedLine[3]) == 0) {
						strcpy(processes[j].state, "Ready");
					}
				}
			}
			else if (strcmp(tokenizedLine[3], "out") == 0)						//Process is swapped out
			{
				for (j = 0; j < processCount; j++) {							// Blocked -> Blocked/Suspended
					if (strcmp(processes[j].pid, tokenizedLine[0]) == 0) {		// Ready -> Ready/Suspended
						if (strcmp(processes[j].state, "Blocked") == 0) {
							strcpy(processes[j].state, "Blocked/Suspended");
						}
						else {
							strcpy(processes[j].state, "Ready/Suspended");
						}
					}
				}
			}
			else if (strcmp(tokenizedLine[3], "in") == 0)						//Process is swapped in
			{
				for (j = 0; j < processCount; j++) {							// Blocked/Suspended -> Blocked
					if (strcmp(processes[j].pid, tokenizedLine[0]) == 0) {		// Ready/Suspended -> Ready
						if (strcmp(processes[j].state, "Blocked/Suspended") == 0) {
							strcpy(processes[j].state, "Blocked");
						}
						else {
							strcpy(processes[j].state, "Ready");
						}
					}
				}
			}
			else if (strcmp(tokenizedLine[1], "interrupt") == 0)				//An interrupt has occured
			{
				for (j = 0; j < processCount; j++) {							// Blocked -> Ready
					if (strcmp(processes[j].pid, tokenizedLine[4]) == 0) {		// Blocked/Suspended -> Ready/Suspended
						if (strcmp(processes[j].state, "Blocked") == 0) {
							strcpy(processes[j].state, "Ready");
						}
						else {
							strcpy(processes[j].state, "Ready/Suspended");
						}

						// Handle Disk, Printer, and Keyboard Queues (Removing)
						for (k = 0; k < diskQueueCount; k++) { // Search through diskQueue array
							// If process is found
							if (strcmp(diskQueue[k], tokenizedLine[4]) == 0) {
								int l;
								// Shift all remaining elements in array one position to the left
								for (l = k; l < diskQueueCount - 1; l++) {
									strcpy(diskQueue[l], diskQueue[l + 1]);
								}
								diskQueueCount--;
							}
						}
						for (k = 0; k < printerQueueCount; k++) { // Search through printerQueue array
							// If process is found
							if (strcmp(printerQueue[k], tokenizedLine[4]) == 0) {
								int l;
								// Shift all remaining elements in array one position to the left
								for (l = k; l < printerQueueCount - 1; l++) {
									strcpy(printerQueue[l], printerQueue[l + 1]);
								}
								printerQueueCount--;
							}
						}
						for (k = 0; k < keyboardQueueCount; k++) { // Search through keyboardQueue array
							// If process is found
							if (strcmp(keyboardQueue[k], tokenizedLine[4]) == 0) {
								int l;
								// Shift all remaining elements in array one position to the left
								for (l = k; l < keyboardQueueCount - 1; l++) {
									strcpy(keyboardQueue[l], keyboardQueue[l + 1]);
								}
								keyboardQueueCount--;
							}
						}
					}
				}
			}
			else																//Process has been terminated
			{
				for (j = 0; j < processCount; j++) {							// Running -> Exit
					if (strcmp(processes[j].pid, tokenizedLine[0]) == 0) {
						strcpy(processes[j].state, "Exit");
					}
				}
			}
		}

		// Print to fp2 the processes and their states
		for (i = 0; i < processCount; i++) {
			if (strcmp(processes[i].pid, "P1") == 0) { // Get rid of extra space for first process
				if (strcmp(processes[i].state, processes[i].prevState) == 0) {
					fprintf(fp2, "%s %s ", processes[i].pid, processes[i].state);
				}
				// If the state has changed, add an "*"
				else {
					fprintf(fp2, "%s %s* ", processes[i].pid, processes[i].state);
				}
			}
			else {
				if (strcmp(processes[i].state, processes[i].prevState) == 0) {
					fprintf(fp2, " %s %s ", processes[i].pid, processes[i].state);
				}
				// If the state has changed, add an "*"
				else {
					fprintf(fp2, " %s %s* ", processes[i].pid, processes[i].state);
				}
			}
		}

		// Print the device queues
		fprintf(fp2, "\n");
		fprintf(fp2, "Disk queue:");
		for (i = 0; i < diskQueueCount; i++) {
			fprintf(fp2, " %s", diskQueue[i]);
		}
		fprintf(fp2, "\n");

		fprintf(fp2, "Printer queue:");
		for (i = 0; i < printerQueueCount; i++) {
			fprintf(fp2, " %s", printerQueue[i]);
		}
		fprintf(fp2, "\n");

		fprintf(fp2, "Keyboard queue:");
		for (i = 0; i < keyboardQueueCount; i++) {
			fprintf(fp2, " %s", keyboardQueue[i]);
		}
		fprintf(fp2, "\n");

		// Move each processes state to previous state
		for (i = 0; i < processCount; i++) {
			strcpy(processes[i].prevState, processes[i].state);
		}

		fprintf(fp2, "\n");
	}

	printf("Parsing complete\n\n");

	fclose(fp1);
	fclose(fp2);

	return 0;
}
