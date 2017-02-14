/* #ifndef UTILS_H_   /\* Include guard *\/ */
/* #define UTILS_H_ */

#define Board(x,y) board[(x)*N + (y)]
#define NewBoard(x,y) newboard[(x)*N + (y)]

#define top(x) top[x]
#define bottom(x) bottom[x]

void playFirstRow (int *board, int *newboard, int *top, int N);
void playLastRow (int *board, int *newboard, int *bottom, int M, int N);

/* set everthing to zero */

void initialize_board (int *board, int M,int N);

/* add to a width index, wrapping around like a cylinder */

int xadd (int i, int a, int N);

/* add to a height index, wrapping around */

int yadd (int i, int a, int N);

/* return the number of on cells adjacent to the i,j cell */

int adjacent_to (int *board, int i, int j, int M,int N);

void topAdjacent_to(int *board, int *newboard, int *top, int pos, int N);

void bottomAdjacent_to(int *board, int *newboard, int *bottom, int pos, int M, int N);

/* play the game through one generation */

void play (int *board, int *newboard, int M,int N);

void MPIplay (int *board, int *newboard, int M, int N);

void playFirstRow (int *board, int *newboard, int *top, int N);

void playLastRow (int *board, int *newboard, int *bottom, int M, int N);

/* print the life board */

void print (int *board, int M, int N);

/* generate random table */

void generate_table (int *board, int M, int N, float threshold,int pid);

/* display the table with delay and clear console */

void display_table(int *board, int M, int N);

/* #endif // FOO_H_ */
