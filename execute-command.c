// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include "sys/types.h"
#include "sys/wait.h"
#include "stdio.h"
#include <string.h>
#include <error.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int
command_status (command_t c)
{
  return c->status;
}

void sysError()
{
	fprintf(stderr, "%s\n", strerror(errno));
	exit(3);
}

// IO

void io(command_t c)
{
	if (c->input) {
		int fin = open(c->input, O_RDONLY);
		// map the fin to the standard input
		dup2(fin, 0);
		close(fin);
	}

	if (c->output) {
		int fout = open(c->output, O_WRONLY|O_CREAT|O_TRUNC, 0666);
		// map the output to the standard output
		dup2(fout, 1);
		close(fout);
	}
}

// SIMPLE_COMMAND
void execute_simple(command_t c)
{
	pid_t pid = fork();
	int status;

	if (pid == -1) { // error
		sysError();
	} else if (pid == 0) { // child
		if (c->u.word[0][0] != ':') {
			io(c);
			if (execvp(c->u.word[0], c->u.word) == -1)
				sysError();
		} else
			exit(0); // : return true only and does nothing
	}

	// parent
	// wait for the child to finish
	if (waitpid(pid, &status, 0) == -1)
		sysError();

	// save exit status
	c->status = WIFEXITED(status);
}

// AND_COMMAND
void execute_and(command_t c)
{
	execute_command(c->u.command[0]);

	if (c->u.command[0]->status == 0) { // the && command returns true
		execute_command(c->u.command[1]);
		c->status = c->u.command[1]->status;
	} else { // if the first command return false, we don't need to execute the second one
		c->status = c->u.command[0]->status;
	}
}

// SEQUENCE_COMMAND
void execute_sequence(command_t c)
{
	execute_command(c->u.command[0]);
	execute_command(c->u.command[1]);
	c->status = c->u.command[1]->status;
}

// OR_COMMAND
void execute_or(command_t c)
{
	execute_command(c->u.command[0]);
	if (c->u.command[0]->status == 0) { // the first one returns true
		// no need to execute the second command
		c->status = c->u.command[0]->status;
	} else {
		execute_command(c->u.command[1]);
		c->status = c->u.command[1]->status;
	}
}

// PIPE_COMMAND
/*
 * child 1 --> | pipefd[1]     pipefd[0] | --> child 2
 * whatever is written to pipefd[1] will be read from pipefd[0]
 */

void execute_pipe(command_t c)
{
	// create a pipe
	int pipefd[2];
	if (pipe(pipefd) == -1)
		sysError();

	// first child run the first command
	pid_t pid_write = fork();
	if (pid_write == -1) {
		sysError();
	} else if (pid_write == 0) { // child
		// map standard out of this child to the write end of the pipe
		dup2(pipefd[1], 1);

		// we don't need the input end
		close(pipefd[0]);

		// execute the first command
		execute_command(c->u.command[0]);

		// exit here
		exit(c->u.command[0]->status);
	}

	// second child
	pid_t pid_read = fork();
	if (pid_read == -1) {
		sysError();
	} else if (pid_read == 0) {
		// read from pipefd[0]
		// the end of this pipe becomes standard output
		dup2(pipefd[0], 0);

		// does not use the other end
		close(pipefd[1]);

		/* the exec'd programs will inherit the standard stream from its spawners
		 * It means the input of this child will be the input of the newly exec'd programs
		 */
		execute_command(c->u.command[1]);

		// exit here
		exit(c->u.command[1]->status);
	}

	// parent
	// important! we close the descriptors so the reading child knows when to exit
	close(pipefd[0]);
	close(pipefd[1]);

	// wait for children to complete
	int status_write;
	int status_read;
	waitpid(pid_write, &status_write, 0);
	waitpid(pid_read, &status_read, 0);

	if (!WIFEXITED(status_write) || !WIFEXITED(status_read))
		sysError();

	// update the status
	c->status = WEXITSTATUS(status_read);
}

// SUBSHELL_COMMAND
void execute_subshell(command_t c)
{
	io(c);
	execute_command(c->u.subshell_command);
	c->status = c->u.subshell_command->status;
}

void
execute_command (command_t c)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */

	if (c->type == SIMPLE_COMMAND) {
		execute_simple(c);
	} else if (c->type == AND_COMMAND) {
		execute_and(c);
	} else if(c->type == SEQUENCE_COMMAND) {
		execute_sequence(c);
	} else if(c->type == OR_COMMAND) {
		execute_or(c);
	} else if(c->type == PIPE_COMMAND) {
		execute_pipe(c);
	} else if(c->type == SUBSHELL_COMMAND) {
		execute_subshell(c);
	} else {
		error(1, 0, "Unknown error!");
	}
}

/*
 * Use dependency graph to execute commands in parallel
 * @return command_t last command
 */
command_t execute_timetravel(command_t c)
{
	// construct a 2D array to

}
