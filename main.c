/*
 * My2048
 * (c) 2018 by Felice Murolo, all rights reserved
 * email: linuxboy@giove.tk
 *
 * released under LGPL license
 * Please, add a mention to my work if you use this sources
 * This is a c + ncurses version of famous 2048 game
 *
 * COMPILATION:
 * mkdir build
 * cd build
 * cmake .
 * make
 *
 * run the game in a xterm console using: ./my2048
 *
 */

#include <curses.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

static void uscita(int sig);
void init_grid(void);
void draw_grid(void);
void init_game(void);
int logOf2(int);
int powerOf2(int);
void move_left(void);
void move_right(void);
void move_up(void);
void move_down(void);
void upd_status(char *);
void upd_score(void);
void spawnTile(void);
#define GAME_CAN_CONTINUE 0
#define GAME_HAS_HORI_COUPLE 1
#define GAME_HAS_VERT_COUPLE 2
int checkGame(void);
void gameWin(void);
void gameLose(void);

static char *prgname="my2048";
static char *version="1.0a";
static char *copyright="(c) 2017 by Felice Murolo";
static char *email="linuxboy@giove.tk";

static WINDOW *cell[4][4];
static WINDOW *title, *status, *help, *game;
static int grid[4][4];
int moves=0,score=0;

void main(int argc, char* argv[]) {
    bool run = false;
    int ch=0, r=0, c=0;

    signal(SIGINT, uscita);      /* termina al segnale di interruzione */

    setlocale(LC_ALL,"");
    initscr();      /* inizializza la libreria curses */
    keypad(stdscr, TRUE);  /* abilita la mappatura della tastiera */
    nonl();         /* non convertire NL->CR/NL in output */
    cbreak();       /* prende i caratteri in input uno alla volta, senza attendere il \n */
    noecho();       /* nessuna echo dell'input */
    clear();        /* clear the screen */
    curs_set(0);    /* hide cursor */

    if (has_colors()) {
        start_color();
        init_pair(0, COLOR_BLACK, COLOR_BLACK);
        init_pair(1,COLOR_BLACK, COLOR_GREEN);
        init_pair(2,COLOR_BLACK, COLOR_RED);
        init_pair(3,COLOR_BLACK, COLOR_CYAN);
        init_pair(4,COLOR_BLACK, COLOR_WHITE);
        init_pair(5,COLOR_BLACK, COLOR_MAGENTA);
        init_pair(6,COLOR_WHITE, COLOR_BLUE);
        init_pair(7,COLOR_BLACK, COLOR_YELLOW);
        init_pair(8, COLOR_GREEN, COLOR_BLACK);
        init_pair(9, COLOR_RED, COLOR_BLACK);
        init_pair(10, COLOR_CYAN, COLOR_BLACK);
        init_pair(11, COLOR_WHITE, COLOR_BLACK);
        init_pair(12, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(13, COLOR_BLUE, COLOR_BLACK);
        init_pair(14, COLOR_YELLOW, COLOR_BLACK);
        init_pair(15, COLOR_BLUE,COLOR_BLUE);
    }
    else {
        uscita(1);
    }

    title = newwin(1,40,0,1);
    wattron(title,A_BOLD|COLOR_PAIR(10));
    wprintw(title,"%s v%s - %s",prgname,version,copyright);
    refresh();
    wrefresh(title);

    status = newwin(1,40,15,1);
    wattron(status,A_BOLD|COLOR_PAIR(10));
    wprintw(status,"Prova di status");
    refresh();
    wrefresh(status);

    help = newwin(1,40,16,1);
    wattron(help,A_BOLD|COLOR_PAIR(11));
    wprintw(help,"Q or q to Quit, arrow keys to move");
    refresh();
    wrefresh(help);

    game = newwin(1,40,1,1);
    wattron(game,A_BOLD|COLOR_PAIR(14));
    wprintw(game,"Points: 0 - Moves: 0");
    refresh();
    wrefresh(game);

    init_game();

    run = true;
    while (run) {
        refresh();
        ch = getch();
        switch(ch) {
            case -1:
            case 10:
            case 13:
                break;
            case 'q':
            case 'Q':
                run = false;
                break;

            case KEY_LEFT:
                upd_status("You have typed LEFT");
                move_left();
                break;

            case KEY_RIGHT:
                upd_status("You have typed RIGHT");
                move_right();
                break;

            case KEY_UP:
                upd_status("You have typed UP");
                move_up();
                break;

            case KEY_DOWN:
                upd_status("You have typed DOWN");
                move_down();
                break;

            default:
                break;
        }
    }

    uscita(0);

}

int checkGame(){
    int r=0, c=0, z=0, k=0, j=0;
    bool zero_found=false;
    bool hori_couple=false;
    bool vert_couple=false;
    bool game_win = false;
    bool game_lose = false;

    // controlla se c'è una cella a 2048 (vincita)
    for(r=0; r<4; r++){
        for(c=0; c<4; c++){
            if (grid[r][c] == 2048) game_win = true;
        }
    }
    if (game_win) gameWin();

    // controlla che ci siano celle libere
    for(r=0; r<4; r++){
        for(c=0; c<4; c++){
            if (grid[r][c] == 0) zero_found = true;
        }
    }
    if (zero_found) return(GAME_CAN_CONTINUE);

    // controlla che ci siano valori uguali vicini
    for(r=0; r<4; r++){
        for(c=0; c<4; c++){
            z=grid[r][c];
            k=0;
            if (c<3) k=grid[r][c+1];
            j=0;
            if (r<3) j=grid[r+1][c];
            if (z==k) hori_couple = true;
            if (z==j) vert_couple = true;
        }

    }
    if (hori_couple) {
        upd_status("There is an horizontal couple to join");
        return(GAME_HAS_HORI_COUPLE);
    }
    if (vert_couple) {
        upd_status("There is a vertical couple to join");
        return(GAME_HAS_HORI_COUPLE);
    }
    gameLose();
}

void move_left(void) {
    int r=0, c=0, p=0, k=0, z=0;
    int g1[4][4],g2[4][4];

    // elimina le caselle con valore = 0, affiancando a sinistra
    for(r=0; r<4; r++){
        p=0;
        for(c=0; c<4; c++){
            g1[r][c] = 0;
            z = grid[r][c];
            if (z!=0) {
                g1[r][p] = z;
                p++;
            }
        }
    }

    // verifica i valori affiancati
    for(r=0; r<4; r++){
        p=0;
        for(c=0; c<4; c++){
            g2[r][c] = 0;
            z = g1[r][c];
            k=0;
            if (c <3) k = g1[r][c+1];
            if (k==z && k!=0) {
                g2[r][p] = 2*z;
                g1[r][c+1] = 0;
                score += 2*z;
                p++;
            }
            else if (k!=z){
                g2[r][p] = g1[r][c];
                p++;
            }
        }
    }

    // elimina le caselle con valore = 0, affiancando a sinistra
    for(r=0; r<4; r++){
        p=0;
        for(c=0; c<4; c++){
            g1[r][c] = 0;
            z = g2[r][c];
            if (z!=0) {
                g1[r][p] = z;
                p++;
            }
        }
    }

    // copia i nuovi valori nella griglia
    for(r=0; r<4; r++){
        for(c=0; c<4; c++){
            grid[r][c]=g1[r][c];
        }
    }

    k = checkGame();
    if (k==GAME_CAN_CONTINUE) spawnTile();
    draw_grid();
    moves++;
    upd_score();
}

void move_right() {
    int r=0, c=0, p=0, k=0, z=0;
    int g1[4][4],g2[4][4];

    // elimina le caselle con valore = 0, affiancando a destra
    for(r=0; r<4; r++){
        p=3;
        for(c=3; c>=0; c--){
            g1[r][c] = 0;
            z = grid[r][c];
            if (z!=0) {
                g1[r][p] = z;
                p--;
            }
        }
    }

    // verifica i valori affiancati
    for(r=0; r<4; r++){
        p=3;
        for(c=3; c>=0; c--){
            g2[r][c] = 0;
            z = g1[r][c];
            k=0;
            if (c >0) k = g1[r][c-1];
            if (k==z && k!=0) {
                g2[r][p] = 2*z;
                g1[r][c-1] = 0;
                score += 2*z;
                p--;
            }
            else if (k!=z){
                g2[r][p] = g1[r][c];
                p--;
            }
        }
    }

    // elimina le caselle con valore = 0, affiancando a destra
    for(r=0; r<4; r++){
        p=3;
        for(c=3; c>=0; c--){
            g1[r][c] = 0;
            z = g2[r][c];
            if (z!=0) {
                g1[r][p] = z;
                p--;
            }
        }
    }

    // copia i nuovi valori nella griglia
    for(r=0; r<4; r++){
        for(c=0; c<4; c++){
            grid[r][c]=g1[r][c];
        }
    }

    k = checkGame();
    if (k==GAME_CAN_CONTINUE) spawnTile();
    draw_grid();
    moves++;
    upd_score();

}

void move_up() {
    int r=0, c=0, p=0, k=0, z=0;
    int g1[4][4],g2[4][4];

    // elimina le caselle con valore = 0, affiancando sopra
    for(c=0; c<4; c++){
        p=0;
        for(r=0; r<4; r++){
            g1[r][c] = 0;
            z = grid[r][c];
            if (z!=0) {
                g1[p][c] = z;
                p++;
            }
        }
    }

    // verifica i valori affiancati
    for(c=0; c<4; c++){
        p=0;
        for(r=0; r<4; r++){
            g2[r][c] = 0;
            z = g1[r][c];
            k=0;
            if (r <3) k = g1[r+1][c];
            if (k==z && k!=0) {
                g2[p][c] = 2*z;
                g1[r+1][c] = 0;
                score += 2*z;
                p++;
            }
            else if (k!=z){
                g2[p][c] = g1[r][c];
                p++;
            }
        }
    }

    // elimina le caselle con valore = 0, affiancando sopra
    for(c=0; c<4; c++){
        p=0;
        for(r=0; r<4; r++){
            g1[r][c] = 0;
            z = g2[r][c];
            if (z!=0) {
                g1[p][c] = z;
                p++;
            }
        }
    }

    // copia i nuovi valori nella griglia
    for(r=0; r<4; r++){
        for(c=0; c<4; c++){
            grid[r][c]=g1[r][c];
        }
    }

    k = checkGame();
    if (k==GAME_CAN_CONTINUE) spawnTile();
    draw_grid();
    moves++;
    upd_score();

}

void move_down() {
    int r=0, c=0, p=0, k=0, z=0;
    int g1[4][4],g2[4][4];

    // elimina le caselle con valore = 0, affiancando sotto
    for(c=0; c<4; c++){
        p=3;
        for(r=3; r>=0; r--){
            g1[r][c] = 0;
            z = grid[r][c];
            if (z!=0) {
                g1[p][c] = z;
                p--;
            }
        }
    }

    // verifica i valori affiancati
    for(c=0; c<4; c++){
        p=3;
        for(r=3; r>=0; r--){
            g2[r][c] = 0;
            z = g1[r][c];
            k=0;
            if (r>0) k = g1[r-1][c];
            if (k==z && k!=0) {
                g2[p][c] = 2*z;
                g1[r-1][c] = 0;
                score += 2*z;
                p--;
            }
            else if (k!=z){
                g2[p][c] = g1[r][c];
                p--;
            }
        }
    }

    // elimina le caselle con valore = 0, affiancando sotto
    for(c=0; c<4; c++){
        p=3;
        for(r=3; r>=0; r--){
            g1[r][c] = 0;
            z = g2[r][c];
            if (z!=0) {
                g1[p][c] = z;
                p--;
            }
        }
    }

    // copia i nuovi valori nella griglia
    for(r=0; r<4; r++){
        for(c=0; c<4; c++){
            grid[r][c]=g1[r][c];
        }
    }

    k = checkGame();
    if (k==GAME_CAN_CONTINUE) spawnTile();
    draw_grid();
    moves++;
    upd_score();
}


void init_game(void) {
    int r=0, c=0, z=0;

    init_grid();
    for (r=0; r<4; r++) {
        for(c=0; c<4; c++) {
            grid[r][c]=0;
        }
    }
    spawnTile();
    draw_grid();
}

/**
 * Generate random tile.
 */
void spawnTile() {

    /* Randomize starting position. */
    int x = rand() % 4, y = rand() % 4, z;

    /* Check for available tile. */
    while (grid[x][y] != 0) {
        x = rand() % 4;
        y = rand() % 4;
    }

    /* Randomize tile's value with ratio 1:3 */
    z = (rand() & 3) ? 2 : 4;
    z = 2;

    grid[x][y] = z;
}

void init_grid(void) {
    int r=0, c=0;
    for (r=0; r<4; r++) {
        for (c=0; c<4; c++) {
            cell[r][c] = newwin(3,7,(r*3)+3,(c*7)+1);
            wbkgd(cell[r][c],COLOR_PAIR(15));
            wrefresh(cell[r][c]);
            refresh();
        }
    }
}

void draw_grid(void) {
    int r=0, c=0;
    char s[10];

    for (r=0; r<4; r++) {
        for (c=0; c<4; c++) {
            wbkgd(cell[r][c],COLOR_PAIR(15));
            werase(cell[r][c]);
            if (grid[r][c] != 0) {
                wbkgd(cell[r][c],COLOR_PAIR(logOf2(grid[r][c])));
                sprintf(s,"%d",grid[r][c]);
                mvwprintw(cell[r][c],1,((7-strlen(s))/2),"%d",grid[r][c]);
            }
            wrefresh(cell[r][c]);
            refresh();
        }
    }
}

void upd_status(char *s) {
    werase(status);
    wprintw(status,s);
    wrefresh(status);
    refresh();
}

void upd_score(void) {
    werase(game);
    wprintw(game,"Points: %d - Moves: %d",score,moves);
    wrefresh(game);
    refresh();
}

/**
 * Return the logarithm of a number.
 *
 * @param a The number to compute.
 * @return log2(a).
 */
int logOf2(int a) {
    int foo = 0;
    while ((powerOf2(foo)) != a)
        foo++;
    return foo;
}

/**
 * Return the power of 2 of a given number.
 *
 * @param a The number to compute.
 * @return	2 ^ a.
 */
int powerOf2(int a) {

    if (a == 0) {
        return 1;
    } else {
        return 2 * powerOf2(a - 1);
    }
}

void gameWin(){
    WINDOW *gamew;
    gamew = newwin(3,40,7,1);
    wbkgd(gamew,COLOR_PAIR(1));
    mvwprintw(gamew,1,4,"CONGRATULATION! YOU WIN THE GAME!");
    wrefresh(gamew);
    refresh();
    getch();
    uscita(0);
}

void gameLose(){
    WINDOW *gamew;
    gamew = newwin(3,40,7,1);
    wbkgd(gamew,COLOR_PAIR(2));
    mvwprintw(gamew,1,4,"OH, SORRY, BUT YOU LOSE THE GAME.");
    wrefresh(gamew);
    refresh();
    getch();
    uscita(0);
}


static void uscita(int sig){
    endwin();
    printf("\r\n");
    printf("Return Code: %d\r\n",sig);
    exit(0);
}


