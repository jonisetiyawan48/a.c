#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(char argc, char *argv[])
{
	int pfds[2]; // 0 is read end, 1 is write end
	int wfd;
	pid_t pid;
	char cmd[100];
	int cmdlen;
	int stop, status;

	if (pipe(pfds) == -1)
	{
		perror(argv[0]);
		return 1;
	}

	pid = fork();
	if (pid == -1)
	{
		perror(argv[0]);
		return 2;
	}
	else if (pid != 0) // Parent
	{
		close(pfds[0]); // Close the read end of the pipe
		wfd = pfds[1];
		// Continue doing other stuff, e.g.
		// take commands from user and pass onto the master script
		stop = 0;
		do
		{
			printf("Cmd (type \"done\" to exit): ");
			if ((status = scanf("%[^\n]", cmd)) <= 0)
			{
				getchar(); // Remove the \n
				continue;
			}
			getchar(); // Remove the \n
			if (strcmp(cmd, "done") == 0)
			{
				stop = 1;
			}
			else
			{
				cmdlen = strlen(cmd);
				cmd[cmdlen++] = '\n';
				//cmd[cmdlen] = '\0';
				// Pass on the command to master script
				write(wfd, cmd, cmdlen);
			}
		}
		while (!stop);
		close(wfd);
	}
	else
	{
		close(pfds[1]); // Close the write end of the pipe
		dup2(pfds[0], 0); // Make stdin the read end of the pipe
		if (execl("./master_script.sh", "master_script.sh", (char *)(NULL))
				== -1)
		{
			perror("Master script process spawn failed");
		}
	}

	return 0;
}
