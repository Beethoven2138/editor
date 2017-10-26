#include <buffer.h>
#include <tui.h>
#include <loop.h>

int main(int argc, char *argv[])
{
	FILE_BUFFER *buffer = init_buffer("Makefile");

	//print_buffer(buffer);
	/*end_tui(buffer);
	insert_item("I", 1, 288, buffer);
	insert_item(" Work", 5, 289, buffer);

	insert_item("HELLO", 5, 67, buffer);
	insert_item("3HELLO2", 7, 72, buffer);
	insert_item("test", 4, 72, buffer);

	insert_item(" BYE!", 5, 289, buffer);
	insert_item("BYE !", 5, 200, buffer);
	delete_item(68, 2, buffer);
	insert_item("yo yo", 5, 0, buffer);
	insert_item("more", 4, 5, buffer);
	//printf("\n\n\n\n\n\n");

	print_buffer(buffer);*/

	//fill_lines(buffer, 0);

	//print_lines(buffer->lines);
	
	//init_tui(buffer);

	//end_tui(buffer);

	loop(buffer);
	
	release_buffer(buffer);

	return 0;
}

