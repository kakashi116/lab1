#include<stdlib.h>
#include<stdio.h>

struct list_el {
   char *word;
};

int main(){
	char *buffer = malloc(sizeof(char));
	buffer[0] = '1';
	struct list_el test;
	test->word = buffer;
}
