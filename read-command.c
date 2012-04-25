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

command_t build_single_command(struct token **current) {
	command_t command = checked_malloc(sizeof(struct command));
	command->type = SIMPLE_COMMAND;
	command->status = -1;
	char** w = checked_malloc(sizeof(char*) + 1);
	w[0] = (*current)->data;
	w[1] = NULL;
	command->u.command[0] = NULL;
	command->u.command[1] = NULL;
	command->u.word = w;
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
	if (b == NULL || b->type == SUBSHELL_COMMAND || b->type == SIMPLE_COMMAND)
		return build_command(type, a, b);
	else {
		return build_command(b->type, left_associative(type, a, b->u.command[0]), b->u.command[1]);
	}
}


command_t make_tree(struct token **current) {
	command_t first;
	char *input = NULL;
	char *output = NULL;
	
	if ((*current)->isWord) {
		// detect I/O
		if ((*current)->next != NULL && !strcmp((*current)->next->data, "<")) { // input
			input = (*current)->data;
			*current = (*current)->next->next; // skip 2 tokens [input] [>]
			first = make_tree(current);
			first->input = input;
		} else {
			first = build_single_command(current);
			*current = (*current)->next;
		}
		
		if (*current == NULL || !strcmp((*current)->data, ")")) {
			return first;
		}
	}

	command_t second = checked_malloc(sizeof(struct command));
	enum command_type type;

	if ((*current)->isWord == 0){
		// detect I/O
		if (!strcmp((*current)->data, "(")) { // subshell
			command_t subshell;
			*current = (*current)->next;
			subshell = make_tree(current);
			first = build_command(SUBSHELL_COMMAND, subshell, NULL);
			*current = (*current)->next;
			if (*current == NULL)
				return first;
		}

		if (!strcmp((*current)->data, ">")) // output
		{
			*current = (*current)->next;
			output = (*current)->data;
			first->output = output;
			*current = (*current)->next; // NULL
			return first;
		}
		
		if (!strcmp((*current)->data, "&&"))
			type = AND_COMMAND;
		else if(!strcmp((*current)->data, ";"))
			type = SEQUENCE_COMMAND;
		else if(!strcmp((*current)->data, "||"))
			type = OR_COMMAND;
		else if(!strcmp((*current)->data, "|"))
			type = PIPE_COMMAND;

		*current = (*current)->next;
	    second = make_tree(current);
	}
	

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
					error(1, 0, "error in line%d\n", line);
				}
			case '\n':
				create_token("\n\0", &head, &current, 0);
				pop(get_next_byte, get_next_byte_argument, &n);
				break;
			default:
				error(1, 0, "error in line %d\n", line);
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
	command_t complete_command = make_tree(&current);
	command_stream_t command_node = checked_malloc(sizeof(command_stream_t));

	command_node->next = NULL;
	command_node->previous = NULL;
	command_node->current_command = complete_command;
	//head_node = &command_node;

	return command_node;
}

void expect(char expectChar, char currentChar, int* i, int* commandLen) {
	if (i == commandLen || currentChar != expectChar) {
		fprintf(stderr, "Line %d: syntax error", 0);
		exit(1);
	}
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

