#include <common.h>

void Assert(int assertion, char* error)
{
	if(!assertion) {
		printf("Assertion Failed: %s\n",error);
		exit(-1);
	}
}
