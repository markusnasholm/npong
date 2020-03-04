#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <time.h>

//#define CLOCK       50000000L
#define CLOCK      16666666L

typedef struct score{
    int one;
    int two;
} score;

typedef struct racket{
    int x,y;
    int w, h;
    int dir;
    bool player;
} racket;

typedef struct ball{
    float x;
    float y;
    float speed[2];
} ball;

struct timespec ts = {0,   CLOCK};

int draw(racket* rkts[2], ball b)
{
    mvprintw(b.y, b.x, "*");

    int x,y;
    for (int i = 0; i < 2; i++) {
        for (x = rkts[i]->x; x < rkts[i]->x + rkts[i]->w; x++) {
            mvprintw(rkts[i]->y, x, "#");
        }
        for (x = rkts[i]->x; x < rkts[i]->x + rkts[i]->w; x++) {
            mvprintw(rkts[i]->y - 1 + rkts[i]->h, x, "#");
        }
        for (y = rkts[i]->y + 1; y < rkts[i]->y + rkts[i]->h - 1; y++) {
            mvprintw(y, rkts[i]->x, "#");
        }
        for (y = rkts[i]->y + 1; y < rkts[i]->y + rkts[i]->h - 1; y++) {
            mvprintw(y, rkts[i]->x + rkts[i]->w - 1, "#");
        }
    }
    return 0;
}

int rAI(racket* r, ball b) {
    int m = r->y + (r->h / 2);
    if (b.speed[0] > 0) {
        if (b.y > m + 1)
            r->dir = 1;
        else if (b.y < m - 1)
            r->dir = -1;
        else
            r->dir = 0;
    }
    else
        r->dir = 0;
    return 0;
}

int rmove(racket* rkts[2], int row)
{
    for (int i = 0; i < 2; i++) {
        if (rkts[i]->y > 0 && rkts[i]->y + rkts[i]->h < row) {
            rkts[i]->y += rkts[i]->dir;
        }
        else {
            if (rkts[i]->y <= 0)
                ++rkts[i]->y;
            if (rkts[i]->y + rkts[i]->h >= row)
                --rkts[i]->y;
            rkts[i]->dir = 0;
        }
    }
    return 0;
};

int bmove(ball* b, racket* rkts[2], int c, int r)
{
    b->x += b->speed[0];
    b->y += b->speed[1];
    if (b->x > c) 
        return 1;
    if (b->x < 0)
        return 2;

    if (b->x >= rkts[1]->x && b->x < rkts[1]->x + rkts[1]->w) {
        if (b->y >= rkts[1]->y && b->y < rkts[1]->y + rkts[1]->h){
            b->speed[0] = -b->speed[0];
            int m = rkts[1]->y + (rkts[1]->h / 2);
            b->speed[1] += (b->y - m) / 10;
        }
    }

    if (b->x <= rkts[0]->x + rkts[0]->w - 1 && b->x > rkts[0]->x) {
        if (b->y >= rkts[0]->y && b->y < rkts[0]->y + rkts[0]->h){
            b->speed[0] = -b->speed[0];
            int m = rkts[0]->y + (rkts[0]->h / 2);
            b->speed[1] += (b->y - m) / 10;
        }
    }
    if (b->y > r || b->y < 0)
        b->speed[1] = -b->speed[1];

    return 0;
};

int gameLoop(int col, int row, score score)
{
    ball b = {col/2, row/2, {1,0}};
    
    racket one = {6,0, 3, row/8, 0, true};
    racket two = {col - 8,0, 3, row/8, 0, false};
    one.y = (row/2) - one.h/2;
    two.y = (row/2) - two.h/2;
    racket* rackets[2] = {&one, &two};

    int i = 0;
    int result = 0;
    int debug = 0;
    int key = 0;

    while (1){
        erase();
        mvprintw(0, (col/2 - 5), "%d | %d", score.one, score.two);

        if ('q' == (key = getch()))
            break;
        else if ('d' == key){
            if (!debug)
                debug = 1;
            else
                debug = 0;
        }


        for (int j = 0; j < 2; j++) {
            if (rackets[j]->player) {
                if ('j' == key){
                    if (rackets[j]->dir >= 0)
                        rackets[j]->dir = 1;
                    else
                        rackets[j]->dir = 0;
                }
                else if ('k' == key){
                    if (rackets[j]->dir <= 0)
                        rackets[j]->dir = -1;
                    else
                        rackets[j]->dir = 0;
                }
            }
            else
                rAI(rackets[j], b);
        }

        result = bmove(&b, rackets, col, row);
        if (result)
            return result;

        rmove(rackets, row);


        // Drawing:
        draw(rackets, b);
        if (debug) {
            mvprintw(0, 1, "%d %d %d", i, col, row);
            mvprintw(1, 1, "ONE %d %d %d", one.x, one.y, one.h);
            mvprintw(2, 1, "TWO %d %d %d", two.x, two.y, one.h);
            mvprintw(3, 1, "BAL %f", b.speed[1]);
            mvprintw(4, 1, "BAL %f", b.y);
            int m = two.y + (two.h / 2);
            mvprintw(5, 1, "2_m %d", m);
        }
        refresh();
        nanosleep(&ts, NULL);
        ++i;
    }
    return 0;
}

int main()
{
    //init ncurses
    int row,col;
    initscr();
    getmaxyx(stdscr,row,col);
    row--;
    col--;
    raw();
    timeout(0);
    noecho();
    curs_set(0);

    score score = {0,0};
    int result = 0;
    bool game_over = false;

    while ( !game_over ){
        result = gameLoop(col, row, score);
        if (result == 0)
            game_over = true;
        if (result == 1)
            ++score.one;
        if (result == 2)
            ++score.two;
        if (score.one > 3 || score.two > 3)
            game_over = true;
    }
    endwin();
    return 0;
}
