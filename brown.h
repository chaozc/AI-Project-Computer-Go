/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* This is Brown, a simple go program.                           *
*                                                               *
* Copyright 2003 and 2004 by Gunnar Farnebäck.                  *
*                                                               *
* Permission is hereby granted, free of charge, to any person   *
* obtaining a copy of this file gtp.c, to deal in the Software  *
* without restriction, including without limitation the rights  *
* to use, copy, modify, merge, publish, distribute, and/or      *
* sell copies of the Software, and to permit persons to whom    *
* the Software is furnished to do so, provided that the above   *
* copyright notice(s) and this permission notice appear in all  *
* copies of the Software and that both the above copyright      *
* notice(s) and this permission notice appear in supporting     *
* documentation.                                                *
*                                                               *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY     *
* KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE    *
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR       *
* PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS. IN NO      *
* EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS  *
* NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR    *
* CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING    *
* FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF    *
* CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT    *
* OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS       *
* SOFTWARE.                                                     *
*                                                               *
* Except as contained in this notice, the name of a copyright   *
* holder shall not be used in advertising or otherwise to       *
* promote the sale, use or other dealings in this Software      *
* without prior written authorization of the copyright holder.  *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define VERSION_STRING "3.1416"

#define MIN_BOARD 2
#define MAX_BOARD 23

static const int EMPTY = 0;
static const int WHITE = 1;
static const int BLACK = 2;
static const int DEAD = 0;
static const int ALIVE = 1;
static const int SEKI = 2;
static const int WHITE_TERRITORY = 3;
static const int BLACK_TERRITORY = 4;
static const int UNKNOWN = 5;

/* Macros to convert between 1D and 2D coordinates. The 2D coordinate
* (i, j) points to row i and column j, starting with (0,0) in the
* upper left corner.
*/

#include "GoBoard.h"
#include "FastBoard.h"
#define POS(i, j) ((i) * GoBoard::BOARD_SIZE + (j))
#define I(pos) ((pos) / GoBoard::BOARD_SIZE)
#define J(pos) ((pos) % GoBoard::BOARD_SIZE)

/* Macro to find the opposite color. */
#define OTHER_COLOR(color) (WHITE + BLACK - (color))

extern float komi;
extern int board_size;

void init_brown(void);
void clear_board(void);
int board_empty(void);
int get_board(int i, int j);
int get_string(int i, int j, int *stonei, int *stonej);
int legal_move(int i, int j, int color);
void play_move(int i, int j, int color);
void generate_move(int *i, int *j, int color);
void compute_final_status(void);
int get_final_status(int i, int j);
void set_final_status(int i, int j, int status);
int valid_fixed_handicap(int handicap);
void place_fixed_handicap(int handicap);
void place_free_handicap(int handicap);

/*
* Local Variables:
* tab-width: 8
* c-basic-offset: 2
* End:
*/