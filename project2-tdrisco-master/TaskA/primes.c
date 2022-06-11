//Timothy Driscoll  2/22/20

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

//prime_sieve function which is used to implement the prime sieve algorithm
//and fork to make multiple processes in order to compute the prime numbers
//the function recuirsvly calls itself
void prime_sieve()
	{
		//Define needed variables
		int fd[2];
		int primes;
		int nums;
		int count;
		int pid;

		//Read the prime, (first number from each updated list)
		read(0,&primes,sizeof(primes));
		printf("PID = %d Prime = %d\n",getpid(),primes);
		//int prime = primes;
		fflush(stdout);
		count = 0;

		while(1)
			{
				//printf("%d\n",prime);
				//read from stdin and check to make sure data was read
				if(read(0,&nums,sizeof(nums)) <= 0)
					{
						//printf("going to break\n");
						break;
					}
				//printf("%d mod %d\n",nums,primes);
				//Determine if the number is divisible
				if(nums%primes != 0)
					{
						//printf("%d\n",nums);
						//if first number for the process
						if(count == 0)
							{
								//setup pipe
								count = 1;
								pipe(fd);
								pid = fork();
								//child process
								if(pid == 0)
									{
										close(0);
									  dup(fd[0]);
										close(fd[0]);
									  close(fd[1]);
										prime_sieve();
									} else{
										close(fd[0]);
									}

								}
							//Parent process after the fork has occured
							if (pid>0 && count == 1)
								{
									write(fd[1],&nums,sizeof(nums));
								}
							}
						}

					if(pid >0)
						{
							close(fd[1]);
							wait(0);
						}

			exit(0);

		}


int main(int argc, char *argv[]) {

	//Declare file descriptors and setup the pipes
	int fd[2];
	pipe(fd);
	int N = 0;
	int nums = 0;


	// get the value of N from command line argument
	//Deal with the command line arguments
	if(argc == 2)
		{
			N = atoi(argv[1]);
		}
	else
		{
			// Check for correct command line parameters
					fprintf(stderr,"Error: Bad command line parameters\n");
					return(1);
		}

		//printf("%d\n",N);

	//Do Child process stuff
	if(fork() == 0)
		{
			//Things to do: Setup Pipes, redirect stdin, and compute prime
			close(0);
			dup(fd[0]);
			close(fd[0]);
			close(fd[1]);
			prime_sieve();
			//while(read(fd[0],&nums,sizeof(nums)) > 0)
				//{
				//	printf("PID = %d NUM = %d\n",getpid(),nums);
			//	}

		}
	//Do parent process stuff
	else
		{
			close(fd[0]);
			for(nums = 2; nums <= N; nums++)
				{
					write(fd[1],&nums,sizeof(nums));
				}
			close(fd[1]);
			wait(0);
		}
	exit(0);
}
