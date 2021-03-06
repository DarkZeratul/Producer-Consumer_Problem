/* 100277865 - dribeirobarbos00 - Douglas Ribeiro Barbosa
 * Dr. Ardeshir Bagheri
 * CPSC 2280 - 001
 * Lab 08: 19 March 2018
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>		/* For O_* constants */
#include <sys/stat.h>	/* For mode constants */
#include <semaphore.h>

#define SHM_SIZE 4096 // PAGE_SIZE on i386 machines

int main(int argc, char* argv[])
{
	printf("Producer is running.\n");

	char* input;
	FILE * pFile;

	key_t key;
	int shm_handle;
	char * shm_ptr;

	sem_t* sem_buffer_handle;
	sem_t* sem_myFlag_handle;

	int * bytesRead;
	int done = 0;

	// Gets the name of the input file.
	if (argc > 1)
	{
		input = argv[1];
	}

	// Opens the input file as binary for reading.
	pFile = fopen(input, "rb");
	if (pFile == NULL)
	{
		printf("Couldn't open %s", input);
		return 0;
	}

	// Generates a System V IPC key.
	key = ftok("makefile", 1);
	if (key == -1)
	{
		fclose(pFile);
		printf("Couldn't generate a System V IPC key.\n");
		return 0;
	}

	// Creates the shared memory block.
	shm_handle = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
	if (shm_handle == -1)
	{
		fclose(pFile);

		printf("Couldn't create the shared memory block.\n");
		return 0;
	}

	// Maps the shared memory block into the process' address space.
	shm_ptr = (char*)shmat(shm_handle,0,0);
	if (shm_ptr == (char*)-1)
	{
		fclose(pFile);
		shmctl(shm_handle, IPC_RMID, 0);

		printf("Couldn't map the shared memory block into the process' address space.\n");
		return 0;
	}

	// Creates a new POSIX semaphore or opens an existing semaphore.
	sem_buffer_handle = sem_open("sem_buffer", O_CREAT, S_IRWXU, 1);
	sem_myFlag_handle = sem_open("sem_myFlag", O_CREAT, S_IRWXU, 0);
	if (sem_buffer_handle == SEM_FAILED || sem_myFlag_handle == SEM_FAILED)
	{
		fclose(pFile);
		shmdt(shm_ptr);
		shmctl(shm_handle, IPC_RMID, 0);

		printf("Couldn't map the shared memory block into the process' address space.\n");
		return 0;
	}

	bytesRead = (int*)(shm_ptr + 1);

	*bytesRead = fread(shm_ptr + 5, 1, (SHM_SIZE - 5), pFile);
	*shm_ptr = '1';

	sem_post(sem_myFlag_handle);

	// ### CRITICAL SECTION

	while (done == 0)
	{
		sem_wait(sem_myFlag_handle);

		if (*shm_ptr == '0')	// Buffer is NOT Full
		{
			sem_wait(sem_buffer_handle);

			*bytesRead = fread(shm_ptr + 5, 1, (SHM_SIZE - 5), pFile);
			if (feof(pFile))
			{
				*shm_ptr = '2';
				done = 1;
			}
			else
			{
				*shm_ptr = '1';
			}
			sem_post(sem_buffer_handle);
		}

		sem_post(sem_myFlag_handle);
	}

	// ### CRITICAL SECTION

	shmdt(shm_ptr); // Detaches the shared memory segment.

	shmctl(shm_handle, IPC_RMID, 0); // Destroys the shared memory segment.

	fclose(pFile);

	printf("Producer has ended.\n");

	return 0;
}

