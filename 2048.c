#include <panel.h>
#include <stdlib.h>
#include <time.h>
#include <sys/select.h>

#define WIDTH 28 //latimea ferestrei de meniu
#define HEIGHT 9  //inaltimea ferestrei de meniu
#define S_WIDTH 8 //latimea unei celule
#define S_HEIGHT 6 //inaltimea unei celule

void print_menu(WINDOW *menu_win, int glow);
void print_board(WINDOW *win, int starty, int startx, int height, int width, int square_height, int square_width, int scor);
void init_board(int board[4][4]);
void fill_board(WINDOW *win, int board[4][4], int winner, int scor);
void game(int board[4][4], int dir, int *scor, int *winner);
void new_value(int board[4][4]);
int valid_move(int board[4][4], int dir, int *scor, int *winner, int mode);
int gameover(int board[4][4]);
void gameover_print(WINDOW *win, int *running_game, PANEL *game_panel, PANEL *menu_panel, PANEL *bkgr_panel);
int automove(WINDOW *win, int board[4][4], int *scor, int *winner);
void backup_undo(int board[4][4], int *scor, int *winner, int undo[4][4], int *scor_backup, int *winner_backup, int mode);

int main()
{       WINDOW *menu_win, *bkgr_win, *game_win;
		PANEL *menu_panel, *bkgr_panel, *game_panel;
        int glow = 1; //glow pe optiunea selectata in meniu
        int choice; //optiunea selectata propriu-zis
        int key, autom; //tasta selectata && variabila pentru valoarea returnata de automove
        int n_choices = 3; //cele 3 optiuni din meniu
        int board[4][4], undo[4][4]; //matricele principala si pentru optiunea undo
        int scor, scor_backup, scor_test; 
        int winner, winner_backup, winner_test;
        int running_game = 0; //exista un joc incaput?

        //elemente necesare pentru mutarea automata dupa 8 secunde (functia select)
        int sel;
		fd_set read_descriptors;
		struct timeval timeout;

        system("resize -s 35 88"); //resizing-ul automat al ferestrei executabilului
        initscr(); //initializarea ecranului (ncurses)

        start_color(); //programul suporta culorile
        //definirea unor culori non-standard
        init_color(25, 0, 400, 0); //dark green - background
        init_color(26, 400, 0, 0); //dark red
        init_color(27, 500, 0, 500); //purple
        init_color(28, 500, 500, 0); //mustar
        init_color(29, 0, 500, 500); //blue-green
        init_color(30, 1000, 150, 0); //bright red
        init_color(31, 150, 1000, 0); //light green
        init_color(32, 150, 150, 150); //black

        //initializarea perechilor de culori
        init_pair(1, COLOR_RED, 25); 
        init_pair(2, 32, 25);
        init_pair(3, COLOR_WHITE, 25);
        init_pair(4, COLOR_BLUE, 25);
        init_pair(5, COLOR_YELLOW, 25);
        init_pair(6, COLOR_MAGENTA, 25);
        init_pair(7, 31, 25);
        init_pair(8, 26, 25);
        init_pair(9, 27, 25);
        init_pair(10, 28, 25);
        init_pair(11, 29, 25);
        init_pair(12, 30, 25);
        init_pair(13, COLOR_CYAN, 25);

        noecho(); //nu afisa pe ecran ce se tasteaza
        cbreak(); //dezactiveaza buffering-ul input-ului de la tastatura
        curs_set(0); //dezactiveaza cursorul
        int startx = (88 - WIDTH) / 2; //coordonate folosite frecvent in program
        int starty = (35 - HEIGHT) / 2 + 4;
                
        //initializarea ferestrelor si panourilor corespunzatoare
        menu_win = newwin(HEIGHT, WIDTH, starty, startx); //meniu
        bkgr_win = newwin(35, 88, 0, 0); //background meniu
        game_win = newwin(35, 88, 0, 0); //fereastra de joc
        keypad(menu_win, TRUE); //acceptarea folosirii tastelor sageti
        keypad(bkgr_win, TRUE);
        keypad(game_win, TRUE);
        keypad(stdscr, TRUE);
        bkgr_panel = new_panel(bkgr_win); //panoul background-ului meniului
        menu_panel = new_panel(menu_win); //panoul meniului
        game_panel = new_panel(game_win); //panoul jocului
        hide_panel(game_panel); //ascunderea panoului jocului

        //initializarea culorilor de background
        wbkgd(menu_win, COLOR_PAIR(3));
        wbkgd(bkgr_win, COLOR_PAIR(3));
        wbkgd(game_win, COLOR_PAIR(3));
        wattron(bkgr_win, A_BOLD); //functii folosite pentru activarea/dezactivarea atributelor
        //scrierea mesajului de welcome pe background
        mvwprintw(bkgr_win, 3, startx - 11, "Welcome to the C-ncurses version of the popular 2048!");
		wattron(bkgr_win, COLOR_PAIR(4));
		mvwprintw(bkgr_win, 8, 30, "   ___   ____  __ __  ____ "); //some ASCII Art
		mvwprintw(bkgr_win, 9, 30, "  |__ \\ / __ \\/ // / ( __ )");
		mvwprintw(bkgr_win, 10, 30, "  __/ // / / / // /_/ __  |");
		mvwprintw(bkgr_win, 11, 30, " / __// /_/ /__  __/ /_/ / ");
		mvwprintw(bkgr_win, 12, 30, "/____/\\____/  /_/  \\____/  ");
        wattroff(bkgr_win, A_BOLD | COLOR_PAIR(4));
        wattron(bkgr_win, A_ITALIC); 
        mvwprintw(bkgr_win, LINES - 3, startx - 19, "Use UP and DOWN arrows to navigate; Press ENTER to select an option.");
        wattroff(bkgr_win, A_ITALIC);
        mvwprintw(bkgr_win, LINES - 2, startx + 4, "© 2018 Rares Borcea");

        menu_label:

        //Se goleste multimea de lucru pentru functia select
		FD_ZERO(&read_descriptors);
		//Tastatura este adaugata la multime
		FD_SET(0, &read_descriptors);
		//Daca dupa un timp (timeout) nu au loc evenimente de la tastatura, 
		//atunci se returneaza valoarea 0
		timeout.tv_sec = 8; //dupa 8 secunde
		timeout.tv_usec = 0; //si 0 milisecunde		

        glow = 1; //glow pe primul element din meniu
        choice = 0; //selectarea primei optiuni
        print_menu(menu_win, glow); 
        while(1)
        {       
        		key = wgetch(menu_win);
                switch(key)
                {       
                		case KEY_UP:
                                {
                                	//trecerea de la inceputul la sfarsitul meniului
                                	if(glow == 1) glow = n_choices; 
                                	//excluderea optiunii Resume daca nu exista joc neterminat
                              	    else if(glow == 3) 
                                    	if(running_game) glow--;
                                    	else glow = glow - 2;
                                    else glow--;
                                	break;
                                }
                        case KEY_DOWN:
                                {
                                	//trecerea de la sfarsitul la inceputul meniului
                                	if(glow == n_choices) glow = 1;
                                	else if(glow == 1)
                                		if(running_game) glow++;
                                		else glow = glow + 2;
                                	else glow++;
                                	break;
                                }
                        case 10: //daca se apasa enter
                                { 
                                	choice = glow; //se face o selectie
                                	break; 
                                }
                }
                print_menu(menu_win, glow);
                if(choice != 0) //daca exista o selectie, iesi din while
                	break;
        }
        if(choice == 1) //s-a selectat *New Game*
        	{
        		//se reinitializeaza variabilele, panourile si ferestrele
        		winner = 0;
        		hide_panel(menu_panel);
        		hide_panel(bkgr_panel);
        		show_panel(game_panel);
        		scor = 0;
        		running_game = 1;
        		init_board(board);
        		fill_board(game_win, board, winner, scor);
        		backup_undo(board, &scor, &winner, undo, &scor_backup, &winner_backup, 0); //fa back-up la tabla
        		
        		resume_label:

        		while(1)
        		{
        			sel = select(1, &read_descriptors, NULL, NULL, &timeout);
        			switch(sel)
        			{
        				case 1:
        				{
        					key = getch();
        					switch(key)
        					{
        						case KEY_UP: //muta spre nord
        							{
        								if(valid_move(board, 0, &scor_test, &winner_test, 0)) //e valida mutarea?
        								{	
        									backup_undo(board, &scor, &winner, undo, &scor_backup, &winner_backup, 0); //fa back-up
        									valid_move(board, 0, &scor, &winner, 1); //fa mutarea
        									new_value(board); //introdu pe tabla o valoare aleatoare
        									fill_board(game_win, board, winner, scor); //printeaza tabla
        								}
        								if(gameover(board)) //mai exista mutari valide?
        									{
        										//daca nu, printeaza mesajul de gameover
        										gameover_print(game_win, &running_game, game_panel, menu_panel, bkgr_panel); 
        										goto menu_label;
        									}
        								break;
        							}
        						case KEY_DOWN: //muta spre sud
        							{
        								if(valid_move(board, 1, &scor_test, &winner_test, 0))
        								{	
        									backup_undo(board, &scor, &winner, undo, &scor_backup, &winner_backup, 0);
        									valid_move(board, 1, &scor, &winner, 1);
        									new_value(board);
        									fill_board(game_win, board, winner, scor);
        								}
        								if(gameover(board))
        									{
        										gameover_print(game_win, &running_game, game_panel, menu_panel, bkgr_panel);
        										goto menu_label;
        									}
        								break;
        							}
        						case KEY_RIGHT: //muta spre est
        							{
        								if(valid_move(board, 2, &scor_test, &winner_test, 0))
        								{	
        									backup_undo(board, &scor, &winner, undo, &scor_backup, &winner_backup, 0);
        									valid_move(board, 2, &scor, &winner, 1);
        									new_value(board);
        									fill_board(game_win, board, winner, scor);
        								}
        								if(gameover(board))
        									{
        										gameover_print(game_win, &running_game, game_panel, menu_panel, bkgr_panel);
        										goto menu_label;
        									}
        								break;
        							}
        						case KEY_LEFT: //muta spre vest
        							{
        								if(valid_move(board, 3, &scor_test, &winner_test, 0))
        								{	
        									backup_undo(board, &scor, &winner, undo, &scor_backup, &winner_backup, 0);
        									valid_move(board, 3, &scor, &winner, 1);
        									new_value(board);
        									fill_board(game_win, board, winner, scor);
        								}
        								if(gameover(board))
        									{
        										gameover_print(game_win, &running_game, game_panel, menu_panel, bkgr_panel);
        										goto menu_label;
        									}
        								break;
        							}
        						case 'Q': //iesi la meniu
        						case 'q': break;	
        						case 'A': //efectueaza automove
        						case 'a':
        						{
        							backup_undo(board, &scor, &winner, undo, &scor_backup, &winner_backup, 0);
        							autom = automove(game_win, board, &scor, &winner);
        							if(!autom)
        							{	
        								//daca nu se poate efectua, nu mai exista miscari valide
        								//deci iesi din sesiune
        								gameover_print(game_win, &running_game, game_panel, menu_panel, bkgr_panel);
        								goto menu_label;
        							}
        							break;
        						}
        						case 'U': //efectueaza undo
        						case 'u':
        						{
        							backup_undo(board, &scor, &winner, undo, &scor_backup, &winner_backup, 1); 
        							fill_board(game_win, board, winner, scor);
        							break;
        						}
        					}
        					break;
        				}

        				case 0: //nu se intampla nimic 8 secunde, apeleaza automove
        				{
        					backup_undo(board, &scor, &winner, undo, &scor_backup, &winner_backup, 0);
        					autom = automove(game_win, board, &scor, &winner);
        					if(!autom)
        					{
        						gameover_print(game_win, &running_game, game_panel, menu_panel, bkgr_panel);
        						goto menu_label;
        					}
        					break;
        				}
        			}

        			//reinitializeaza variabilele pentru select
        			FD_SET(0, &read_descriptors);
					timeout.tv_sec = 8;
					timeout.tv_usec = 0;

        			if(key == 'Q' || key == 'q') break; //s-a apasat q, iesi la meniu
        		}

        		//reafiseaza meniul
        		hide_panel(game_panel);
        		show_panel(bkgr_panel);
        		show_panel(menu_panel);
        		//actualizeaza ecranul
        		update_panels();
    			doupdate();
    			goto menu_label;
        	}
        else if (choice == 2) //s-a selectat *Resume*
        {
        	hide_panel(menu_panel);
        	hide_panel(bkgr_panel);
        	show_panel(game_panel);
        	update_panels();
    		doupdate();
        	goto resume_label;
        }
        else if (choice == 3) //s-a selectat *Quit*
        {
        	//”omoara” procesul jocului 2048
        	system("pkill 2048");
        }
        getch();
        //inchide fereastra ncurses
        endwin();
        return 0;
}

//funtie pentru printarea meniului
void print_menu(WINDOW *menu_win, int glow)
{
        int w_width, i, starty = 4;   
        int n_choices = 3;
        char *choices[] = { //cele trei optiuni disponibile
                        "New Game",
                        "Resume",
                        "Quit"
                 		  }; 
        //obtinerea dimensiunilor ferestrei
        getmaxyx(menu_win, i, w_width);
        box(menu_win, 0, 0); //incadrarea intr-un chenar a ferestrei
        int title_pos = (int)(w_width - 9) / 2; //calcularea pozitiei orizontale a titlului
        wattron(menu_win, A_BOLD | COLOR_PAIR(1));
        mvwprintw(menu_win, 1, title_pos, "%s", "MAIN MENU");
        wattroff(menu_win, A_BOLD | COLOR_PAIR(1));
        //printarea liniei de separare si a elementelor de imbinare
        mvwaddch(menu_win, 2, 0, ACS_LTEE); 
        mvwhline(menu_win, 2, 1, ACS_HLINE, w_width - 2);
        mvwaddch(menu_win, 2, w_width - 1, ACS_RTEE);


        for(i = 0; i < n_choices; i++)
        {       if(glow == i + 1) //glow pe alegerea curenta
                {       wattron(menu_win, A_REVERSE); 
                        mvwprintw(menu_win, starty, 2, "%s", choices[i]);
                        wattroff(menu_win, A_REVERSE);
                }
                else
                        mvwprintw(menu_win, starty, 2, "%s", choices[i]);
                starty++;
        }
        update_panels();
        doupdate();
}

//functie pentru printarea ferestrei de joc
void print_board(WINDOW *win, int starty, int startx, int height, int width, int square_height, int square_width, int scor)
{	
	int i, j, endy, endx;
	//calcularea pozitiilor de sfarsit pentru tabela folosind dimensiunile unei celule
	endy = starty + height * square_height;
	endx = startx + width  * square_width;

	wattron(win, A_BOLD);
	for(i = startx; i <= endx; i = i + square_width) //printarea liniilor verticale
		for(j = starty; j <= endy; j++)
			mvwaddch(win, j, i, ACS_VLINE);
	for(i = starty; i <= endy; i = i + square_height) //printarea liniilor orizontale
		for(j = startx; j <= endx; j++)
			mvwaddch(win, i, j, ACS_HLINE);
	//printrarea elementelor de la intersectia mai multor linii
	for(i = startx + square_width; i <= endx - square_width; i = i + square_width) //pentru top si bottom
	{	
		mvwaddch(win, starty, i, ACS_TTEE);
		mvwaddch(win, endy, i, ACS_BTEE);
	}
	for(i = starty + square_height; i <= endy - square_height; i = i + square_height) //pentru left si right
	{	
		mvwaddch(win, i, startx, ACS_LTEE);
		mvwaddch(win, i, endx, ACS_RTEE);	
		for(j = startx + square_width; j <= endx - square_width; j = j + square_width) //pentru celulele din interiorul tabelei
			mvwaddch(win, i, j, ACS_PLUS);
	}
	//pentru cele 4 colturi
	mvwaddch(win, starty, startx, ACS_ULCORNER);
	mvwaddch(win, endy, startx, ACS_LLCORNER);
	mvwaddch(win, starty, endx, ACS_URCORNER);
	mvwaddch(win, endy, endx, ACS_LRCORNER);

	//elementele necesare afisarii orei si datei (vezi lab-ul 09)
	time_t timer;
    char date_t[24];
    struct tm* ltime;
    time(&timer);
    ltime = localtime(&timer);
    strftime(date_t, 24, "    %d-%m-%Y   %H:%M", ltime); //retinerea datei si orei in sirul date_t

    wattron(win, COLOR_PAIR(4));
	mvwprintw(win, 7, 9, "   ___   ____  __ __  ____ "); //some other ASCII Art :-D
	mvwprintw(win, 8, 9, "  |__ \\ / __ \\/ // / ( __ )");
	mvwprintw(win, 9, 9, "  __/ // / / / // /_/ __  |");
	mvwprintw(win, 10, 9, " / __// /_/ /__  __/ /_/ / ");
	mvwprintw(win, 11, 9, "/____/\\____/  /_/  \\____/  ");
	wattroff(win, COLOR_PAIR(4));
	wattron(win, A_REVERSE);
	mvwprintw(win, 13, 9, "       Control Panel       ");
	wattroff(win, A_REVERSE | A_BOLD);
	mvwprintw(win, 14, 9, "   _ _ _ _ _ _ _ _ _ _ _");
	mvwprintw(win, 15, 9, "          ");
	wattron(win, A_BOLD);
	mvwprintw(win, 16, 9, "      Date and Time:");
	wattroff(win, A_BOLD);
	mvwprintw(win, 17, 9, "%s", date_t);
	mvwprintw(win, 18, 9, "          ");
	wattron(win, A_BOLD);
	mvwprintw(win, 19, 9, "          Score:");
	wattroff(win, A_BOLD);
	mvwprintw(win, 20, 9, "        %d points", scor);
	mvwprintw(win, 21, 9, "          ");
	wattron(win, A_BOLD);
	mvwprintw(win, 22, 9, "         Controls:");
	wattroff(win, A_BOLD);
	mvwprintw(win, 23, 9, " UP DOWN LEFT RIGHT - Moves"); //legenda tastelor utilizabile
	mvwprintw(win, 24, 9, "     A key - Automove");
	mvwprintw(win, 25, 9, "  U key - Undo last move");
	mvwprintw(win, 26, 9, "   Q - Quit to Main Menu");

	wattron(win, A_BOLD);
	for(i = 7; i <= 37; i = i + 30)
		for(j = 7; j <= 28; j++)
			mvwaddch(win, j, i, ACS_VLINE);
	for(i = 6; i <= 28; i = i + 22)
		for(j = 7; j <= 37; j++)
			mvwaddch(win, i, j, ACS_HLINE);

	mvwaddch(win, 6, 7, ACS_ULCORNER);
	mvwaddch(win, 28, 7, ACS_LLCORNER);
	mvwaddch(win, 6, 37, ACS_URCORNER);
	mvwaddch(win, 28, 37, ACS_LRCORNER);
	wattroff(win, A_BOLD);

	update_panels();
    doupdate();
}

//initializarea matricei de joc + crearea a 2 elemente aleatoare cu rand()
void init_board(int board[4][4])
{
	int i, j, i1, j1, i2, j2, g1 = 0, g2 = 0;
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			board[i][j] = 0;
	srand(time(NULL)); //pe baza timpului curen

	while(1) //pana cand gasesti 2 elemente potrivite
	{
		i1 = rand() % 4; //pozitii aleatoare
		j1 = rand() % 4;
		i2 = rand() % 4;
		j2 = rand() % 4;

		if(board[i1][j1] == 0) 
			{ 
				board[i1][j1] = (rand() % 2 + 1) * 2; //elementul aleator
				g1 = 1; 
			}
		if(board[i2][j2] == 0)
			{ 
				board[i2][j2] = (rand() % 2 + 1) * 2; 
				g2 = 1; 
			}
		if(g1 && g2) break;
		else
		{
			board[i1][j1] = 0;
			board[i2][j2] = 0;
			g1 = 0;
			g2 = 0;
		}
	}
}

//printarea matricei si a mesajului de win
void fill_board(WINDOW *win, int board[4][4], int winner, int scor)
{
	int i,j, nline, ncols;
	int startx, starty;

	getmaxyx(win, nline, ncols);

	starty = (nline - 4 * S_HEIGHT) / 2;
	startx = (ncols  - 4 * S_WIDTH) / 2 + 18;
	wclear(win); //curata fereastra
	print_board(win, starty, startx, 4, 4, S_HEIGHT, S_WIDTH, scor);

	if(winner) //afiseaza mesajul in cazul in care castiga
	{
		wattron(win, A_BOLD | COLOR_PAIR(1));
    	mvwprintw(win, nline - 5, startx + 10, "YOU WON, PAL!");
    	wattron(win, A_ITALIC);
    	mvwprintw(win, nline - 4, startx, "De-acum, e doar apa de ploaie! ;)");
    	wattroff(win, A_BOLD | COLOR_PAIR(1) | A_ITALIC);
	}

	wattron(win, A_BOLD);
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			if(board[i][j] != 0) //printarea colorata a elementelor matrice
				{
					if(board[i][j] == 2) wattron(win, COLOR_PAIR(1));
					if(board[i][j] == 4) wattron(win, COLOR_PAIR(2));
					if(board[i][j] == 8) wattron(win, COLOR_PAIR(4));
					if(board[i][j] == 16) wattron(win, COLOR_PAIR(5));
					if(board[i][j] == 32) wattron(win, COLOR_PAIR(6));
					if(board[i][j] == 64) wattron(win, COLOR_PAIR(7));
					if(board[i][j] == 128) wattron(win, COLOR_PAIR(8));
					if(board[i][j] == 256) wattron(win, COLOR_PAIR(9));
					if(board[i][j] == 512) wattron(win, COLOR_PAIR(10));
					if(board[i][j] == 1024) wattron(win, COLOR_PAIR(11));
					if(board[i][j] == 2048) wattron(win, COLOR_PAIR(12));
					if(board[i][j] == 4096) wattron(win, COLOR_PAIR(13));
					if(board[i][j] > 4096) wattron(win, COLOR_PAIR(3));
					mvwprintw(win, starty + i * S_HEIGHT + S_HEIGHT/2,
					 startx + j * S_WIDTH  + S_WIDTH/2,
					 "%d", board[i][j]);
				}
	wattrset(win, A_NORMAL); //dezactivarea atributelor pentru fereastra
	update_panels();
    doupdate();
}

//functia jocului propriu-zis
void game(int board[4][4], int dir, int *scor, int *winner)
{
	int i, j, k, g;
	if(dir == 0) //nord
	{
		for(j = 0; j < 4; j++)
		{
			for(i = 0; i < 4; i++)
			{
				g = i;
				//cat timp este nevalid si exista elemente pe tabla, muta-le succesiv
				while(board[i][j] == 0 && g < 3)
					{
						for(k = i; k < 3; k++)
							board[k][j] = board[k + 1][j];
						board[3][j] = 0; //si introdu noi celule libere la extremitate
						g++;
					}
				if(board[i][j] != 0)
				{
					if(i > 0) //exista campuri valide vecine in directia selectata?
						if(board[i - 1][j] == board[i][j]) //daca da, se pot combina?
						{
							*scor = *scor + 2 * board[i][j];
							board[i - 1][j] = 2 * board[i][j];
							if(board[i - 1][j] == 2048) *winner = 1; //verificare joc castigat
							board[i][j] = 0;
							g = i;
							//cat timp este nevalid si exista elemente pe tabla, muta-le succesiv
							while(board[i][j] == 0 && g < 3) 
							{
								for(k = i; k < 3; k++)
									board[k][j] = board[k + 1][j];
								board[3][j] = 0; //si introdu noi celule libere la extremitate
								g++;
							}	
						}
				}
			}
		}
	}

	if(dir == 1) //sud
	{
		for(j = 0; j < 4; j++)
		{
			for(i = 3; i >= 0; i--)
			{
				g = i;
				//cat timp este nevalid si exista elemente pe tabla, muta-le succesiv
				while(board[i][j] == 0 && g > 0)
					{
						for(k = i; k > 0; k--)
							board[k][j] = board[k - 1][j];
						board[0][j] = 0;  //si introdu noi celule libere la extremitate
						g--;
					}
				if(board[i][j] != 0)
				{
					if(i < 3) //exista campuri valide vecine in directia selectata?
						if(board[i + 1][j] == board[i][j]) //daca da, se pot combina?
						{
							*scor = *scor + 2 * board[i][j];
							board[i + 1][j] = 2 * board[i][j];
							if(board[i + 1][j] == 2048) *winner = 1; //verificare joc castigat
							board[i][j] = 0;
							g = i;
							//cat timp este nevalid si exista elemente pe tabla, muta-le succesiv
							while(board[i][j] == 0 && g > 0)
								{
									for(k = i; k > 0; k--)
										board[k][j] = board[k - 1][j];
									board[0][j] = 0; //si introdu noi celule libere la extremitate
									g--;
								}	
						}
				}
			}
		}
	}

	if(dir == 2) //est
	{
		for(i = 0; i < 4; i++)
		{
			for(j = 3; j >= 0; j--)
			{
				g = j;
				//cat timp este nevalid si exista elemente pe tabla, muta-le succesiv
				while(board[i][j] == 0 && g > 0)
					{
						for(k = j; k > 0; k--)
							board[i][k] = board[i][k - 1];
						board[i][0] = 0; //si introdu noi celule libere la extremitate
						g--;
					}
				if(board[i][j] != 0)
				{
					if(j < 3) //exista campuri valide vecine in directia selectata?
						if(board[i][j + 1] == board[i][j]) //daca da, se pot combina?
						{
							*scor = *scor + 2 * board[i][j];
							board[i][j + 1] = 2 * board[i][j];
							if(board[i][j + 1] == 2048) *winner = 1; //verificare joc castigat
							board[i][j] = 0;
							g = j;
							//cat timp este nevalid si exista elemente pe tabla, muta-le succesiv
							while(board[i][j] == 0 && g > 0)
								{
									for(k = j; k > 0; k--)
										board[i][k] = board[i][k - 1];
									board[i][0] = 0; //si introdu noi celule libere la extremitate
									g--;
								}	
						}
				}
			}
		}
	}

	if(dir == 3) //vest
	{
		for(i = 0; i < 4; i++)
		{
			for(j = 0; j < 4; j++)
			{
				g = j;
				//cat timp este nevalid si exista elemente pe tabla, muta-le succesiv
				while(board[i][j] == 0 && g < 3)
					{
						for(k = j; k < 3; k++)
							board[i][k] = board[i][k + 1];
						board[i][3] = 0; //si introdu noi celule libere la extremitate
						g++;
					}
				if(board[i][j] != 0)
				{
					if(j > 0) //exista campuri valide vecine in directia selectata?
						if(board[i][j - 1] == board[i][j]) //daca da, se pot combina?
						{
							*scor = *scor + 2 * board[i][j];
							board[i][j - 1] = 2 * board[i][j];
							if(board[i][j - 1] == 2048) *winner = 1; //verificare joc castigat
							board[i][j] = 0;
							g = j;
							//cat timp este nevalid si exista elemente pe tabla, muta-le succesiv
							while(board[i][j] == 0 && g < 3)
								{
									for(k = j; k < 3; k++)
										board[i][k] = board[i][k + 1];
									board[i][3] = 0; //si introdu noi celule libere la extremitate
									g++;
								}	
						}
				}
			}
		}
	}

}

//functie pentru generarea unei noi valori 2 sau 4
void new_value(int board[4][4])
{
	int i, j;
	srand(time(NULL)); //pe baza timpului curent

	while(1)
	{
		i = rand() % 4;
		j = rand() % 4;

		if(board[i][j] == 0)
			{
				board[i][j] = (rand() % 2 + 1) * 2; 
				break;
			}
	}
}

//functie pentru verificarea validitatii mutarii
//daca mode == 1, aplica miscarea pe matricea jocului
//daca mode == 0, doar verifica validitatea miscarii
int valid_move(int board[4][4], int dir, int *scor, int *winner, int mode)
{
	int i, j, test[4][4], valid = 0;
	for(i = 0; i < 4; i++) //copierea matricei intr-o matrice test
		for(j = 0; j < 4; j++)
			test[i][j] = board[i][j];
	game(test, dir, scor, winner); //aplicarea mutarii pe matricea de test
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			if(test[i][j] != board[i][j]) valid = 1; 
	if(valid && mode)
		for(i = 0; i < 4; i++)
			for(j = 0; j < 4; j++)
				board[i][j] = test[i][j];
	return valid;
}

//functie de verificare pentru joc terminat (nu mai exista mutari valide)
int gameover(int board[4][4])
{
	int test1 = 0, test2 = 0, i, j, dir0, dir1, dir2, dir3;
	for(i = 0; i < 4; i++) //verificare existenta celule libere
		for(j = 0; j < 4; j++)
			if(board[i][j] == 0) return 0; 
	//verificare directii valide
	dir0 = valid_move(board, 0, &test1, &test2, 0); 
	dir1 = valid_move(board, 1, &test1, &test2, 0);
	dir2 = valid_move(board, 2, &test1, &test2, 0);
	dir3 = valid_move(board, 3, &test1, &test2, 0);
	if(dir0 || dir1 || dir2 || dir3) return 0;
	else return 1; //altfel, gameover
}

//printarea mesajului de gameover si revenirea la meniu
void gameover_print(WINDOW *win, int *running_game, PANEL *game_panel, PANEL *menu_panel, PANEL *bkgr_panel)
{
	int nline, ncols, startx;

	getmaxyx(win, nline, ncols);
	startx = (ncols  - 4 * S_WIDTH) / 2;

	wattron(win, A_BOLD | COLOR_PAIR(1));
    mvwprintw(win, nline - 3, startx + 12, "IT'S OVER!");
   	wattron(win, A_ITALIC);
   	mvwprintw(win, nline - 2, startx + 1, "N-a fost sa fie de mai mult! :(");
   	wattroff(win, A_BOLD | COLOR_PAIR(1) | A_ITALIC);
   	*running_game = 0; //finalizare sesiune
   	update_panels();
   	doupdate();
   	getch(); //mai asteapta inca o tasta dupa gameover pentru a reveni la meniu
   	hide_panel(game_panel);
   	show_panel(bkgr_panel);
   	show_panel(menu_panel);
   	update_panels();
   	doupdate();
}

//functia pentru mutarea automata
int automove(WINDOW *win, int board[4][4], int *scor, int *winner)
{
	int i, j, k, test[4][4], scor_test = 0, winner_test = 0, counter, best_dir = -1, max = 0;

	for(k = 0; k < 4; k++) //pentru cele patru directii
	{
		if(valid_move(board, k, &scor_test, &winner_test, 0))
		{
			counter = 0; //numarator de celule goale
			for(i = 0; i < 4; i++)
				for(j = 0; j < 4; j++)
					test[i][j] = board[i][j];
			game(test, k, &scor_test, &winner_test);
			for(i = 0; i < 4; i++)
				for(j = 0; j < 4; j++)
					if(test[i][j] == 0) counter++;
			if(counter > max) 
				{
					best_dir = k; //retinerea celei mai bune directii
					max = counter;
				}
		}
	}
	if(best_dir >= 0) //daca exista inca miscari valide, execut-o pe cea mai eficienta
	{
		game(board, best_dir, scor, winner);
		new_value(board);
		fill_board(win, board, *winner, *scor);
		update_panels();
    	doupdate();
		return 1;
	}
	else return 0; //altfel, nu mai exista miscari valide
}

//functie de back-up si undo
//daca mode == 0, atunci retine scorul, statusul curent al variabilei winner si matricea
//daca mode == 1, atunci efectueaza undo pe cele trei elemente
void backup_undo(int board[4][4], int *scor, int *winner, int undo[4][4], int *scor_backup, int *winner_backup, int mode)
{
	int i, j;
	if(mode == 0) //backup mode
	{
		for(i = 0; i < 4; i++)
			for(j = 0; j < 4; j++)
				undo[i][j] = board[i][j];
		*scor_backup = *scor;
		*winner_backup = *winner;
	}
	else //undo mode
	{
		for(i = 0; i < 4; i++)
			for(j = 0; j < 4; j++)
				board[i][j] = undo[i][j];
		*scor = *scor_backup;
		*winner = *winner_backup;
	}
}
