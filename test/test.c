/*
 * test.c
 *
 *  Created on: Apr 30, 2012
 *      Author: Liem Do
 */


void gain_memory(char ***ptr) {
	*ptr = (char **) realloc(*ptr, sizeof(char*) * 3);
	(*ptr)[2] = "b2323";
}

int main()
{
	char **ptr = malloc(sizeof(char*));
	gain_memory(&ptr);
	printf("%s", ptr[2]);
	return 0;
}
