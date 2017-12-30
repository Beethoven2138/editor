#ifndef COMMON_H
#define COMMON_H

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#define BLOCK_SIZE 512
#define PAGE_SIZE 4096

#define NR_TEXT 30

#define NR_LINES 66
#define NR_COLS 250

#define CACHE_SIZE 10
#define GAP_SIZE 50

#define MAX_UINT 1844674407370955161

#define MAGIC 0xBAD1234

#define LINE_CACHE_SIZE 10 /*number of lines that will be copied 
			     before and after the visible lines*/

/* comment out the line below to remove all the debugging assertion */
/* checks from the compiled code.  */
#define DEBUG_ASSERT 1

void Assert(int assertion, char *error);



enum
{
	
	HOME_KEY=0,
	END_KEY=1,
	ESC_KEY=2,
	PAGE_UP_KEY=3,
	PAGE_DOWN_KEY=4,
	BACKSPACE_KEY = 8,
        TAB_KEY = 9,
	ENTER_KEY = 10,

	DEL_KEY = 127,

	ARROW_UP = 256,
	ARROW_DOWN = 257,
	ARROW_RIGHT = 258,
	ARROW_LEFT = 259,

	F1_KEY = 260,
	F2_KEY=261,
	F3_KEY=262,
	F4_KEY=263,
	F5_KEY=264,
	F6_KEY=265,
	F7_KEY=267,
	F8_KEY=268,
	F9_KEY=269,
	F10_KEY=270,
	F11_KEY=280,
	F12_KEY=281,
};

#endif
