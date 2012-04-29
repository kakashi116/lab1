#include<stdlib.h>
#include<stdio.h>

struct list_el {
   char* word;
};

typedef struct list_el *mystruct;

void change(struct list_el *st)
{
	st = NULL;
}


int main(){
	mystruct test = malloc(sizeof(struct list_el));
	test->word = "1";
	change(test);
	if (test == NULL) {
		puts("fail");
		exit(1);
	}
	puts(test->word);
}
