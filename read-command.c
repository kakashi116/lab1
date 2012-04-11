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
  char* buffer = checked_malloc(sizeof(char));
  command_stream_t current = NULL, head = NULL;
  int i = 0;
  int startFromLine = 0;

  int c = get_next_byte(get_next_byte_argument);
  while (c != EOF) { // EOF
	  if (c == '\n') { // new line
	  		++startFromLine;
		  //  Check the previous character to see if it's a special character
		  if(!continueOnNewLine(buffer[i])){
			  buffer[i] = '\0';
			  // add new command_stream
			  command_stream_t newNode = checked_malloc(sizeof(command_stream_t));
			  newNode->startFromLine = startFromLine;
			  newNode->token = buffer;
			  newNode->next = NULL;

			  if (head == NULL) { // first time insertion
				  head = newNode;
				  current = head;
			  } else { // insert at the end
				  current->next = newNode;
				  current = current->next;
			  }
		  } 
	  } else {
		  buffer[i] = c;
		  ++i;
		  checked_realloc(buffer, sizeof(char) * (i + 1));
	  }
	  c = get_next_byte(get_next_byte_argument);
  }

  // save the last one
  ++startFromLine;
  command_stream_t newNode = checked_malloc(sizeof(command_stream_t));
  newNode->startFromLine = startFromLine;
  newNode->token = buffer;
  newNode->next = NULL;
  current->next = newNode;

  return head;
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
	if (s == NULL)
		return 0;

	char *buffer1 = malloc(sizeof(char));
	char *buffer2 = malloc(sizeof(char));
	int charBufferCount1 = 0;
	int charBufferCount2 = 0;
	int insideSubShell = 0;
	command_t command = checked_malloc(sizeof(command_t));

	int commandLen = strlen(s->token);

	// scan for ; | && || in order
	// make sure that it's not inside a subshell
	int i;
	for (i = 0; i < commandLen; ++i) {
		if (s->token[i] == '(') ++insideSubShell;
		if (s->token[i] == ')') --insideSubShell;

		if (!insideSubShell && (s->token[i] == ';' || s->token[i] == '|' || s->token[i] == '&')) {
			switch (s->token[i]) {
			case ';':
				command->type = SEQUENCE_COMMAND;
				break;
			case '|':
				command->type = PIPE_COMMAND;
				if (s->token[i+1] != '\0' && s->token[i+1] == '|') {
					command->type = OR_COMMAND;
					++i;
				}
				break;
			case '&':
				if (s->token[i+1] != '\0' && s->token[i+1] == '&') {
					command->type = AND_COMMAND;
					++i;
				} else {
					printf("%s", s->token);
					exit(1);
					fprintf (stderr, "Line %d: syntax error", s->startFromLine);
				}
				break;
			}
			command_stream_t command_stream1 = checked_malloc(sizeof(command_stream_t));
			checked_realloc(buffer1, sizeof(char)* (charBufferCount1+1));
			buffer1[charBufferCount1] = '\0';
			command_stream1->token = buffer1;
			command_stream1->startFromLine = s->startFromLine;
			command->u.command[0] = read_command_stream(command_stream1);

			// put the other half to the u.command[1]
			for (++i; i < commandLen; ++i) {
				checked_realloc(buffer2, sizeof(char)* (charBufferCount2+1));
				buffer2[charBufferCount2] = s->token[i];
				++charBufferCount2;
			}
			command_stream_t command_stream2 = checked_malloc(sizeof(command_stream_t));
			checked_realloc(buffer2, sizeof(char)* (charBufferCount2+1));
			buffer1[charBufferCount2] = '\0';
			command_stream2->token = buffer2;
			command_stream2->startFromLine = s->startFromLine;
			command->u.command[1] = read_command_stream(command_stream2);

			s = s->next;
			return command;
		}
		checked_realloc(buffer1, sizeof(char)* (charBufferCount1+1));
		buffer1[charBufferCount1] = s->token[i];
		++charBufferCount1;
	}

	// simple command


	command->type = SIMPLE_COMMAND;
	command->u.word = "test";
	s = s->next;
	return command;

/*
	//

	int i;
	for (i = 0; i < commandLen; ++i) {



		switch (s->token[i]) {
			case '(':
				int parenOpen = 1;
				int parenClose = 1;
				type = SUBSHELL_COMMAND;
				// wrap content between the parentheses
				++i;
				int charBufferCount = 0;
				int k;
				for (k = i; i < commandLen; ++i) {
					switch (s->token[i]) {
					case '(':
						++parenOpen;
						break;
					case ')':
						++parenClose;
						if (parenClose == parenOpen) {
							command->type = type;
						}
						break;
					default:
						buffer1 = realloc(buffer1, sizeof(char)* (charBufferCount+1));
						buffer1[charBufferCount] = s->token[i];
					}

				}
			break;

			case '&':
				++specialCharCount;
				type = AND_COMMAND;
				checkSpecialChar(&specialCharCount, s->startFromLine);
			break;

			case ';':
				++specialCharCount;
				type = SEQUENCE_COMMAND;
				checkSpecialChar(&specialCharCount, s->startFromLine);
			break;

			case '|':
				++specialCharCount;
				if (specialCharCount == 1)
					type = PIPE_COMMAND;
				else
					type = OR_COMMAND;
				checkSpecialChar(&specialCharCount, s->startFromLine);
			break;

			case ' ': // space
			break;

			default:
				if (specialCharCount != 0) {
					int j;
					for (j = 0; i < commandLen; ++j, ++i) {
						buffer2 = realloc(buffer2, sizeof(char) * (j + 1));
						buffer2[j] = command[i];
					}
					//TODO add buffer to command1 & command2
					command->type = SIMPLE_COMMAND;

					//TODO recall read_command on command2
					break;
				}
				buffer1 = realloc(buffer1, sizeof(char)* (charBufferCount+1));
				buffer1[charBufferCount] = command[i];
				++charBufferCount;
		}
	}
	*/

}

