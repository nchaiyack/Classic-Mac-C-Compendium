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
/**                  See the file "c4.c" for documentation.                **/
/**                                                                        **/
/****************************************************************************/

#ifndef C4_DEFINED
#define C4_DEFINED

#define EMPTY   2

#ifndef Boolean
#define Boolean unsigned char
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef struct {

    char **board;           /* The board configuration of the game state.  */
                            /* board[x][y] specifies the position of the   */
                            /* xth column and the yth row of the board,    */
                            /* where column and row numbering starts at 0. */
                            /* (The 0th row is the bottom row.)            */
                            /* A value of 0 specifies that the position is */
                            /* occupied by a piece owned by player 0, a    */
                            /* value of 1 specifies that the position is   */
                            /* occupied by a piece owned by player 1, and  */
                            /* a value of EMPTY specifies that the         */
                            /* position is unoccupied.                     */

    int *(score_array[2]);  /* An array specifying statistics on both      */
                            /* players.  score_array[0] specifies the      */
                            /* statistics for player 0, while              */
                            /* score_array[1] specifies the statistics for */
                            /* player 1.  These statistics have little     */
                            /* meaning outside the realm of the local      */
                            /* functions of the "c4.c" file.               */

    int num_of_pieces;      /* The number of pieces currently occupying    */
                            /* board spaces.                               */

} Game_state;


/* See the file "c4.c" for documentation on the following functions. */

extern void poll(void (*poll_func)(void), int level);
extern void new_game(int width, int height, int num);
extern Boolean make_move(int player, int column);
extern int automatic_move(int player, int level);
extern Game_state get_game_state(void);
extern int score_of_player(int player);
extern Boolean is_winner(int player);
extern Boolean is_tie(void);
extern void win_coords(int player, int *x1, int *y1, int *x2, int *y2);
extern void end_game(void);

#endif /* C4_DEFINED */
