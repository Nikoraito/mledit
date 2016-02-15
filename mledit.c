#include <ncurses.h>
#include <string.h>

#define WIDTH 40
#define HEIGHT 40

#define ENTER 10
#define ESCAPE 27

#define TRUE 1
#define FALSE 0


enum STATUS{	
	OKAY,			//All operations normal
	WRONG_KEY,		//Unrecognized key hit in whatever mode
	READ_FAIL,		//Unable to read file
	WRITE_FAIL,		//Unable to write to file
	NAME_FAIL,		//Unspecified failure about names
	MISC_ERROR,		//Anything else
	QUITTING,		//Quitting without a prompt
	QUIT_ASK,		//Prompting to save and stuff before quitting
};
	
enum MODE{	
	META,			//Read, Write, file io, etc.
	NAVIGATION,		//The default mode, moves through the tags	
	TAG_EDIT,		//Edit the type and attributes of tags
	CONTENT_EDIT,	//Edit the content INSIDE tags
	TAG_SEARCH,		//primed with # from Navigation; goto a tag with id #(name)
};

enum TAB_MODES{
	SPACES,
	TABS,
	NONE,
};
	


	
char * filename;
char * buffer;
char * filebuffer;

char current_line[2048];
char current_tag[1024];


unsigned short status = OKAY;
unsigned short cur_mode = NAVIGATION;	//Start off in navigation mode...

unsigned short override = FALSE;
unsigned short running = TRUE;
unsigned short tab_mode = 0; // 0 for spaces,  for \t.

unsigned short c = 0;

unsigned int tab_size = 4; // default number of spaces to a tab
unsigned int cursor = 0;	
unsigned int line = 0;
unsigned int indent = 0;

int main(){
	initscr();
	cbreak();
	keypad(stdscr, TRUE);
	noecho();
	
	FILE *fp;	
	
	int y = 0, x = 0;
	
	
	
	start_color();
	init_pair(1, COLOR_BLACK, COLOR_GREEN);
	init_pair(2, COLOR_RED, COLOR_WHITE);
	init_pair(3, COLOR_WHITE, COLOR_RED);
	
	while(running){		
	
		
		pstat();
		pmode();		
		
		//mvprintw(1, 0, "UP: %d, DOWN: %d, LEFT: %d, RIGHT: %d", KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT);
		
		switch(cur_mode){
			
			case CONTENT_EDIT: 
				c_edit();
				update();
				break;
			
			case TAG_EDIT:
				t_edit();
				update();
				break;
			
			case NAVIGATION: 
				nav();
				update();
				break;
			
			case META: 
				m_menu();
				break;
			default: break;
			
		}
	}	
	
	endwin();	
	return 0;
}

int c_edit(){
	c = getch();
	
	status = OKAY;
	
	switch(c){
		case ENTER:
			
			break;
		
		case ESCAPE:
			cur_mode = TAG_EDIT;
			break;
			
		default: 
			fprintf(stderr, "Before print");
			strcat(current_tag, (char*)c);
			fprintf(stderr, "After Print"); 
			break;
		
	}	
	
	return 0;
}

int t_edit(){
	c = getch();
	
	status = OKAY;
	
	switch(c){
		case KEY_RIGHT:
			if(cursor < strlen(current_line)-1) cursor++;
			break;
		case KEY_LEFT:
			if(cursor > 0) cursor--;
			break;
		case ENTER:
			cur_mode = CONTENT_EDIT;
			break;
		
		case ESCAPE:
			cur_mode = NAVIGATION;
			break;
			
		default: 
			strcat(current_tag, (char*)c);
			strcat(current_line, current_tag);
			break;
		
	}	
	return 0;
}


int m_menu(){
	char cmd[64];
	
	status = OKAY;
	
	echo();
	mvprintw(1,0, ":");
	getnstr(cmd, 64);
	
	int i = 0;
	
	if (cmd[0] == '#'){
		tag_search(cmd);	
	}
	
	while(cmd[i] != '\0'){
		switch(cmd[i++]){
			case 'q': 
				if(override) status = QUITTING;
				else status = QUIT_ASK;
				break;
			
			case '!': 
				if (status == QUIT_ASK) status = QUITTING;
				else override = TRUE;
				break;
			
			case 'w':
				write_file();
				break;
				
			case 'o':
				prompt_file();
				break;
				
			case ESCAPE:
			cur_mode = NAVIGATION;
			break;
		}
	}
	
	if(status == QUITTING){
		running = FALSE;
	}
	
	cur_mode = NAVIGATION;
	noecho();
	return 0;
}


int nav(){
	
	c = getch();
	
	status = OKAY;
	
	switch(c){
		
		case ':': 
			cur_mode = META;
			break;
		
		case ENTER:
			cur_mode = TAG_EDIT;
			break;
			
		case 'N':
			break;	//CODE TO MAKE A NEW TAG AND GO INTO TAG-EDIT MODE HERE
		
		case 'E':
			break;	//CODE TO EDIT A TAG
		
		case 'R':	//CODE TO DELETE A TAG - PROMPT ABOUT DELETING CONTENTS AS WELL
			break;

		case 'D':
			break;	//CODE TO DUPLICATE A TAG
		
		case KEY_UP:
			if(line-1 >= 0){
				line--;
			}
			break;

		case KEY_DOWN:
			if(line+1 <= gtamnt(indent)){
				line++;
			}
			break;
			
		case KEY_LEFT:
			indent--;
			break;
			
		case KEY_RIGHT:
			indent++;
			break;

		default: 
			status = WRONG_KEY;
			break;
		
	}	
	
	return 0;
}

int gtamnt(int indent){
	
}

int pstat(){
	move(0, 0);
	clrtoeol();
	
	printw("Status: ");
	
	switch (status){
		case OKAY: 
			attron(COLOR_PAIR(1));
			printw("OK");
			attroff(COLOR_PAIR(1));
			break;
			
		case WRONG_KEY:
			attron(COLOR_PAIR(2));
			printw("Wrong key? Entered: %d", c);
			attroff(COLOR_PAIR(2));
			break;
			
		case READ_FAIL:
			attron(COLOR_PAIR(3));
			printw("Could not read file.");
			attroff(COLOR_PAIR(3));
			break;
			
		case WRITE_FAIL:
			attron(COLOR_PAIR(3));
			printw("Could not write to file.");
			attroff(COLOR_PAIR(3));
			break;
			
		case NAME_FAIL:
			attron(COLOR_PAIR(3));
			printw("Name invalid.");
			attroff(COLOR_PAIR(3));
			break;
			
		case QUIT_ASK:
			attron(COLOR_PAIR(3));
			printw("Really Quit? (Y/N)");
			attroff(COLOR_PAIR(3));
			c = getch();
			if(c == 'y' || c == 'Y') running = FALSE;
			break;
		
		default:
			attron(COLOR_PAIR(3));
			printw("UNDEFINED ERROR.");
			attroff(COLOR_PAIR(3));
			break;
	}
	return 0;
}

int pmode(){
	move(HEIGHT-1,0);
	clrtoeol();
	
	printw("Mode: ");
	
	switch(cur_mode){
		case NAVIGATION:
			printw("Navigation");
			break;
		case TAG_EDIT:
			printw("Tag editing");
			break;
		case CONTENT_EDIT:
			printw("Content editing");
			break;
		case META:
			printw("Meta menu");
		default : break;
	}
}

int write_file(){
	return 0;
	
}

int prompt_file(){
	return 0;

}

int tag_search(char * t){
	return -1;
}

int update(){
	//This function refreshes the relevent text from buffer on the screen
	mvprintw(3, indent*tab_size, current_line);
}
