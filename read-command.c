// UCLA CS 111 Lab 1 command reading

#include "command-internals.h"
#include "command.h"
#include "alloc.h"
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int isWord(char a) {
	if (isalnum((int) a))
		return 1;
	if (a == '!' || a == '%' || a == '+' || a == ',' || a == '-' || a == '.'
			|| a == '/' || a == ':' || a == '@' || a == '^' || a == '_')
		return 1;
	return 0;
}
/*//checks the next byte in the buffer without popping off the byte.
 void next(int (*get_next_byte) (void *),
 void *get_next_byte_argument, char *n)
 {
 if(n==NULL)
 n = (get_next_byte(get_next_byte_argument));
 return *n;
 }*/
//pops next letter off stream
void pop(int(*get_next_byte)(void *), void *get_next_byte_argument, char *n) {
	*n = (char) get_next_byte(get_next_byte_argument);
}
//skips all spaces and tabs if n=space or tab. returns with n=next
void skipST(int(*get_next_byte)(void *), void *get_next_byte_argument, char *n) {
	while (*n == ' ' || *n == '\t') {
		pop(get_next_byte, get_next_byte_argument, n);
	}
}
//if next is word, use getWord to parse word until next is not a word.
void getWord(int(*get_next_byte)(void *), void *get_next_byte_argument,
		char *n, char **temp) {
	char buf[256];
	int i = 0;
	while (isWord(*n)) {
		buf[i] = *n;
		i++;
		pop(get_next_byte, get_next_byte_argument, n);
	}
	buf[i] = '\0';
	*temp = (char*) checked_malloc(sizeof(char) * i + 1);
	strcpy(*temp, buf);
}

struct token {
	char* data;
	struct token *next; // next
	int isWord; // 1 if it's a word, 0 otherwise
};

typedef struct token *token_t;

void create_token(char *token, struct token **head, struct token **current, int isWord) {
	struct token *new1 = (struct token*) checked_malloc(sizeof(struct token));
	new1->data = (char*) (checked_malloc(sizeof(char) * strlen(token)));
	new1->isWord = isWord;
	strcpy(new1->data, token);
	new1->next = NULL;
	if (*head == NULL)
		*head = new1;
	else
		(*current)->next = new1;
	*current = new1;
}

command_t build_single_command(token_t *current) {
	command_t command = checked_malloc(sizeof(struct command));
	command->type = SIMPLE_COMMAND;
	command->status =-1;
	command->input = 0;
	command->output = 0;
	int i = 1;
	char** w = checked_malloc(sizeof(char*) * 2);
	w[0] = (*current)->data;

	// check to see if the next token is a word
	*current = (*current)->next;

	while (*current != NULL && (*current)->isWord) {
		w = checked_realloc(w, sizeof(char*) * (i + 2));
		w[i] = (*current)->data;
		++i;
		*current = (*current)->next;
	}

	w[i] = NULL;

	command->u.command[0] = NULL;
	command->u.command[1] = NULL;
	command->u.word = w;
	return command;
}

command_t build_sub_shell(command_t *subshell) {
	command_t command = checked_malloc(sizeof(struct command));
	command->type = SUBSHELL_COMMAND;
	command->status = -1;
	command->input = 0;
	command->output = 0;
	command->u.subshell_command = *subshell;
	return command;
}


command_t build_command(enum command_type type, command_t a, command_t b) {
  command_t temp = checked_malloc(sizeof(struct command));
  temp->type = type;
  temp->status = -1;
  if (type == SUBSHELL_COMMAND){
	  temp->u.subshell_command = a;
  } else {
	  temp->u.command[0] = a;
	  temp->u.command[1] = b;
  }
  return temp;
}


command_t left_associative(enum command_type type, command_t a, command_t b) {
	// in a sub shell command, the second argument 'b' is always set to NULL
	if (type == SEQUENCE_COMMAND || type == PIPE_COMMAND || b == NULL || b->type == SUBSHELL_COMMAND || b->type == SIMPLE_COMMAND)
		return build_command(type, a, b);
	else {
		return build_command(b->type, left_associative(type, a, b->u.command[0]), b->u.command[1]);
	}
}

// expect a specific token
// return 0 if not found
int expect(int isWord, char* token, token_t *current) {
	if (*current == NULL)
		return 0;

	if (isWord)
		return ((*current)->isWord);
	else
		return (!strcmp(token, (*current)->data));
}

// used to output error
void outputError(int *onLine){
	error(1, 0, "error in line %d\n", *onLine);
}

// read IO
void readIO(command_t *command, token_t *current, int *currentLine) {
	// input
	if (!strcmp("<", (*current)->data)) {;
		*current = (*current)->next;
		if (expect(1, NULL, current)) {
			(*command)->input = (*current)->data;
			*current = (*current)->next;
		} else
			outputError(currentLine);
	}

	// output
	if (*current != NULL && !strcmp(">", (*current)->data)) {
		*current = (*current)->next;
		if (expect(1, NULL, current)) {
			(*command)->output = (*current)->data;
			*current = (*current)->next;
		} else
			outputError(currentLine);
	}
}

command_t make_tree(token_t *current, int *currentLine) {
	command_t first;

	// ignore newlines before a word or a subshell command
	while(*current != NULL && !strcmp((*current)->data, "\n")) {
		*current = (*current)->next;
		++(*currentLine);
	}

	if (*current == NULL)
		return NULL;

	if(expect(1, NULL, current)) { // expect a word
		first = build_single_command(current);
	} else if (expect(0, "(", current)) { // or a subshell command
		*current = (*current)->next;
		command_t subshell;
		subshell = make_tree(current, currentLine);

		if (subshell == NULL)
			outputError(currentLine);

		first = build_sub_shell(&subshell);
		if (!expect(0, ")", current))
			outputError(currentLine);
		else
			*current = (*current)->next;
	} else // error
		outputError(currentLine);

	// read IO
	if (*current != NULL)
		readIO(&first, current, currentLine);

	if (*current == NULL || !strcmp(")", (*current)->data))
		return first;

	if (!strcmp("\n", (*current)->data)) {
		*current = (*current)->next;
		++(*currentLine);
		return first;
	}

	command_t second = checked_malloc(sizeof(struct command));
	enum command_type type;
		
	if (!strcmp("&&", (*current)->data))
		type = AND_COMMAND;
	else if(!strcmp(";", (*current)->data))
		type = SEQUENCE_COMMAND;
	else if(!strcmp("||", (*current)->data))
		type = OR_COMMAND;
	else if(!strcmp("|", (*current)->data))
		type = PIPE_COMMAND;
	else
		outputError(currentLine);

	*current = (*current)->next;
	second = make_tree(current, currentLine);

	if (second == NULL)
		outputError(currentLine);

	return left_associative(type, first, second);
	//return build_command(type, first, second);
	
}

command_stream_t make_command_stream(int(*get_next_byte)(void *),
		void *get_next_byte_argument) {
	/* FIXME: Replace this with your implementation.  You may need to
	 add auxiliary functions and otherwise modify the source code.
	 You can also use external functions defined in the GNU C Library.  */
	char n;
	pop(get_next_byte, get_next_byte_argument, &n);
	skipST(get_next_byte, get_next_byte_argument, &n);
	int line = 1;
	char *temp = NULL;
	struct token *head = NULL, *current = NULL;
	while (n != EOF) {
		if (isWord(n)) {
			getWord(get_next_byte, get_next_byte_argument, &n, &temp);
			create_token(temp, &head, &current, 1);
			free(temp);
		} else
			switch (n) {
			case '#': // comment
				// ignore this line
				while (n != '\n' && n != EOF)
					pop(get_next_byte, get_next_byte_argument, &n);
			break;

			case '(':
				create_token("(\0", &head, &current, 0);
				pop(get_next_byte, get_next_byte_argument, &n);
				break;
			case ')':
				create_token(")\0", &head, &current, 0);
				pop(get_next_byte, get_next_byte_argument, &n);
				break;
			case '<':
				create_token("<\0", &head, &current, 0);
				pop(get_next_byte, get_next_byte_argument, &n);
				break;
			case '>':
				create_token(">\0", &head, &current, 0);
				pop(get_next_byte, get_next_byte_argument, &n);
				break;
			case ';':
				create_token(";\0", &head, &current, 0);
				pop(get_next_byte, get_next_byte_argument, &n);
				break;
			case '|':
				pop(get_next_byte, get_next_byte_argument, &n);
				if (n == '|') {
					create_token("||\0", &head, &current, 0);
					pop(get_next_byte, get_next_byte_argument, &n);
				} else
					create_token("|\0", &head, &current, 0);
				break;
			case '&':
				pop(get_next_byte, get_next_byte_argument, &n);
				if (n == '&') {
					create_token("&&\0", &head, &current, 0);
					pop(get_next_byte, get_next_byte_argument, &n);
					break;
				} else {
					error(1, 0, "error in line %d\n", line);
				}
				break;

			case '\n':
				create_token("\n\0", &head, &current, 0);
				pop(get_next_byte, get_next_byte_argument, &n);
				break;

			default:
				error(1, 0, "error in line %d\n", line);
				break;
			}
		skipST(get_next_byte, get_next_byte_argument, &n);
	}
	current = head;

	/*
	while (current!= NULL) {
		printf("%s \n", current->data);
		current = current->next;
	}

	//delete token stream
	current = head;
	while (current != NULL) {
		head = current->next;
		free(current);
		current = head;
	}*/

	// construct the tree
	int currentLine = 1;
	command_stream_t tree_head = NULL, tree_current = NULL;

	while (current != NULL) {
		command_t complete_command = make_tree(&current, &currentLine);

		if (complete_command == NULL)
			break;

		// new node
		command_stream_t command_node = checked_malloc(sizeof(struct command_stream));

		command_node->next = NULL;
		command_node->previous = NULL;
		command_node->current_command = complete_command;

		if (tree_head == NULL) { // first node
			tree_head = tree_current = command_node;
		} else {
			tree_current->next = command_node;
			command_node->previous = tree_current;
			tree_current = tree_current->next;
		}

	}

	return tree_head;
}

command_t read_command_stream(command_stream_t *s) {
	while (*s != NULL) {
		command_stream_t current_stream = *s;
		*s = (*s)->next;

		// free the previous node
		if (current_stream->previous != NULL) {
			free(current_stream->previous->current_command);
			free(current_stream->previous);
		}

		return current_stream->current_command;
	}

	return NULL;
}

