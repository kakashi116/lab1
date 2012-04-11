// UCLA CS 111 Lab 1 command reading

#include "command-internals.h"
#include "command.h"
#include "alloc.h"
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int continueOnNewLine(int c) {
	if (c == '|' || c == '&')
			return 1;
	return 0;
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */

}

void expect(char expectChar, char currentChar, int* i, int* commandLen) {
	if (i == commandLen || currentChar != expectChar) {
		fprintf (stderr, "Line %d: syntax error", 0);
		exit(1);
	}
}


command_t
read_command_stream (command_stream_t s)
{
	while (s != NULL) {
		command_stream_t current = s;
		s = s->next;
		return current->my_command;
	}

	return 0;
}

