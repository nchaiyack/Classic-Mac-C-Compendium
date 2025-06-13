/****************************************************************************/
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
/**  This file provides the functions necessary to implement a front-end-  **/
/**  independent, device-independent Connect-4 game.  Multiple board sizes **/
/**  are supported.  It is also possible to specify the number of pieces   **/
/**  necessary to connect in a row in order to win.  Therefore one can     **/
/**  play Connect-3, Connect-5, etc.  An efficient tree-searching          **/
/**  algorithm (making use of alpha-beta cutoff decisions) has been        **/
/**  implemented to insure that the computer plays as quickly as possible. **/
/**                                                                        **/
/**  The declaration of the public functions necessary to use this file    **/
/**  are contained in "c4.h".                                              **/
/**                                                                        **/
/**  In all of the public functions, the value of player can be any        **/
/**  integer, where an even integer refers to player 0 and an odd integer  **/
/**  refers to player 1.                                                   **/
/**                                                                        **/
/****************************************************************************/
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "c4.h"

/* Some macros for convenience. */

#define NUM_OF_TEMP_STATES  42
#define other(x)            (((x)==1)? 0 : 1)
#define real_player(x)      ((x) & 1)

/* The static global variables required. */

static int size_x, size_y, num_to_connect;
static int win_places;
static Boolean ***map;
static Game_state real_state;
static Boolean game_in_progress = FALSE, seed_chosen = FALSE;
static Boolean move_in_progress = FALSE;
static void (*poll_function)(void) = NULL;
static int poll_level;
static Game_state temp_states[NUM_OF_TEMP_STATES];
static Boolean temp_array[NUM_OF_TEMP_STATES];
static int temp_states_allocated = 0;

/* A declaration of the local functions. */

static int num_of_win_places(int x, int y, int n);
static void update_score(Game_state *state, int player, int x, int y);
static Boolean drop_piece(Game_state *state, int player, int column);
static int player_score(Game_state *state, int player);
static Boolean winner(Game_state *state, int player);
static Boolean tie(Game_state *state);
static int goodness_of(Game_state *state, int player);
static Game_state *copy_state(Game_state *state);
static void destroy_state(Game_state *state);
static int worst_goodness(Game_state *state, int player, int level,
                          int depth, int alpha, int beta);
static void *emalloc(unsigned int n);


/****************************************************************************/
/**                                                                        **/
/**  This function specifies that the computer should call the specified   **/
/**  function from time to time, in essence polling it to see if the       **/
/**  front-end interface requires any attention.  The specified function   **/
/**  should accept void and return void.  level is the level of lookahead  **/
/**  at which the function should be called.  This level is measured from  **/
/**  the bottom.  Eg. If the lookahead level is set to 6 and level is set  **/
/**  to 4, with a 7x6 board, this function will be called a maximum of     **/
/**  7^2 = 49 times (once for each (6-4)th = 2nd level node visited.       **/
/**                                                                        **/
/**  Note that if a node is not visited due to alpha-beta cutoff, this     **/
/**  function will not be called at that node.  Therefore only a maximum   **/
/**  number of calls can be predicted (with a minimum of 1).               **/
/**                                                                        **/
/**  If no polling is required, the polling function can be specified as   **/
/**  NULL.  This is the default.  This function can be called an arbitrary **/
/**  number of times throughout any game.                                  **/
/**                                                                        **/
/**  It is illegal for the specified poll function to call the functions   **/
/**  make_move(), automatic_move(), or end_game().                         **/
/**                                                                        **/
/****************************************************************************/

void
poll(void (*poll_func)(void), int level)
{
    poll_function = poll_func;
    poll_level = level;
}


/****************************************************************************/
/**                                                                        **/
/**  This function sets up a new game.  This must be called exactly once   **/
/**  before each game is started.  Before it can be called a second time,  **/
/**  end_game() must be called to destroy the previous game.               **/
/**                                                                        **/
/**  width and height are the desired dimensions of the game board, while  **/
/**  num is the number of pieces required to connect in a row in order to  **/
/**  win the game.                                                         **/
/**                                                                        **/
/****************************************************************************/

void
new_game(int width, int height, int num)
{
    register int i, j, k;
    int count;

    assert(!game_in_progress);
    assert(width >= 1 && height >= 1 && num >= 1);

    size_x = width;
    size_y = height;
    num_to_connect = num;
    win_places = num_of_win_places(size_x, size_y, num_to_connect);

    /* Set up a random seed for making random decisions when there is */
    /* equal goodness between two moves.                              */
    if (!seed_chosen) {
        srand(time((time_t *)0));
        seed_chosen = TRUE;
    }

    /* Set up the board */

    real_state.board = (char **) emalloc(size_x * sizeof(char *));
    for (i=0; i<size_x; i++) {
        real_state.board[i] = (char *) emalloc(size_y * sizeof(char));
        for (j=0; j<size_y; j++)
            real_state.board[i][j] = EMPTY;
    }

    /* Set up the score array */

    real_state.score_array[0] = (int *) emalloc(win_places * sizeof(int));
    real_state.score_array[1] = (int *) emalloc(win_places * sizeof(int));
    for (i=0; i<win_places; i++) {
        real_state.score_array[0][i] = 1;
        real_state.score_array[1][i] = 1;
    }

    /* Set up the map */

    map = (Boolean ***) emalloc(size_x * sizeof(Boolean **));
    for (i=0; i<size_x; i++) {
        map[i] = (Boolean **) emalloc(size_y * sizeof(Boolean *));
        for (j=0; j<size_y; j++) {
            map[i][j] = (Boolean *) emalloc(win_places * sizeof(Boolean));
            for (k=0; k<win_places; k++)
                map[i][j][k] = FALSE;
        }
    }

    count = 0;

    /* Fill in the horizontal win positions */
    for (i=0; i<size_y; i++)
        for (j=0; j<size_x-num_to_connect+1; j++) {
            for (k=0; k<num_to_connect; k++)
                map[j+k][i][count] = TRUE;
            count++;
        }

    /* Fill in the vertical win positions */
    for (i=0; i<size_x; i++)
        for (j=0; j<size_y-num_to_connect+1; j++) {
            for (k=0; k<num_to_connect; k++)
                map[i][j+k][count] = TRUE;
            count++;
        }

    /* Fill in the forward diagonal win positions */
    for (i=0; i<size_y-num_to_connect+1; i++)
        for (j=0; j<size_x-num_to_connect+1; j++) {
            for (k=0; k<num_to_connect; k++)
                map[j+k][i+k][count] = TRUE;
            count++;
        }

    /* Fill in the backward diagonal win positions */
    for (i=0; i<size_y-num_to_connect+1; i++)
        for (j=size_x-1; j>=num_to_connect-1; j--) {
            for (k=0; k<num_to_connect; k++)
                map[j-k][i+k][count] = TRUE;
            count++;
        }

    real_state.num_of_pieces = 0;

    for (i=0; i<NUM_OF_TEMP_STATES; i++)
        temp_array[i] = FALSE;

    game_in_progress = TRUE;
}


/****************************************************************************/
/**                                                                        **/
/**  This function drops a piece of the specified player into the          **/
/**  specified column.  Note that column numbering starts at 0.  A value   **/
/**  of TRUE is returned if the drop was successful, or FALSE otherwise.   **/
/**  A drop is unsuccessful if the specified column number is invalid or   **/
/**  full.                                                                 **/
/**                                                                        **/
/****************************************************************************/

Boolean
make_move(int player, int column)
{
    assert(game_in_progress);
    assert(!move_in_progress);
    if (column >= size_x || column < 0) return FALSE;
    return drop_piece(&real_state, real_player(player), column);
}


/****************************************************************************/
/**                                                                        **/
/**  This function instructs the computer to make a move for the specified **/
/**  player.  level specifies the number of levels deep the computer       **/
/**  should search the game tree in order to make its decision.  This      **/
/**  corresponds to the number of "moves" in the game, where each player's **/
/**  turn is considered a move.  The column number of the column in which  **/
/**  the piece was dropped is returned.  Note that column numbering starts **/
/**  at 0.  If no move is possible (i.e. the game board is full), -1 is    **/
/**  returned.  Note that for a standard 7x6 game of Connect-4, the        **/
/**  computer is brain-dead at levels of three or less, while at levels of **/
/**  4 or more the computer provides a challenge.                          **/
/**                                                                        **/
/****************************************************************************/

int
automatic_move(int player, int level)
{
    int i, best_column = -1, goodness = 0, best_worst = -30000;
    Game_state *temp_state;
    int num_of_equal = 0, real;

    assert(game_in_progress);
    assert(!move_in_progress);
    assert(level >= 1);

    move_in_progress = TRUE;
    real = real_player(player);

    /* Simulate a drop in each of the columns and see what the results are. */

    for (i=0; i<size_x; i++) {
        temp_state = copy_state(&real_state);

        /* If this column is full, ignore it as a possibility. */
        if (!drop_piece(temp_state, real, i)) {
            destroy_state(temp_state);
            continue;
        }

        /* If this drop wins the game, it is a really good move! */
        if (winner(temp_state, real)) {
            best_worst = 25000;
            best_column = i;
        }

        /* Otherwise, look ahead to see how good this move may turn out */
        /* to be (assuming the opponent makes the best moves possible). */
        else
            goodness = worst_goodness(temp_state, real, level, 1,
                                      -30000, -best_worst);

        /* If this move looks better than the ones previously considered, */
        /* remember it.                                                   */
        if (goodness > best_worst) {
            best_worst = goodness;
            best_column = i;
            num_of_equal = 1;
        }

        /* If two moves are equally as good, make a random decision. */
        if (goodness == best_worst) {
            num_of_equal++;
            if (rand()%10000 < ((float)1/(float)num_of_equal) * 10000)
                best_column = i;
        }

        destroy_state(temp_state);
    }

    /* Drop the piece in the column decided upon. */

    if (best_column >= 0)
        drop_piece(&real_state, real, best_column);

    move_in_progress = FALSE;
    return best_column;
}


/****************************************************************************/
/**                                                                        **/
/**  This function returns the state of the current game.  The Game_state  **/
/**  structure returned is defined in "c4.h".                              **/
/**                                                                        **/
/****************************************************************************/

Game_state
get_game_state(void)
{
    assert(game_in_progress);
    return real_state;
}


/****************************************************************************/
/**                                                                        **/
/**  This function returns the "score" of the specified player.  This      **/
/**  score is a function of how many winning positions are still available **/
/**  to the player and how close he/she is to achieving each of these      **/
/**  positions.  The scores of both players can be compared to observe how **/
/**  well they are doing relative to each other.                           **/
/**                                                                        **/
/****************************************************************************/

int
score_of_player(int player)
{
    assert(game_in_progress);
    return player_score(&real_state, real_player(player));
}


/****************************************************************************/
/**                                                                        **/
/**  This function returns TRUE if the specified player has won the game,  **/
/**  and FALSE otherwise.                                                  **/
/**                                                                        **/
/****************************************************************************/

Boolean
is_winner(int player)
{
    assert(game_in_progress);
    return winner(&real_state, player);
}


/****************************************************************************/
/**                                                                        **/
/**  This function returns TRUE if the board is completely full, FALSE     **/
/**  otherwise.                                                            **/
/**                                                                        **/
/****************************************************************************/

Boolean
is_tie()
{
    assert(game_in_progress);
    return tie(&real_state);
}


/****************************************************************************/
/**                                                                        **/
/**  This function returns the coordinates of the winning connections of   **/
/**  the specified player.  It is assumed that the specified player has    **/
/**  indeed won the game.  The coordinates are returned in x1, y1, x2, y2, **/
/**  where (x1, y1) specifies the lower-left piece of the winning          **/
/**  connection, and (x2, y2) specifies the upper-right piece of the       **/
/**  winning connection.  If more than one winning connection exists, only **/
/**  one will be returned.                                                 **/
/**                                                                        **/
/****************************************************************************/

void
win_coords(int player, int *x1, int *y1, int *x2, int *y2)
{
    register int i, j;
    int look_for, win_pos = -1;
    int realplayer;
    Boolean found;

    assert(game_in_progress);
    look_for = 1 << num_to_connect;
    realplayer = real_player(player);
    for (i=0; i<win_places; i++)
        if (real_state.score_array[realplayer][i] == look_for) {
            win_pos = i;
            break;
        }

    if (win_pos == -1) {
        fprintf(stderr, "win_coords: no winner\n");
        exit(1);
    }

    /* Find the lower-left piece of the winning connection. */

    found = FALSE;
    for (j=0; j<size_y && !found; j++)
        for (i=0; i<size_x; i++)
            if (map[i][j][win_pos]) {
                *x1 = i;
                *y1 = j;
                found = TRUE;
                break;
            }

    /* Find the upper-right piece of the winning connection. */

    found = FALSE;
    for (j=size_y-1; j>=0 && !found; j--)
        for (i=size_x-1; i>=0; i--)
            if (map[i][j][win_pos]) {
                *x2 = i;
                *y2 = j;
                found = TRUE;
                break;
            }
}


/****************************************************************************/
/**                                                                        **/
/**  This function ends the current game.  It is assumed that a game is    **/
/**  indeed in progress.  It is illegal to call any other game function    **/
/**  immediately after this one except for new_game() and poll().          **/
/**                                                                        **/
/****************************************************************************/

void
end_game(void)
{
    int i, j;

    assert(game_in_progress);
    assert(!move_in_progress);

    /* Free up the memory used by the game state. */

    for (i=0; i<size_x; i++)
        free(real_state.board[i]);
    free(real_state.board);
    free(real_state.score_array[0]);
    free(real_state.score_array[1]);

    /* Free up the memory used by the map. */

    for (i=0; i<size_x; i++) {
        for (j=0; j<size_y; j++)
            free(map[i][j]);
        free(map[i]);
    }
    free(map);

    /* Free up the memory of all the temporary states used. */

    for (i=0; i<temp_states_allocated; i++) {
        temp_array[i] = FALSE;
        for (j=0; j<size_x; j++)
            free(temp_states[i].board[j]);
        free(temp_states[i].board);
        free(temp_states[i].score_array[0]);
        free(temp_states[i].score_array[1]);
    }
    temp_states_allocated = 0;

    game_in_progress = FALSE;
}


/****************************************************************************/
/****************************************************************************/
/**                                                                        **/
/**  The following functions are local to this file and should not be      **/
/**  called externally.                                                    **/
/**                                                                        **/
/****************************************************************************/
/****************************************************************************/


/****************************************************************************/
/**                                                                        **/
/**  This function returns the number of possible win positions on a board **/
/**  of dimensions x by y with n being the number of pieces required in a  **/
/**  row in order to win.                                                  **/
/**                                                                        **/
/****************************************************************************/

static int
num_of_win_places(int x, int y, int n)
{
    return 4*x*y - 3*x*n - 3*y*n + 3*x + 3*y - 4*n + 2*n*n + 2;
}


/****************************************************************************/
/**                                                                        **/
/**  This function updates the score of the specified player given that    **/
/**  the player has just placed a game piece in column x, row y.           **/
/**                                                                        **/
/**  The specified game state is used, which may be a temporary state.     **/
/**                                                                        **/
/****************************************************************************/

static void
update_score(Game_state *state, int player, int x, int y)
{
    register int i;

    for (i=0; i<win_places; i++)
        if (map[x][y][i]) {
            state->score_array[player][i] <<= 1;
            state->score_array[other(player)][i] = 0;
        }
}


/****************************************************************************/
/**                                                                        **/
/**  This function drops a piece of the specified player into the          **/
/**  specified column.  A value of TRUE is returned if the drop was        **/
/**  successful, and FALSE if it was not (i.e. the specified column is     **/
/**  full).                                                                **/
/**                                                                        **/
/**  The specified game state is used, which may be a temporary state.     **/
/**                                                                        **/
/****************************************************************************/

static Boolean
drop_piece(Game_state *state, int player, int column)
{
    int y = 0;

    while (state->board[column][y] != EMPTY && ++y < size_y)
        ;

    if (y == size_y)
        return FALSE;

    state->board[column][y] = player;
    state->num_of_pieces++;
    update_score(state, player, column, y);

    return TRUE;
}


/****************************************************************************/
/**                                                                        **/
/**  This function returns the "score" of the specified player.  This      **/
/**  score is a function of how many winning positions are still available **/
/**  to the player and how close he/she is to achieving each of these      **/
/**  positions.  The scores of both players can be compared to observe how **/
/**  well they are doing relative to each other.                           **/
/**                                                                        **/
/**  The specified game state is used, which may be a temporary state.     **/
/**                                                                        **/
/****************************************************************************/

static int
player_score(Game_state *state, int player)
{
    register int i;
    int score = 0;

    for (i=0; i<win_places; i++)
        score += state->score_array[player][i];

    return score;
}


/****************************************************************************/
/**                                                                        **/
/**  This function returns TRUE if the specified player has won the game,  **/
/**  and FALSE otherwise.                                                  **/
/**                                                                        **/
/**  The specified game state is used, which may be a temporary state.     **/
/**                                                                        **/
/****************************************************************************/

static Boolean
winner(Game_state *state, int player)
{
    register int i;
    int look_for = 1 << num_to_connect;

    for (i=0; i<win_places; i++)
        if (state->score_array[player][i] == look_for)
            return TRUE;

    return FALSE;
}


/****************************************************************************/
/**                                                                        **/
/**  This function returns TRUE if the board is completely full, FALSE     **/
/**  otherwise.                                                            **/
/**                                                                        **/
/**  The specified game state is used, which may be a temporary state.     **/
/**                                                                        **/
/****************************************************************************/

static Boolean
tie(Game_state *state)
{
    return (state->num_of_pieces == size_x * size_y);
}


/****************************************************************************/
/**                                                                        **/
/**  This function returns a measure of the "goodness" of the specified    **/
/**  state for the specified player.  This goodness value is calculated by **/
/**  subtracting the score of the player's opponent from the player's own  **/
/**  score.  A positive value will result if the specified player is in a  **/
/**  better situation than his/her opponent.                               **/
/**                                                                        **/
/****************************************************************************/

static int
goodness_of(Game_state *state, int player)
{
    return player_score(state, player) - player_score(state, other(player));
}


/****************************************************************************/
/**                                                                        **/
/**  This function will return a copy of the specified state.  For         **/
/**  efficiency, an array of allocated temporary states is kept, so that   **/
/**  memory does not have to be allocated for a new state every time this  **/
/**  function is called.  When the copy is finished with, it must be       **/
/**  destroyed with destroy_state().                                       **/
/**                                                                        **/
/****************************************************************************/

static Game_state *
copy_state(Game_state *state)
{
    register int i, j, k;

    for (i=0; i<temp_states_allocated; i++)
        if(!temp_array[i]) break;

    if (i==temp_states_allocated) {
        if (i==NUM_OF_TEMP_STATES) {
            fprintf(stderr, "copy_state: too many temp states\n");
            exit(1);
        }

        /* Allocate space for the board */

        temp_states[i].board = (char **) emalloc(size_x * sizeof(char *));
        for (j=0; j<size_x; j++)
            temp_states[i].board[j] = (char *) emalloc(size_y * sizeof(char));

        /* Allocate space for the score array */

        temp_states[i].score_array[0] =
                                (int *) emalloc(win_places * sizeof(int));
        temp_states[i].score_array[1] =
                                (int *) emalloc(win_places * sizeof(int));

        temp_states_allocated++;
    }

    temp_array[i] = TRUE;

    /* Copy the board */

    for (j=0; j<size_x; j++)
        for (k=0; k<size_y; k++)
            temp_states[i].board[j][k] = state->board[j][k];

    /* Copy the score array */

    for (j=0; j<win_places; j++) {
        temp_states[i].score_array[0][j] = state->score_array[0][j];
        temp_states[i].score_array[1][j] = state->score_array[1][j];
    }

    return &temp_states[i];
}


/****************************************************************************/
/**                                                                        **/
/**  This function destroys the specified game state (assumed to have been **/
/**  created by copy_state()).  For efficiency, the memory used by the     **/
/**  state is not deallocated, so that copy_state() may use this memory    **/
/**  again for a new state.                                                **/
/**                                                                        **/
/****************************************************************************/

static void
destroy_state(Game_state *state)
{
    register int i;

    for (i=0; i<temp_states_allocated; i++)
        if(state == &temp_states[i])
            break;

    assert(i < temp_states_allocated);

    temp_array[i] = FALSE;
}


/****************************************************************************/
/**                                                                        **/
/**  This function determines how good the specified state may turn out to **/
/**  be for the specified player.  It does this by looking ahead level     **/
/**  moves.  It is assumed that both the specified player and the opponent **/
/**  may make the best move possible.  Since this function is recursive,   **/
/**  depth keeps track of the current depth of recursion.  so_far keeps    **/
/**  track of the best worst goodness (if you dig my meaning) so far, so   **/
/**  that if a goodness worse than that crops up, the game tree can be     **/
/**  pruned to avoid searching unneccessary paths (this technique is       **/
/**  called alpha-beta cutoff).                                            **/
/**                                                                        **/
/**  The specified poll function (if any) is called at the appropriate     **/
/**  level.                                                                **/
/**                                                                        **/
/**  The worst goodness that the specified state can produce in the number **/
/**  of moves (levels) searched is returned.  This is the best the         **/
/**  specified player can hope to achieve with this state (since it is     **/
/**  assumed that the opponent will make the best moves possible).         **/
/**                                                                        **/
/****************************************************************************/

static int
worst_goodness(Game_state *state, int player, int level, int depth,
               int alpha, int beta)
{
    int i, goodness, best, maxab = alpha;
    Game_state *temp_state;

    if (poll_function && level-depth==poll_level)
        (*poll_function)();

    if (level == depth)
        return goodness_of(state, player);
    else {
        /* Assume it is the other player's turn. */
        best = -30000;
        for(i=0; i<size_x; i++) {
            temp_state = copy_state(state);
            if (!drop_piece(temp_state, other(player), i)) {
                destroy_state(temp_state);
                continue;
            }
            if (winner(temp_state, other(player)))
                goodness = 25000 - depth;
            else
                goodness = worst_goodness(temp_state, other(player), level,
                                          depth+1, -beta, -maxab);
            if (goodness > best) {
                best = goodness;
                if (best > maxab)
                    maxab = best;
            }
            destroy_state(temp_state);
            if (best > beta)
                break;
        }

        /* What's good for the other player is bad for this one. */
        return -best;
    }
}


/****************************************************************************/
/**                                                                        **/
/**  A safer version of malloc().                                          **/
/**                                                                        **/
/****************************************************************************/

static void *
emalloc(unsigned int n)
{
    void *ptr;

    ptr = (void *) malloc(n);
    if ( ptr == NULL ) {
        fprintf(stderr,"c4: emalloc() - Can't allocate %d bytes.\n", n);
        exit(1);
    }
    return ptr;
}
