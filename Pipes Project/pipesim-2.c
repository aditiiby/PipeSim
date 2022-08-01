 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*   Project 1 2020
    Name:  Aditi Malu name
    snumber(s):  22526301
 */


//  MAXIMUM NUMBER OF PROCESSES OUR SYSTEM SUPPORTS (PID=1..20)
#define MAX_PROCESSES                       20

//  MAXIMUM NUMBER OF SYSTEM-CALLS EVER MADE BY ANY PROCESS
#define MAX_SYSCALLS_PER_PROCESS            50

//  MAXIMUM NUMBER OF PIPES THAT ANY SINGLE PROCESS CAN HAVE OPEN (0..9)
#define MAX_PIPE_DESCRIPTORS_PER_PROCESS    10

//  TIME TAKEN TO SWITCH ANY PROCESS FROM ONE STATE TO ANOTHER
#define USECS_TO_CHANGE_PROCESS_STATE       5

//  TIME TAKEN TO TRANSFER ONE BYTE TO/FROM A PIPE
#define USECS_PER_BYTE_TRANSFERED           1


//  ---------------------------------------------------------------------

//  YOUR DATA STRUCTURES, VARIABLES, AND FUNCTIONS SHOULD BE ADDED HERE:

int timetaken = 0;
typedef struct _link
{
	struct _link* next;
	void* data;
} Link;
typedef struct _queue
{
	Link* front;
	Link* rear;
} Queue;
void insertInQueue(Queue* q, void* data)
{
	if(q->front == NULL)
	{
		q->front = (Link*)malloc(sizeof(Link));
		q->front->next = NULL;
		q->front->data = data;
		q->rear = q->front;
		return;
	}
	q->rear->next = (Link*)malloc(sizeof(Link));
	q->rear = q->rear->next;
	q->rear->next = NULL;
	q->rear->data = data;
}
void* removeFromQueue(Queue* q)
{
	if(q->front == NULL)
		return NULL;
	else if(q->front->next == NULL)
	{
		void* data = q->front->data;
		free(q->front);
		q->front = NULL;
		q->rear = NULL;
		return data;
	}
	void* data = q->front->data;
	Link* temp = q->front;
	q->front = q->front->next;
	free(temp);
	return data;
}
Queue* readyQueue = NULL;
int sleepList[MAX_PROCESSES+1];
int runner = 1;
typedef struct _process
{
  int in_exec;
	Queue* instructions;
	Queue* waitings;
} Process;
Process processes[MAX_PROCESSES+1];
void loadFromSleepList()
{
	int i = 1;
	while(i<=MAX_PROCESSES)
	{
		if(sleepList[i] != 0 && timetaken <= sleepList[i])
		{
			int* data = (int*)malloc(sizeof(int));
			*data = i;
			insertInQueue(readyQueue, data);
			i = 1;
			timetaken += USECS_TO_CHANGE_PROCESS_STATE;
		}
		else
			i++;
	}
}
int performExecution(int cpuQuantum, int pipeSize)
{
  if(runner == 0)
	{
		// cpu is in free state
		int i;
		int minNode = -1;
		for(i = 1; i<=MAX_PROCESSES; i++)
			if(sleepList[i]!=0 && (minNode==-1 || sleepList[i]<sleepList[minNode]))
				minNode = i;
		if(minNode != -1 && timetaken <= sleepList[minNode])
		{
			timetaken = sleepList[minNode] + USECS_TO_CHANGE_PROCESS_STATE;
			sleepList[minNode] = 0;
			int* data = (int*)malloc(sizeof(int));
			*data = minNode;
			insertInQueue(readyQueue, data);
	  		return 1;
		}
		loadFromSleepList();
		int* pid = (int*)removeFromQueue(readyQueue);
		if(pid != NULL)
		{
			timetaken += USECS_TO_CHANGE_PROCESS_STATE;
			runner = *pid;
			free(pid);
      		return 1;
		}
		return 0;
	}
	int* top = (int*)processes[runner].instructions->front->data;
	switch(top[0])
	{
	case 1:
		if(top[1] > cpuQuantum)
		{
			timetaken += cpuQuantum + USECS_TO_CHANGE_PROCESS_STATE;
			top[1] -= cpuQuantum;
		}
		else
		{
			timetaken += top[1] + USECS_TO_CHANGE_PROCESS_STATE;
			free(removeFromQueue(processes[runner].instructions));
		}
		{
			int* data = (int*)malloc(sizeof(int));
			*data = runner;
			insertInQueue(readyQueue, data);
		}
		runner = 0;
	break;
	case 2:
		sleepList[runner] = timetaken+top[1]+USECS_TO_CHANGE_PROCESS_STATE;
    	free(removeFromQueue(processes[runner].instructions));
		runner = 0;
		timetaken += USECS_TO_CHANGE_PROCESS_STATE;
	break;
	case 3:
		timetaken += USECS_TO_CHANGE_PROCESS_STATE;
		{
			void* data = removeFromQueue(processes[runner].waitings);
			while(data != NULL)
			{
				timetaken += USECS_TO_CHANGE_PROCESS_STATE;
				insertInQueue(readyQueue, data);
				data = removeFromQueue(processes[runner].waitings);
			}
		}
	    free(removeFromQueue(processes[runner].instructions));
	    processes[runner].in_exec = 0;
		runner = 0;
	break;
	case 4:
		timetaken += USECS_TO_CHANGE_PROCESS_STATE*2;
		{
			int* data = (int*)malloc(sizeof(int));
			*data = top[1];
			insertInQueue(readyQueue, data);
			data = (int*)malloc(sizeof(int));
		    *data = runner;
			insertInQueue(readyQueue, data);
		}
    	free(removeFromQueue(processes[runner].instructions));
		runner = 0;
	break;
	case 5:
		timetaken += USECS_TO_CHANGE_PROCESS_STATE;
		if(processes[top[1]].in_exec)
    	{
			int* data = (int*)malloc(sizeof(int));
			*data = runner;
			insertInQueue(processes[top[1]].waitings, data);
		}
	    else
	    {
	      		int* data = (int*)malloc(sizeof(int));
				*data = runner;
				insertInQueue(readyQueue, data);
	    }
	    free(removeFromQueue(processes[runner].instructions));
	    runner = 0;
	break;
	case 6:
		timetaken += USECS_TO_CHANGE_PROCESS_STATE;
		{
	  		int* data = (int*)malloc(sizeof(int));
			*data = runner;
			insertInQueue(readyQueue, data);	
		}
		free(removeFromQueue(processes[runner].instructions));
		runner = 0;
	break;
	case 7: case 8:
		timetaken += USECS_TO_CHANGE_PROCESS_STATE*2 + top[2]*USECS_PER_BYTE_TRANSFERED;
		free(removeFromQueue(processes[runner].instructions));
		{
	  		int* data = (int*)malloc(sizeof(int));
			*data = runner;
			insertInQueue(readyQueue, data);	
		}
		runner = 0;
	break;
	}
  return 1;
}
void freeAll()
{
	void* data = removeFromQueue(readyQueue);
	while(data != NULL)
	{
		free(data);
		data = removeFromQueue(readyQueue);
	}
	free(readyQueue);
	for(int i = 1; i<=MAX_PROCESSES; i++)
	{
		data = removeFromQueue(processes[i].instructions);
		while(data != NULL)
		{
			free(data);		
			data = removeFromQueue(processes[i].instructions);
		}
		free(processes[i].instructions);
		data = removeFromQueue(processes[i].waitings);
		while(data != NULL)
		{
			free(data);		
			data = removeFromQueue(processes[i].waitings);
		}
		free(processes[i].waitings);		
	}
}
//  ---------------------------------------------------------------------

//  FUNCTIONS TO VALIDATE FIELDS IN EACH eventfile - NO NEED TO MODIFY
int check_PID(char word[], int lc)
{
    int PID = atoi(word);

    if(PID <= 0 || PID > MAX_PROCESSES) {
        printf("invalid PID '%s', line %i\n", word, lc);
        exit(EXIT_FAILURE);
    }
    return PID;
}

int check_microseconds(char word[], int lc)
{
    int usecs = atoi(word);

    if(usecs <= 0) {
        printf("invalid PID '%s', line %i\n", word, lc);
        exit(EXIT_FAILURE);
    }
    return usecs;
}

int check_descriptor(char word[], int lc)
{
    int pd = atoi(word);

    if(pd < 0 || pd >= MAX_PIPE_DESCRIPTORS_PER_PROCESS) {
        printf("invalid pipe descriptor '%s', line %i\n", word, lc);
        exit(EXIT_FAILURE);
    }
    return pd;
}

int check_bytes(char word[], int lc)
{
    int nbytes = atoi(word);

    if(nbytes <= 0) {
        printf("invalid number of bytes '%s', line %i\n", word, lc);
        exit(EXIT_FAILURE);
    }
    return nbytes;
}

//  parse_eventfile() READS AND VALIDATES THE FILE'S CONTENTS
//  YOU NEED TO STORE ITS VALUES INTO YOUR OWN DATA-STRUCTURES AND VARIABLES
void parse_eventfile(char program[], char eventfile[])
{
#define LINELEN                 100
#define WORDLEN                 20
#define CHAR_COMMENT            '#'

//  ATTEMPT TO OPEN OUR EVENTFILE, REPORTING AN ERROR IF WE CAN'T
    FILE *fp    = fopen(eventfile, "r");

    if(fp == NULL) {
        printf("%s: unable to open '%s'\n", program, eventfile);
        freeAll();
        exit(EXIT_FAILURE);
    }

    char    line[LINELEN], words[4][WORDLEN];
    int     lc = 0;

//  READ EACH LINE FROM THE EVENTFILE, UNTIL WE REACH THE END-OF-FILE
    while(fgets(line, sizeof line, fp) != NULL) {
        ++lc;

//  COMMENT LINES ARE SIMPLY SKIPPED
        if(line[0] == CHAR_COMMENT) {
            continue;
        }

//  ATTEMPT TO BREAK EACH LINE INTO A NUMBER OF WORDS, USING sscanf()
        int nwords = sscanf(line, "%19s %19s %19s %19s",
                                    words[0], words[1], words[2], words[3]);

//  WE WILL SIMPLY IGNORE ANY LINE WITHOUT ANY WORDS
        if(nwords <= 0) {
            continue;
        }

//  ENSURE THAT THIS LINE'S PID IS VALID
        int thisPID = check_PID(words[0], lc);

//  OTHER VALUES ON (SOME) LINES
        int otherPID, nbytes, usecs, pipedesc;

//  IDENTIFY LINES RECORDING SYSTEM-CALLS AND THEIR OTHER VALUES
//  THIS FUNCTION ONLY CHECKS INPUT;  YOU WILL NEED TO STORE THE VALUES
        if(nwords == 3 && strcmp(words[1], "compute") == 0) {
            usecs   = check_microseconds(words[2], lc);
            int* data = (int*)malloc(sizeof(int)*2);
            data[0] = 1; // representing compute instruction;
            data[1] = atoi(words[2]);
            insertInQueue(processes[atoi(words[0])].instructions, (void*)data);
        }
        else if(nwords == 3 && strcmp(words[1], "sleep") == 0) {
            usecs   = check_microseconds(words[2], lc);
            int* data = (int*)malloc(sizeof(int)*2);
            data[0] = 2; // representing sleep instruction;
            data[1] = atoi(words[2]);
            insertInQueue(processes[atoi(words[0])].instructions, (void*)data);
        }
        else if(nwords == 2 && strcmp(words[1], "exit") == 0) {
            int* data = (int*)malloc(sizeof(int));
            data[0] = 3; // representing exit instruction;
            insertInQueue(processes[atoi(words[0])].instructions, (void*)data);
        }
        else if(nwords == 3 && strcmp(words[1], "fork") == 0) {
            otherPID = check_PID(words[2], lc);
            int* data = (int*)malloc(sizeof(int)*2);
            data[0] = 4; // representing fork instruction;
            data[1] = atoi(words[2]);
            insertInQueue(processes[atoi(words[0])].instructions, (void*)data);
        }
        else if(nwords == 3 && strcmp(words[1], "wait") == 0) {
            otherPID = check_PID(words[2], lc);
            int* data = (int*)malloc(sizeof(int)*2);
            data[0] = 5; // representing wait instruction;
            data[1] = atoi(words[2]);
            insertInQueue(processes[atoi(words[0])].instructions, (void*)data);
        }
        else if(nwords == 3 && strcmp(words[1], "pipe") == 0) {
            pipedesc = check_descriptor(words[2], lc);
            int* data = (int*)malloc(sizeof(int)*2);
            data[0] = 6; // representing pipe instruction;
            data[1] = atoi(words[2]);
            insertInQueue(processes[atoi(words[0])].instructions, (void*)data);
        }
        else if(nwords == 4 && strcmp(words[1], "writepipe") == 0) {
            pipedesc = check_descriptor(words[2], lc);
            nbytes   = check_bytes(words[3], lc);
            int* data = (int*)malloc(sizeof(int)*3);
            data[0] = 7; // representing writepipe instruction;
            data[1] = atoi(words[2]);
            data[2] = atoi(words[3]);
            insertInQueue(processes[atoi(words[0])].instructions, (void*)data);
        }
        else if(nwords == 4 && strcmp(words[1], "readpipe") == 0) {
            pipedesc = check_descriptor(words[2], lc);
            nbytes   = check_bytes(words[3], lc);
            int* data = (int*)malloc(sizeof(int)*3);
            data[0] = 8; // representing readpipe instruction;
            data[1] = atoi(words[2]);
            data[2] = atoi(words[3]);
            insertInQueue(processes[atoi(words[0])].instructions, (void*)data);
        }
//  UNRECOGNISED LINE
        else {
            printf("%s: line %i of '%s' is unrecognized\n", program,lc,eventfile);
            freeAll();
            exit(EXIT_FAILURE);
        }
    }
    fclose(fp);

#undef  LINELEN
#undef  WORDLEN
#undef  CHAR_COMMENT
}

//  ---------------------------------------------------------------------

//  CHECK THE COMMAND-LINE ARGUMENTS, CALL parse_eventfile(), RUN SIMULATION
int main(int argc, char *argv[])
{
	if(argc != 4)
	{
		printf("%s <eventfile> <cpu-time> <pipe-size>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	int cpuQuantum = atoi(argv[2]);
	int pipeSize = atoi(argv[3]);
	readyQueue = (Queue*)malloc(sizeof(Queue));
	readyQueue->front = NULL;
	readyQueue->rear = NULL;
	int i;
	for(i = 1 ; i<=MAX_PROCESSES; i++)
	{
		processes[i].instructions = (Queue*)malloc(sizeof(Queue));
		processes[i].instructions->front = NULL;
		processes[i].instructions->rear = NULL;
		processes[i].waitings = (Queue*)malloc(sizeof(Queue));
		processes[i].waitings->front = NULL;
		processes[i].waitings->rear = NULL;
		sleepList[i] = 0;
    processes[i].in_exec = 1;
	}
	parse_eventfile(argv[0], argv[1]);
	while(performExecution(cpuQuantum, pipeSize));
  	printf("timetaken %i\n", timetaken);
  	freeAll();
  	return 0;
}
