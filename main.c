/*
NAME = USAMA QURESHI
*/


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>


void logStart(char* tID);//function to log that a new thread is started
void logFinish(char* tID);//function to log that a thread has finished its time

void startClock();//function to start program clock
long getCurrentTime();//function to check current time since clock was started
time_t programClock;//the global timer/clock for the program

typedef struct thread //represents a single thread, you can add more members if required
{
	char tid[4];//id of the thread as read from file
	unsigned int startTime;
	int state;
	pthread_t handle;
	int retVal;
	int flag;      // to check on critical section
} Thread;

char tmp[4] = "noV";    // global variable to save last thread id.

sem_t *sphore;   // sempahore POSIX
//sphore= sem_open("SPHORE", O_CREAT, 0666, 1);
int count = 0;

int integerValue(char *arr);   // return integer value of ThreadID
int threadsLeft(Thread* threads, int threadCount);
int threadToStart(Thread* threads, int threadCount);
void* threadRun(void* t);//the thread function, the code executed by each thread
int readFile(char* fileName, Thread** threads);//function to read the file content and build array of threads

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Input file name missing...exiting with error code -1\n");
		return -1;
	}


	
	sphore = sem_open("SPHORE", O_CREAT, 0666, 1);


	//sem_init(sphore, 0, 1);

	Thread* threads = NULL;
	int threadCount = readFile(argv[1], &threads);

	startClock();



	while (threadsLeft(threads, threadCount) > 0)
	{
		
		int i = 0;
		while ((i = threadToStart(threads, threadCount)) > -1)
		{
		

			threads[i].state = 1;
			//	sem_wait(sphore);


			threads[i].retVal = pthread_create(&(threads[i].handle), NULL, threadRun, &threads[i]);

			
			pthread_join(threads[i].handle, NULL);

			//	sem_post(sphore);
		}

		//	sem_destroy(sphore);
		if (getCurrentTime() > 20)
		{
			break;
		}
	}
	sem_destroy(sphore);
	return 0;
}

int readFile(char* fileName, Thread** threads)
{
	FILE *in = fopen(fileName, "r");
	if (!in)
	{
		printf("Child A: Error in opening input file...exiting with error code -1\n");
		return -1;
	}

	struct stat st;
	fstat(fileno(in), &st);
	char* fileContent = (char*)malloc(((int)st.st_size + 1) * sizeof(char));
	fileContent[0] = '\0';
	while (!feof(in))
	{
		char line[100];
		if (fgets(line, 100, in) != NULL)
		{
			strncat(fileContent, line, strlen(line));
		}
	}
	fclose(in);

	char* command = NULL;
	int threadCount = 0;
	char* fileCopy = (char*)malloc((strlen(fileContent) + 1) * sizeof(char));
	strcpy(fileCopy, fileContent);
	command = strtok(fileCopy, "\r\n");
	while (command != NULL)
	{
		threadCount++;
		command = strtok(NULL, "\r\n");
	}
	*threads = (Thread*)malloc(sizeof(Thread)*threadCount);

	char* lines[threadCount];
	command = NULL;
	int i = 0;
	command = strtok(fileContent, "\r\n");
	while (command != NULL)
	{
		lines[i] = malloc(sizeof(command) * sizeof(char));
		strcpy(lines[i], command);
		i++;
		command = strtok(NULL, "\r\n");
	}

	for (int k = 0; k < threadCount; k++)
	{
		char* token = NULL;
		int j = 0;
		token = strtok(lines[k], ";");
		while (token != NULL)
		{
	

			(*threads)[k].flag = 0;



			(*threads)[k].state = 0;
			if (j == 0)
				strcpy((*threads)[k].tid, token);
			if (j == 1)
				(*threads)[k].startTime = atoi(token);
			j++;
			token = strtok(NULL, ";");
		}
	}
	return threadCount;
}

void logStart(char* tID)
{
	count++;
	printf("[%ld] New Thread with ID %s is started.\n", getCurrentTime(), tID);
}

void logFinish(char* tID)
{
	printf("[%ld] Thread with ID %s is finished.\n", getCurrentTime(), tID);
}

int threadsLeft(Thread* threads, int threadCount)
{
	int remainingThreads = 0;
	for (int k = 0; k < threadCount; k++)
	{
		if (threads[k].state > -1)
			remainingThreads++;
	}
	return remainingThreads;
}

int threadToStart(Thread* threads, int threadCount)
{
	for (int k = 0; k < threadCount; k++)
	{
		if (threads[k].state == 0 && threads[k].startTime == getCurrentTime())
			return k;
	}
	return -1;
}

void* threadRun(void* t)
{
	logStart(((Thread*)t)->tid);

	
	//sem_wait(sphore);

	//if(strcmp(tmp, ((Thread*)t)->tid)!=0)
	if (integerValue(((Thread*)t)->tid) == 0)  //first thread
	{
		strcpy(tmp, ((Thread*)t)->tid);
		sem_wait(sphore);
		((Thread*)t)->flag = 1;


		printf("[%ld] Thread %s is in its critical section\n", getCurrentTime(), ((Thread*)t)->tid);
		//critical section ends here
	//	sem_post(sphore);
		sem_post(t);     

	//	printf("The id of last thread %s\n\n", tmp);
	}
	if ((integerValue(tmp) % 2) != (integerValue(((Thread*)t)->tid) % 2))     // one is even and other will be odd
	{
		strcpy(tmp, ((Thread*)t)->tid);
		sem_wait(sphore);      //my code

		((Thread*)t)->flag = 1;

		//critical section starts here
		printf("[%ld] Thread %s is in its critical section\n", getCurrentTime(), ((Thread*)t)->tid);
		//critical section ends here


		sem_post(sphore);     //my code

	//	printf("The id of last thread %s\n\n", tmp);
	}

	/*
	 if ((count > 4)&&((((Thread*)t)->state) == 1) && ((integerValue(((Thread*)t)->tid)==7)|| (integerValue(((Thread*)t)->tid) == 5))) {
		printf("[%ld] Thread %s is in its critical section\n", getCurrentTime(), ((Thread*)t)->tid);
		((Thread*)t)->flag = 1;
	}

	if ((getCurrentTime() > 5)&& ((integerValue(((Thread*)t)->tid) == 7) || (integerValue(((Thread*)t)->tid) == 5)))
	{
		printf("[%ld] Thread %s is in its critical section\n", getCurrentTime(), ((Thread*)t)->tid);
		((Thread*)t)->flag = 1;

	}
	*/

	/

	//sem_post(sphore);

//your synchronization logic will appear here

	if ((((Thread*)t)->flag) == 1)
	{
		logFinish(((Thread*)t)->tid);
		((Thread*)t)->state = -1;

	}

	pthread_exit(0);
}

void startClock()
{
	programClock = time(NULL);
}

long getCurrentTime()//invoke this method whenever you want check how much time units passed
//since you invoked startClock()
{
	time_t now;
	now = time(NULL);
	return now - programClock;
}

int integerValue(char * arr)
{
	int value;

	value = (int)(arr[1]) + (int)(arr[2]);
	return value;

}