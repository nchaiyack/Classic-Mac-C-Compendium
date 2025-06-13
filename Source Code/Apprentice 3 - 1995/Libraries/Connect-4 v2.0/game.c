/****************************************************************************/
/**                                                                        **/
/**                          Connect-4 Algorithm                           **/
/**                                                                        **/
/**                              Version 2.0                               **/
/**                                                                        **/
/**                            By Keith Pomakis                            **/
/**                     (kppomaki@jeeves.uwaterloo.ca)                     **/
/**                                                                        **/
/**                               Fall, 1993                               **/
/**                                                                        **/
/****************************************************************************/
/**                                                                        **/
/**                          Sample Implementation!                        **/
/**                                                                        **/
/**  This code is poorly written and contains no internal documentation.   **/
/**  Its sole purpose is to quickly demonstrate an actual implementation   **/
/**  of the functions contained in the file "c4.c".  It's a fully working  **/
/**  game which should work on any type of system, so give it a shot!      **/
/**                                                                        **/
/**  The computer is pretty brain-dead at level 3 or less, but at level 4  **/
/**  and up it provides quite a challenge!                                 **/
/**                                                                        **/
/****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include "c4.h"

#define BUFFER_SIZE 80

enum {HUMAN, COMPUTER};

static int get_num(char *prompt, int lower, int upper);
static void print_board(int width, int height);
static void print_dot(void);

static char piece[2] = { 'X', 'O' };

int
main()
{
    int player[2], level[2], turn = 0, num_of_players, move;
    int width, height, num_to_connect;
    int x1, y1, x2, y2;
    char buffer[BUFFER_SIZE];

    printf("\n****  Welcome to the game of Connect!  ****\n\n");
    printf("By Keith Pomakis (kppomaki@jeeves.uwaterloo.ca)\n");
    printf("June 16, 1994\n\n");

    width = get_num("Width of board (7 is standard)? ", 1, 40);
    height = get_num("Height of board (6 is standard)? ", 1, 40);
    num_to_connect = get_num("Number to connect (4 is standard)? ", 1, 40);

    num_of_players = get_num("Number of human players (0, 1 or 2)? ", 0, 2);
    switch (num_of_players) {

        case 0:
            player[0] = player[1] = COMPUTER;
            level[0] = get_num("Skill level of player X (1 - 10)? ", 1, 10);
            level[1] = get_num("Skill level of player O (1 - 10)? ", 1, 10);
            turn = 0;
            break;

        case 1:
            player[0] = HUMAN;
            player[1] = COMPUTER;
            level[1] = get_num("Skill level of computer (1 - 10)? ", 1, 10);
            buffer[0] = NULL;
            do {
                printf("Would you like to go first? ");
                if (!fgets(buffer, BUFFER_SIZE, stdin)) {
                    printf("\nGoodbye!\n");
                    exit(0);
                }
                buffer[0] = tolower(buffer[0]);
            } while (buffer[0] != 'y' && buffer[0] != 'n');

            if (buffer[0] == 'y')
                turn = 0;
            else
                turn = 1;
            break;

        case 2:
            player[0] = player[1] = HUMAN;
            turn = 0;
            break;
    }

    new_game(width, height, num_to_connect);

    do {
        print_board(width, height);
        if (player[turn] == HUMAN) {
            do {
                if (num_of_players == 2)
                    sprintf(buffer, "Player %c, drop in which column? ",
                            piece[turn]);
                else
                    sprintf(buffer, "Drop in which column? ");
                move = get_num(buffer, 1, width) - 1;
            }
            while (!make_move(turn, move));
        }
        else {
            if (num_of_players == 1)
                printf("Thinking");
            else
                printf("Player %c is thinking", piece[turn]);
            fflush(stdout);
            poll(print_dot, level[turn] - 1);
            move = automatic_move(turn, level[turn]);
            if (num_of_players == 1)
                printf("\n\nI dropped my piece into column %d.\n", move+1);
            else
                printf("\n\nPlayer %c dropped its piece into column %d.\n",
                       piece[turn], move+1);
        }

        turn = !turn;

    } while (!is_winner(0) && !is_winner(1) && !is_tie());

    print_board(width, height);

    if (is_winner(0)) {
        win_coords(0, &x1, &y1, &x2, &y2);
        if (num_of_players == 1)
            printf("You won!");
        else
            printf("Player %c won!", piece[0]);
        printf("  (%d,%d) to (%d,%d)\n\n", x1+1, y1+1, x2+1, y2+1);
    }
    else if (is_winner(1)) {
        win_coords(1, &x1, &y1, &x2, &y2);
        if (num_of_players == 1)
            printf("I won!");
        else
            printf("Player %c won!", piece[1]);
        printf("  (%d,%d) to (%d,%d)\n\n", x1+1, y1+1, x2+1, y2+1);
    }
    else {
        printf("There was a tie!\n\n");
    }

    return 0;
}


/****************************************************************************/

static int
get_num(char *prompt, int lower, int upper)
{
    int number;
    static char numbuf[40];

    do {
        printf("%s", prompt);
        if (!fgets(numbuf, 40, stdin)) {
            printf("\nGoodbye!\n");
            exit(0);
        }
        sscanf(numbuf, "%d", &number);
    } while (!isdigit(numbuf[0]) || number < lower || number > upper);

    return number;
}

/****************************************************************************/

static void
print_board(int width, int height)
{
    Game_state state;
    int x, y;
    char spacing[2], dashing[2];

    state = get_game_state();

    spacing[1] = dashing[1] = '\0';
    if (width > 19) {
        spacing[0] = '\0';
        dashing[0] = '\0';
    }
    else {
        spacing[0] = ' ';
        dashing[0] = '-';
    }

    printf("\n");
    for (y=height-1; y>=0; y--) {

        printf("|");
        for (x=0; x<width; x++) {
            if (state.board[x][y] == EMPTY)
                printf("%s %s|", spacing, spacing);
            else
                printf("%s%c%s|", spacing, piece[(int)state.board[x][y]],
                                  spacing);
        }
        printf("\n");

        printf("+");
        for (x=0; x<width; x++)
            printf("%s-%s+", dashing, dashing);
        printf("\n");
    }

    printf(" ");
    for (x=0; x<width; x++)
        printf("%s%d%s ", spacing, (x>8)?(x+1)/10:x+1, spacing);
    printf("\n ");
    for (x=0; x<width; x++)
    if (width > 9)
        for (x=0; x<width; x++)
            printf("%s%c%s ", spacing, (x>8)?'0'+(x+1)-((x+1)/10)*10:' ',
                              spacing);
    printf("\n\n");
}

/****************************************************************************/

static void
print_dot(void)
{
    printf(".");
    fflush(stdout);
}
