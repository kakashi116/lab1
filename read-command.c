// UCLA CS 111 Lab 1 command reading

#include "command-internals.h"
#include "command.h"
#include "alloc.h"
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool isWord(char *a){
  return isalnum(a)||a=='!'||a=='%'||a=='+'||a==','||a=='-'
         ||a=='.'||a=='/'||a==':'||a=='@'||a=='^'||a=='_';
}
//checks the next byte in the buffer without popping off the byte.
char next(int (*get_next_byte) (void *),
	  void *get_next_byte_argument, char *n)
{
  if(*n==NULL) 
    *n = get_next_byte(get_next_byte_argument);
  return *n;
}
//pops next letter off stream
void pop(int (*get_next_byte) (void *),
	  void *get_next_byte_argument, char *n)
{
  if (*n==NULL)
    get_next_byte(get_next_byte_argument);
  *n=NULL;
}
//skips all spaces and tabs if n=space or tab. returns with n=next
void skipST(int (*get_next_byte) (void *),
	  void *get_next_byte_argument, char *n)
{
  while(n==' '||n=='\t')
  {
    pop(get_next_byte, get_next_byte_argument,*n);
    next(get_next_byte, get_next_byte_argument,*n);
  }
}
//if next is word, use getWord to parse word until next is not a word.
char* getWord(int (*get_next_byte) (void *),
	  void *get_next_byte_argument, char *n)
{
  char buf[256];
  int i =0;
  while(isWord(*n))
  {
    buf[i]=*n;
    i++;
    pop(get_next_byte,get_next_byte_argument,*n);
    next(get_next_byte,get_next_byte_argument,*n);
  }
  buf[i]='\0';
  return buf;
}

struct token{
  char* data;
  struct token *ptr;
};

void create_token(char *token, struct token *head, struct token *current)
{
  struct *token new = checked_malloc(sizeof(struct token));
  new->data=token;
  new->ptr=NULL;
  if(head==NULL)
    head=new;
  else
    current->ptr=new;
  current=new;
}
command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  char n;
  next(get_next_byte, get_next_byte_argument, *n);
  skipST(get_next_byte,get_next_byte_argument);
  int line = 1;
  struct *token head = NULL, current = NULL;
  while(n!=EOF)
  {
    if(isWord(n))
    {
      create_token(getWord(get_next_byte,get_next_byte_argument,*n), head, current);
    }
    else
      switch(n){
	case '(':
	  create_token("(", head, current);
	  pop(get_next_byte, get_next_byte_argument, *n);
	  break;
	case ')':
	  create_token(")", head, current);
	  pop(get_next_byte, get_next_byte_argument, *n);
	  break;
	case '<':
	  create_token("<", head, current);
	  pop(get_next_byte, get_next_byte_argument, *n);
	  break;
	case '>':
	  create_token(">", head, current);
	  pop(get_next_byte, get_next_byte_argument, *n);
	  break;
	case ';':
	  create_token(";", head, current);
	  pop(get_next_byte, get_next_byte_argument, *n);
	  break;
	case '|':
	  pop(get_next_byte, get_next_byte_argument, *n);
	  if(next(get_next_byte,get_next_byte_argument)=='|')
	  {
	    create_token("||", head, current);
	    pop(get_next_byte,get_next_byte_argument,*n);
	  }
	  else
	    create_token("|", head, current);
	  break;
	case '&':
	  if(get_next_byte(get_next_byte_argument)=='&')
	  {
	    create_token("&&", head, current);
	    break;
	  }
	  else
	  {
	    error(1,0,"error in line%n\n", line)
	  }  
	case '\n'
	  create_token("\n",head,current);
	  break;
	default:
	  error(1,0,"error in line %n\n", line)
      }
  next(get_next_byte,get_next_byte_argument,*n);
  skipST(get_next_byte,get_next_byte_argument);
  }

  //delete token stream
  current=head;
  while(head->ptr!=NULL)
  {
    current=current->ptr;
    free(head);
    head=current;
  }
  free(head);
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

