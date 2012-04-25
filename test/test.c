#include<stdlib.h>
#include<stdio.h>

struct list_el {
   char* word;
};

void change(struct list_el **st) 
{
	(*st)->word = "2";
}


int main(){
	struct list_el *test = malloc(sizeof(struct list_el));
	test->word = "1";
	change(&test);
	if (test == NULL) {
		puts("fail");
		exit(1);
	}
	puts(test->word);
}
