#include <buffer.h>
#include <tui.h>
#include <loop.h>
#include <signal.h>
//#include <term.h>

FILE_BUFFER *buffer;

void sigsegv_handler(void)
{
	for (;;);
}

int main(int argc, char *argv[])
{
	//signal(SIGSEGV, sigsegv_handler);

//	init_append_buf();
	
//	term_raw_mode_on() == 0;

	//term_clear_screen();
	//term_cursor_forward(3);
//	term_erase_screen(1);
//	term_cursor_pos(35, 5, 1);

//	term_print_screen(1);

//	term_cursor_down(3, 1);

//	release_append_buf();	
//	return 0;
	
	buffer = init_buffer("LICENSE");

	//print_buffer(buffer);
	//end_tui(buffer);
	/*insert_item("I as", 4, 2, buffer);
	insert_item(" Work", 5, 289, buffer);

	insert_item("HELLO", 5, 67, buffer);
	insert_item("3HELLO2", 7, 72, buffer);
	insert_item("test", 4, 72, buffer);

	insert_item(" BYE!", 5, 289, buffer);
	insert_item("BYE !", 5, 200, buffer);
	delete_item(68, 2, buffer);
	insert_item("yo yo", 5, 0, buffer);
	insert_item("more", 4, 5, buffer);*/
	fill_lines(buffer, 1);
	//dec_lineno(buffer);
	//printf("\n\n\n\n\n\n");

	//print_buffer(buffer);

	//fill_lines(buffer, 0);

	//print_lines(buffer);
	
	//init_tui(buffer);

	//end_tui(buffer);

	loop(buffer);
	
	release_buffer(buffer);

	return 0;
}

