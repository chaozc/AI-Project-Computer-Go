#include "GoBoard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <list>
using namespace std;

double GoBoard::KOMI = 6.5;

GoBoard::GoBoard()
{
	memset(board, 0, sizeof(board));
	memset(libertyCount, 0, sizeof(libertyCount));
	memset(next_stone, 0, sizeof(next_stone));
	ko_i = -1;
	ko_j = -1;
	li = -1;
	lj = -1;
	for (int k = 0; k < BOARD_SIZE*BOARD_SIZE; ++k)
		liberties[k] = -1;
}

GoBoard::~GoBoard()
{
}


/* Offsets for the four directly adjacent neighbors. Used for looping. */
static int deltai[4] = { -1, 1, 0, 0 };
static int deltaj[4] = { 0, 0, -1, 1 };
static const int square3i[9] = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };
static const int square3j[9] = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };

/* Macros to convert between 1D and 2D coordinates. The 2D coordinate
* (i, j) points to row i and column j, starting with (0,0) in the
* upper left corner.
*/
#define POS(i, j) ((i) * BOARD_SIZE + (j))
#define I(pos) ((pos) / BOARD_SIZE)
#define J(pos) ((pos) % BOARD_SIZE)

/* Macro to find the opposite color. */
#define OTHER_COLOR(color) (WHITE + BLACK - (color))



void
GoBoard::clear_board()
{
	memset(board, 0, sizeof(board));
}

int
GoBoard::board_empty()
{
	int i;
	for (i = 0; i < BOARD_SIZE * BOARD_SIZE; i++)
	if (board[i] != EMPTY)
		return 0;

	return 1;
}

int
GoBoard::get_board(int i, int j)
{
	return board[i * BOARD_SIZE + j];
}

/* Get the stones of a string. stonei and stonej must point to arrays
* sufficiently large to hold any string on the board. The number of
* stones in the string is returned.
*/
int
GoBoard::get_string(int i, int j, int *stonei, int *stonej)
{
	int num_stones = 0;
	int pos = POS(i, j);
	do {
		stonei[num_stones] = I(pos);
		stonej[num_stones] = J(pos);
		num_stones++;
		pos = next_stone[pos];
	} while (pos != POS(i, j));

	return num_stones;
}

static int
pass_move(int i, int j)
{
	return i == -1 && j == -1;
}

int
GoBoard::on_board(int i, int j)
{
	return i >= 0 && i < BOARD_SIZE && j >= 0 && j < BOARD_SIZE;
}

int
GoBoard::legal_move(int i, int j, int color)
{
	int other = OTHER_COLOR(color);

	/* Pass is always legal. */
	if (pass_move(i, j))
		return 1;

	/* Already occupied. */
	if (get_board(i, j) != EMPTY)
		return 0;

	/* Illegal ko recapture. It is not illegal to fill the ko so we must
	* check the color of at least one neighbor.
	*/
	if (i == ko_i && j == ko_j
		&& ((on_board(i - 1, j) && get_board(i - 1, j) == other)
		|| (on_board(i + 1, j) && get_board(i + 1, j) == other)))
		return 0;

	return 1;
}

/* Does the string at (i, j) have any more liberty than the one at
* (libi, libj)?float simulate(int _board[], int _next_stone[], int _ko_i, int _ko_j, int start_color, int times);

*/
int
GoBoard::has_additional_liberty(int i, int j, int libi, int libj)
{
	int pos = POS(i, j);
	do {
		int ai = I(pos);
		int aj = J(pos);
		int k;
		for (k = 0; k < 4; k++) {
			int bi = ai + deltai[k];
			int bj = aj + deltaj[k];
			if (on_board(bi, bj) && get_board(bi, bj) == EMPTY
				&& (bi != libi || bj != libj))
				return 1;
		}

		pos = next_stone[pos];
	} while (pos != POS(i, j));

	return 0;
}

/* Does (ai, aj) provide a liberty for a stone at (i, j)? */
int
GoBoard::provides_liberty(int ai, int aj, int i, int j, int color)
{
	/* A vertex off the board does not provide a liberty. */
	if (!on_board(ai, aj))
		return 0;

	/* An empty vertex IS a liberty. */
	if (get_board(ai, aj) == EMPTY)
		return 1;

	/* A friendly string provides a liberty to (i, j) if it currently
	* has more liberties than the one at (i, j).
	*/
	if (get_board(ai, aj) == color)
		return has_additional_liberty(ai, aj, i, j);

	/* An unfriendly string provides a liberty if and only if it is
	* captured, i.e. if it currently only has the liberty at (i, j).
	*/
	return !has_additional_liberty(ai, aj, i, j);
}

/* Is a move at (i, j) suicide for color? */
int
GoBoard::suicide(int i, int j, int color)
{
	int k;
	for (k = 0; k < 4; k++)
	if (provides_liberty(i + deltai[k], j + deltaj[k], i, j, color))
		return 0;

	return 1;
}

/* Remove a string from the board array. There is no need to modify
* the next_stone array since this only matters where there are
* stones present and the entire string is removed.
*/
int
GoBoard::remove_string(int i, int j)
{
	int pos = POS(i, j);
	int removed = 0;
	do {
		board[pos] = EMPTY;
		liberties[pos] = -1;
		removed++;
		pos = next_stone[pos];
	} while (pos != POS(i, j));

	return removed;
}

/* Do two vertices belong to the same string. It is required that both
* pos1 and pos2 point to vertices with stones.
*/
int
GoBoard::same_string(int pos1, int pos2)
{
	int pos = pos1;
	do {
		if (pos == pos2)
			return 1;
		pos = next_stone[pos];
	} while (pos != pos1);

	return 0;
}

/* Play at (i, j) for color. No legality check is done here. We need
* to properly update the board array, the next_stone array, and the
* ko point.
*/
void GoBoard::play_move(int i, int j, int color)
{
	int pos = POS(i, j);
	int captured_stones = 0;
	int k;

	li = i;
	lj = j;
	bool flag = true;

	/* Reset the ko point. */
	ko_i = -1;
	ko_j = -1;

	/* Nothing more happens if the move was a pass. */
	if (pass_move(i, j))
		return;

	/* If the move is a suicide we only need to remove the adjacent
	* friendly stones.
	*/
	if (suicide(i, j, color)) {
		for (k = 0; k < 4; k++) {
			int ai = i + deltai[k];
			int aj = j + deltaj[k];
			if (on_board(ai, aj)
				&& get_board(ai, aj) == color) {
				remove_string(ai, aj);
				flag = false;
			}
		}
		return;
	}
	//printf("c1");
	/* Not suicide. Remove captured opponent strings. */
	for (k = 0; k < 4; k++) {
		int ai = i + deltai[k];
		int aj = j + deltaj[k];
		if (on_board(ai, aj)
			&& get_board(ai, aj) == OTHER_COLOR(color)
			&& !has_additional_liberty(ai, aj, i, j))
		{
			flag = false;
			captured_stones += remove_string(ai, aj);
		}
	}
	//printf("c2");
	/* Put down the new stone. Initially build a single stone string by
	* setting next_stone[pos] pointing to itself.
	*/
	board[pos] = color;
	next_stone[pos] = pos;

	/* If we have friendly neighbor strings we need to link the strings
	* together.
	*/
	for (k = 0; k < 4; k++) {
		int ai = i + deltai[k];
		int aj = j + deltaj[k];
		int pos2 = POS(ai, aj);
		/* Make sure that the stones are not already linked together. This
		* may happen if the same string neighbors the new stone in more
		* than one direction.
		*/
		if (on_board(ai, aj) && board[pos2] == color && !same_string(pos, pos2)) {
			/* The strings are linked together simply by swapping the the
			* next_stone pointers.
			*/
			int tmp = next_stone[pos2];
			next_stone[pos2] = next_stone[pos];
			next_stone[pos] = tmp;
		}
	}
	//printf("c3");
	/* If we have captured exactly one stone and the new string is a
	* single stone it may have been a ko capture.
	*/
	if (captured_stones == 1 && next_stone[pos] == pos) {
		int ai, aj;
		/* Check whether the new string has exactly one liberty. If so it
		* would be an illegal ko capture to play there immediately. We
		* know that there must be a liberty immediately adjacent to the
		* new stone since we captured one stone.
		*/
		for (k = 0; k < 4; k++) {
			ai = i + deltai[k];
			aj = j + deltaj[k];
			if (on_board(ai, aj) && get_board(ai, aj) == EMPTY)
				break;
		}

		if (!has_additional_liberty(i, j, ai, aj)) {
			ko_i = ai;
			ko_j = aj;
		}
	}
	//printf("c4\n");

	if (flag) {
		libertiesUpdate(POS(i, j));
		for (int k = 0; k < 4; ++k) {
			int ai = i + deltai[k];
			int aj = j + deltaj[k];
			if (on_board(ai, aj) && get_board(ai, aj) == OTHER_COLOR(color))
				libertiesUpdate(POS(ai, aj));
		}
	}
	else
		libertiesCal();
}

void GoBoard::play_move(int move, int color) {
	play_move(I(move), J(move), color);
}


int  GoBoard::is_pattern_for_Hane(int i, int j, int color){
	int pattern[4][8] = { { 155711, 144611, 155711, 207986, 258086, 144611, 258086, 207986 },
	{ 147507, 144387, 24627, 204912, 208932, 13347, 208902, 196722 },
	{ 161843, 177203, 221363, 237683, 210983, 209963, 209078, 209018 },
	{ 151603, 144403, 90163, 204913, 208933, 78883, 208918, 200818 } };
	int uncertain_dealer[4][8] = { { 262080, 249660, 262080, 62415, 4095, 249660, 4095, 62415 },
	{ 262092, 249852, 262092, 65487, 53247, 249852, 53247, 65487 },
	{ 249804, 249804, 65484, 65484, 53244, 53244, 53199, 53199 },
	{ 262092, 249852, 262092, 65487, 53247, 249852, 53247, 65487 } };
	int color_to_play[4] = { 0, 0, 0, BLACK };
	int square3 = 0, square3_other_color = 0;
	int base = 1 << 16;
	for (int ii = 0; ii < 9; ++ii){
		int k = get_board(i + square3i[ii], j + square3j[ii]);
		square3 += base * k;
		if (k != EMPTY){
			square3_other_color += base * (3 - k);
		}
		base /= 4;
	}
	for (int ii = 0; ii < 4; ++ii){
		for (int jj = 0; jj < 8; ++jj){
			if (((((square3 & uncertain_dealer[ii][jj]) ^ pattern[ii][jj]) & uncertain_dealer[ii][jj]) == 0) &&
				((color_to_play[ii] == 0) || (color_to_play[ii] == color))){
				return 1;
			}
			if (((((square3_other_color & uncertain_dealer[ii][jj]) ^ pattern[ii][jj]) & uncertain_dealer[ii][jj]) == 0) &&
				((color_to_play[ii] == 0) || (color_to_play[ii] + color == WHITE + BLACK))){
				return 1;
			}
		}
	}
	return 0;
}

int GoBoard::is_pattern_for_cut1(int i, int j, int color){
	int pattern[3][8] = { { 161023, 161023, 224383, 224383, 259303, 259303, 261238, 261238 },
	{ 160883, 160823, 222323, 221303, 210023, 226343, 210038, 225398 },
	{ 160823, 160883, 221303, 222323, 226343, 210023, 225398, 210038 } };
	int uncertain_dealer[3][8] = { { 249600, 249600, 62400, 62400, 3900, 3900, 975, 975 },
	{ 249804, 249804, 65484, 65484, 53244, 53244, 53199, 53199 },
	{ 249804, 249804, 65484, 65484, 53244, 53244, 53199, 53199 } };
	int square3 = 0, square3_other_color = 0;
	int base = 1 << 16;
	for (int ii = 0; ii < 9; ++ii){
		int k = get_board(i + square3i[ii], j + square3j[ii]);
		square3 += base * k;
		if (k != EMPTY){
			square3_other_color += base * (3 - k);
		}
		base /= 4;
	}
	for (int jj = 0; jj < 8; ++jj){
		if (((((square3 & uncertain_dealer[0][jj]) ^ pattern[0][jj]) & uncertain_dealer[0][jj]) == 0) &&
			((((square3 & uncertain_dealer[1][jj]) ^ pattern[1][jj]) & uncertain_dealer[1][jj]) != 0) &&
			((((square3 & uncertain_dealer[2][jj]) ^ pattern[2][jj]) & uncertain_dealer[2][jj]) != 0)){
			return 1;
		}
		if (((((square3_other_color & uncertain_dealer[0][jj]) ^ pattern[0][jj]) & uncertain_dealer[0][jj]) == 0) &&
			((((square3_other_color & uncertain_dealer[1][jj]) ^ pattern[1][jj]) & uncertain_dealer[1][jj]) != 0) &&
			((((square3_other_color & uncertain_dealer[2][jj]) ^ pattern[2][jj]) & uncertain_dealer[2][jj]) != 0)){
			return 1;
		}
	}
	return 0;
}

int  GoBoard::is_pattern_for_cut2(int i, int j, int color){
	int pattern[8][8] = { { 242752, 215092, 242752, 28807, 1147, 215092, 1147, 28807 },
	{ 242754, 215094, 242784, 28839, 9339, 223284, 132219, 159879 },
	{ 242760, 215220, 242760, 30855, 33915, 215220, 33915, 30855 },
	{ 242762, 215222, 242792, 30887, 42107, 223412, 164987, 161927 },
	{ 242784, 223284, 242754, 159879, 132219, 215094, 9339, 28839 },
	{ 242786, 223286, 242786, 159911, 140411, 223286, 140411, 159911 },
	{ 242792, 223412, 242762, 161927, 164987, 215222, 42107, 30887 },
	{ 242794, 223414, 242794, 161959, 173179, 223414, 173179, 161959 } };
	int uncertain_dealer[8] = { 53247, 65487, 53247, 249852, 262092, 65487, 262092, 249852 };
	int square3 = 0, square3_other_color = 0;
	int base = 1 << 16;
	for (int ii = 0; ii < 9; ++ii){
		int k = get_board(i + square3i[ii], j + square3j[ii]);
		square3 += base * k;
		if (k != EMPTY){
			square3_other_color += base * (3 - k);
		}
		base /= 4;
	}
	for (int ii = 0; ii < 8; ++ii){
		for (int jj = 0; jj < 8; ++jj){
			if ((((square3 & uncertain_dealer[jj]) ^ pattern[ii][jj]) & uncertain_dealer[jj]) == 0){
				return 1;
			}
			if ((((square3_other_color & uncertain_dealer[jj]) ^ pattern[ii][jj]) & uncertain_dealer[jj]) == 0){
				return 1;
			}
		}
	}
	return 0;
}

int  GoBoard::is_pattern_for_side(int i, int j, int color){
	int pattern[7][8] = { { 144576, 147516, 207936, 24591, 1251, 245796, 3186, 61446 },
	{ 241728, 198708, 242688, 12423, 123, 215088, 1083, 28803 },
	{ 242752, 215092, 242752, 28807, 1147, 215092, 1147, 28807 },
	{ 236736, 247836, 113856, 61581, 3321, 116796, 3291, 53391 },
	{ 236544, 247824, 110784, 61569, 3129, 67644, 219, 4239 },
	{ 236608, 247828, 111808, 61573, 3193, 84028, 1243, 20623 },
	{ 234624, 215064, 112704, 28809, 1209, 100404, 2139, 36999 } };
	int uncertain_dealer[7][8] = { { 249663, 262083, 62463, 262128, 261948, 16383, 259023, 200703 },
	{ 53247, 65487, 53247, 249852, 262092, 65487, 262092, 249852 },
	{ 53247, 65487, 53247, 249852, 262092, 65487, 262092, 249852 },
	{ 62271, 16371, 246591, 200691, 258831, 212931, 258876, 212976 },
	{ 62463, 16383, 249663, 200703, 259023, 262083, 261948, 262128 },
	{ 62463, 16383, 249663, 200703, 259023, 262083, 261948, 262128 },
	{ 65535, 65535, 249855, 249855, 262095, 262095, 262140, 262140 } };
	int color_to_play[7] = { 0, 0, 0, BLACK, WHITE, WHITE, WHITE };
	int square3 = 0, square3_other_color = 0;
	int base = 1 << 16;
	for (int ii = 0; ii < 9; ++ii){
		//int k = (on_board(i + square3i[ii], j + square3j[ii])) ? (get_board(i + square3i[ii], j + square3j[ii])) : (EMPTY);
		int k;
		if (on_board(i + square3i[ii], j + square3j[ii])) k = get_board(i + square3i[ii], j + square3j[ii]);
		else k = EMPTY;
		square3 += base * k;
		if (k != EMPTY){
			square3_other_color += base * (3 - k);
		}
		base /= 4;
	}
	for (int ii = 0; ii < 7; ++ii){
		for (int jj = 0; jj < 8; ++jj){
			if (((((square3 & uncertain_dealer[ii][jj]) ^ pattern[ii][jj]) & uncertain_dealer[ii][jj]) == 0) &&
				((color_to_play[ii] == 0) || (color_to_play[ii] == color))){
				return 1;
			}
			if (((((square3_other_color & uncertain_dealer[ii][jj]) ^ pattern[ii][jj]) & uncertain_dealer[ii][jj]) == 0) &&
				((color_to_play[ii] == 0) || (color_to_play[ii] + color == WHITE + BLACK))){
				return 1;
			}
		}
	}
	return 0;
}
int GoBoard::is_pattern(int i, int j, int color){
	if (on_board(i, j) && on_board(i - 1, j - 1) && on_board(i - 1, j + 1) && on_board(i + 1, j + 1) && on_board(i + 1, j - 1)){
		if ((is_pattern_for_cut2(i, j, color) == 1) ||
			(is_pattern_for_cut1(i, j, color) == 1) ||
			(is_pattern_for_Hane(i, j, color) == 1)){
			return 1;
		}
	}
	else if (on_board(i, j) && (on_board(i - 1, j - 1) || on_board(i - 1, j + 1) || on_board(i + 1, j + 1) || on_board(i + 1, j - 1))){
		if (is_pattern_for_side(i, j, color) == 1){
			return 1;
		}
	}
	return 0;
}


int GoBoard::pattern_matching(int *i, int *j, int color){
	int moves[BOARD_SIZE * BOARD_SIZE];
	int num_moves = 0;
	int move;
	memset(moves, 0, sizeof(moves));
	for (int ii = 0; ii < 9; ++ii){
		int ai = li + square3i[ii];
		int aj = lj + square3j[ii];
		if (on_board(ai, aj) && on_board(ai - 1, aj - 1) && on_board(ai - 1, aj + 1) && on_board(ai + 1, aj + 1) && on_board(ai + 1, aj - 1)){
			if ((is_pattern_for_cut2(ai, aj, color) == 1) ||
				(is_pattern_for_cut1(ai, aj, color) == 1) ||
				(is_pattern_for_Hane(ai, aj, color) == 1)){
				if (legal_move(ai, aj, color) && !suicide(ai, aj, color)) moves[num_moves++] = POS(ai, aj);
			}
		}
		else if (on_board(ai, aj) && (on_board(ai - 1, aj - 1) || on_board(ai - 1, aj + 1) || on_board(ai + 1, aj + 1) || on_board(ai + 1, aj - 1))){
			if (is_pattern_for_side(ai, aj, color) == 1){
				if (legal_move(ai, aj, color) && !suicide(ai, aj, color)) moves[num_moves++] = POS(ai, aj);
			}
		}
	}
	if (num_moves > 0){
		move = moves[rand() % num_moves];
		*i = I(move);
		*j = J(move);
		return 1;
	}
	else return 0;
}

int GoBoard::pattern_matching(int len, int color, int moves[], int valid[]){
	int num_moves = 0;
	for (int ii = -len; ii <= len; ++ii)
	for (int jj = -len; jj <= len; ++jj){
		int ai = li + ii;
		int aj = lj + jj;
		if (valid[POS(ai, aj)] > 0 && is_pattern(ai, aj, color)){
			moves[num_moves++] = POS(ai, aj);
		}
	}
	return num_moves;
}


/* Generate a move. */
void GoBoard::generate_move(int *i, int *j, int color)
{
	int moves[BOARD_SIZE * BOARD_SIZE];
	int capture_moves[BOARD_SIZE * BOARD_SIZE];
	int pattern_moves[BOARD_SIZE * BOARD_SIZE];
	int valid[BOARD_SIZE * BOARD_SIZE];
	int num_moves = 0;
	int num_capture_moves = 0;
	int move = -1;
	int ai, aj;
	int k;
	int start = clock();

	memset(valid, 0, sizeof(valid));
	memset(moves, 0, sizeof(moves));
	memset(pattern_moves, 0, sizeof(pattern_moves));
	for (ai = 0; ai < BOARD_SIZE; ai++)
	for (aj = 0; aj < BOARD_SIZE; aj++) {
		/* Consider moving at (ai, aj) if it is legal and not suicide. */
		if (legal_move(ai, aj, color)
			&& !suicide(ai, aj, color)) {
			/* Further require the move not to be suicide for the opponent... */
			if (!suicide(ai, aj, OTHER_COLOR(color))) {
				capture_moves[num_capture_moves++] = POS(ai, aj);
				moves[num_moves++] = POS(ai, aj);
				valid[POS(ai, aj)] = 1;
			}
			else {
				capture_moves[num_capture_moves++] = POS(ai, aj);
				/* ...however, if the move captures at least one stone,
				* consider it anyway.
				*/
				for (k = 0; k < 4; k++) {
					int bi = ai + deltai[k];
					int bj = aj + deltaj[k];
					if (on_board(bi, bj) && get_board(bi, bj) == OTHER_COLOR(color)) {
						moves[num_moves++] = POS(ai, aj);
						valid[POS(ai, aj)] = 1;
						//here changes
						*i = ai;
						*j = aj;
						break;
					}
				}
			}
		}
	}



	/* Choose one of the considered moves randomly with uniform
	* distribution. (Strictly speaking the moves with smaller 1D
	* coordinates tend to have a very slightly higher probability to be
	* chosen, but for all practical purposes we get a uniform
	* distribution.)
	*/
	if (num_moves > 0) {
		//Atari defense
		move = AtariDefense(moves, num_moves, color);
		if (move != -1) {
			//printf("AD\n");
			*i = I(move);
			*j = J(move);
			return;
		}
		//Atari capture
		move = AtariCapture(moves, num_moves, color);
		if (move != -1) {
			//printf("AC\n");
			*i = I(move);
			*j = J(move);
			return;
		}

		//pattern
		//if (pattern_matching(i, j, color) == 1) return;
		int num_pattern = pattern_matching(1, color, pattern_moves, valid);
		if (num_pattern > 0){
			move = pattern_moves[rand() % num_pattern];
			*i = I(move);
			*j = J(move);
			return;
		}

		//Capture
		move = Capture(capture_moves, num_capture_moves, color);
		if (move != -1) {
			//printf("AD\n");
			*i = I(move);
			*j = J(move);
			return;
		}

		//Random
		//printf("R\n");



		move = moves[rand() % num_moves];
		*i = I(move);
		*j = J(move);
	}
	else {
		/* But pass if no move was considered. */
		*i = -1;
		*j = -1;
	}
}




/* Set a final status value for an entire string. */
void
GoBoard::set_final_status_string(int pos, int status)
{
	int pos2 = pos;
	do {
		final_status[pos2] = status;
		pos2 = next_stone[pos2];
	} while (pos2 != pos);
}

/* Compute final status. This function is only valid to call in a
* position where generate_move() would return pass for at least one
* color.
*
* Due to the nature of the move generation algorithm, the final
* status of stones can be determined by a very simple algorithm:
*
* 1. Stones with two or more liberties are alive with territory.
* 2. Stones in atari are dead.
*
* Moreover alive stones are unconditionally alive even if the
* opponent is allowed an arbitrary number of consecutive moves.
* Similarly dead stones cannot be brought alive even by an arbitrary
* number of consecutive moves.
*
* Seki is not an option. The move generation algorithm would never
* leave a seki on the board.
*
* Comment: This algorithm doesn't work properly if the game ends with
*          an unfilled ko. If three passes are required for game end,
*          that will not happen.
*/
void
GoBoard::compute_final_status(void)
{
	int i, j;
	int pos;
	int k;

	for (pos = 0; pos < BOARD_SIZE * BOARD_SIZE; pos++)
		final_status[pos] = UNKNOWN;

	for (i = 0; i < BOARD_SIZE; i++)
	for (j = 0; j < BOARD_SIZE; j++)
	if (get_board(i, j) == EMPTY)
	for (k = 0; k < 4; k++) {
		int ai = i + deltai[k];
		int aj = j + deltaj[k];
		if (!on_board(ai, aj))
			continue;
		/* When the game is finished, we know for sure that (ai, aj)
		* contains a stone. The move generation algorithm would
		* never leave two adjacent empty vertices. Check the number
		* of liberties to decide its status, unless it's known
		* already.
		*
		* If we should be called in a non-final position, just make
		* sure we don't call set_final_status_string() on an empty
		* vertex.
		*/
		pos = POS(ai, aj);
		if (final_status[pos] == UNKNOWN) {
			if (get_board(ai, aj) != EMPTY) {
				if (has_additional_liberty(ai, aj, i, j))
					set_final_status_string(pos, ALIVE);
				else
					set_final_status_string(pos, DEAD);
			}
		}
		/* Set the final status of the (i, j) vertex to either black
		* or white territory.
		*/
		if (final_status[POS(i, j)] == UNKNOWN) {
			if ((final_status[pos] == ALIVE) ^ (get_board(ai, aj) == WHITE))
				final_status[POS(i, j)] = BLACK_TERRITORY;
			else
				final_status[POS(i, j)] = WHITE_TERRITORY;

		}
	}
}

int
GoBoard::get_final_status(int i, int j)
{
	return final_status[POS(i, j)];
}

void
GoBoard::set_final_status(int i, int j, int status)
{
	final_status[POS(i, j)] = status;
}

#include <iostream>

double GoBoard::simulateRandomly(int start_color) {
	double result = 0;
	int color = start_color;
	//std::cout << "step R2" << std::endl;
	int cnt = 0;

	while (true) {
		int i, j;
		generate_move(&i, &j, color);
		if (++cnt == 500) return rand() % 2;
		if (pass_move(i, j)) {
			color = OTHER_COLOR(color);
			generate_move(&i, &j, color);
			if (pass_move(i, j)) break;
			color = OTHER_COLOR(color);
		}
		play_move(i, j, color);
		//	show();
		//	system("pause");
		color = OTHER_COLOR(color);
	}

	//std::cout << "step R3" << std::endl;
	compute_final_status();
	result += (start_color == WHITE) ? KOMI : -KOMI;
	for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i) {
		if (final_status[i] == BLACK_TERRITORY || final_status[i] == ALIVE && board[i] == BLACK) {
			if (start_color == BLACK) result += 1;
			else result -= 1;
		}
		if (final_status[i] == WHITE_TERRITORY || final_status[i] == ALIVE && board[i] == WHITE) {
			if (start_color == BLACK) result -= 1;
			else result += 1;
		}
	}
	return result;
}

#include <fstream>
void GoBoard::show() {
	ofstream out("my_log.txt", std::fstream::app);
	for (int i = 0; i < 13; ++i) {
		for (int j = 0; j < 13; ++j) {
			if (board[POS(i, j)] == WHITE) out << "O";
			if (board[POS(i, j)] == BLACK) out << "X";
			if (board[POS(i, j)] == EMPTY) out << ".";
		}
		out << std::endl;
	}
	/*for (int i = 0; i < 13; ++i) {
	for (int j = 0; j < 13; ++j) {
	printf("%d ", liberties[POS(i, j)]);
	}
	std::cout << std::endl;
	}*/
}





int GoBoard::NumOfLiberties(int i, int j) {
	if (get_board(i, j) == EMPTY)
		return -1;
	int result = 0;
	int verified[BOARD_SIZE*BOARD_SIZE];
	int pos = POS(i, j);
	do {
		int ai = I(pos);
		int aj = J(pos);
		int k;
		for (k = 0; k < 4; k++) {
			int bi = ai + deltai[k];
			int bj = aj + deltaj[k];
			if (on_board(bi, bj) && get_board(bi, bj) == EMPTY
				&& (!libertyCount[POS(bi, bj)])) {
				libertyCount[POS(bi, bj)] = true;
				verified[result++] = POS(bi, bj);
			}
		}

		pos = next_stone[pos];
	} while (pos != POS(i, j));
	for (int k = 0; k < result; ++k) {
		libertyCount[verified[k]] = false;
	}

	return result;
}


int GoBoard::AtariCapture(int moves[], int num_moves, int color) {
	int pos = -1;
	int final_moves[BOARD_SIZE*BOARD_SIZE];
	int num_final_move = 0;
	for (int n = 0; n < num_moves; ++n) {
		for (int k = 0; k < 4; ++k) {
			int i = I(moves[n]);
			int j = J(moves[n]);
			int ai = i + deltai[k];
			int aj = j + deltaj[k];
			if (on_board(ai, aj) && get_board(ai, aj) == OTHER_COLOR(color) && liberties[POS(ai, aj)] == 2) {
				int lib = 0;
				for (int k = 0; k < 4; ++k) {
					int bi = i + deltai[k];
					int bj = j + deltaj[k];
					if (on_board(bi, bj) && get_board(bi, bj) == EMPTY)
						++lib;
				}
				if (lib > 1)
					final_moves[num_final_move++] = POS(i, j);
			}
		}
	}
	if (num_final_move > 0)
		pos = final_moves[rand() % num_final_move];
	return pos;
}


int GoBoard::AtariDefense(int moves[], int num_moves, int color) {
	int pos = -1;
	int tmp;
	int final_moves[BOARD_SIZE*BOARD_SIZE];
	int num_final_move = 0;
	bool flag = true;
	for (int k = 0; k < 4; ++k) {
		int ai = li + deltai[k];
		int aj = lj + deltaj[k];
		//in atari
		if (on_board(ai, aj) && get_board(ai, aj) == color && liberties[POS(ai, aj)] == 1) {
			//defense by capturing
			tmp = POS(ai, aj);
			do {
				if (!flag)
					break;
				//find the opponent's stone whose liberty is 1
				for (int k = 0; k < 4; ++k) {
					if (!flag)
						break;
					int bi = I(tmp) + deltai[k];
					int bj = J(tmp) + deltaj[k];
					if (on_board(bi, bj) && get_board(bi, bj) == OTHER_COLOR(color) && liberties[POS(bi, bj)] == 1) {
						int tmp2 = POS(bi, bj);
						do {
							if (!flag)
								break;
							for (int k = 0; k < 4; ++k) {
								if (!flag)
									break;
								int ci = I(tmp2) + deltai[k];
								int cj = J(tmp2) + deltaj[k];
								if (on_board(ci, cj) && get_board(ci, cj) == EMPTY) {
									flag = false;
									if (legal_move(ci, cj, color)) {
										int lib = 0;
										for (int k = 0; k < 4; ++k) {
											int di = ci + deltai[k];
											int dj = cj + deltaj[k];
											if (on_board(di, dj) && get_board(di, dj) == EMPTY)
												++lib;
										}
										if (lib > 1)
											return POS(ci, cj);
									}
								}
							}

							tmp2 = next_stone[tmp2];
						} while (tmp2 != POS(bi, bj));
					}
				}

				tmp = next_stone[tmp];
			} while (tmp != POS(ai, aj));

			//just defense
			tmp = POS(ai, aj);
			do {
				//printf("p1\n");
				for (int k = 0; k < 4; ++k) {
					int bi = I(tmp) + deltai[k];
					int bj = J(tmp) + deltaj[k];
					if (on_board(bi, bj) && get_board(bi, bj) == EMPTY) {
						//return POS(bi, bj);
						int lib = 0;
						for (int k = 0; k < 4; ++k) {
							int ci = bi + deltai[k];
							int cj = bj + deltaj[k];
							if (on_board(ci, cj) && get_board(ci, cj) == EMPTY)
								++lib;
						}
						if (lib > 1)
							return POS(bi, bj);
					}
				}
			} while (tmp != POS(ai, aj));

		}
	}

	return pos;
}

int GoBoard::Capture(int moves[], int num_moves, int color) {
	int pos = -1;
	int final_moves[BOARD_SIZE*BOARD_SIZE];
	int num_final_move = 0;
	for (int n = 0; n < num_moves; ++n) {
		for (int k = 0; k < 4; ++k) {
			int i = I(moves[n]);
			int j = J(moves[n]);
			int ai = i + deltai[k];
			int aj = j + deltaj[k];
			if (on_board(ai, aj) && get_board(ai, aj) == OTHER_COLOR(color) && liberties[POS(ai, aj)] == 1) {
				int lib = 0;
				for (int k = 0; k < 4; ++k) {
					int bi = i + deltai[k];
					int bj = j + deltaj[k];
					if (on_board(bi, bj) && get_board(bi, bj) == EMPTY)
						++lib;
				}
				if (lib > 1)
					final_moves[num_final_move++] = POS(i, j);
			}
		}
	}
	if (num_final_move > 0)
		pos = final_moves[rand() % num_final_move];
	return pos;
}


double GoBoard::simulation(int start_color) {
	double result = 0;
	int color = start_color;
	int cnt = 0;

	while (true) {
		int i, j;
		generate_move(&i, &j, color);
		if (++cnt == 500) return rand() % 2;
		if (pass_move(i, j)) {
			break;
		}
		play_move(i, j, color);
		color = OTHER_COLOR(color);
	}
	//std::cout << "step R3" << std::endl;
	compute_final_status();
	result += (start_color == WHITE) ? KOMI : -KOMI;
	for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i) {
		if (final_status[i] == BLACK_TERRITORY || final_status[i] == ALIVE && board[i] == BLACK) {
			if (start_color == BLACK) result += 1;
			else result -= 1;
		}
		if (final_status[i] == WHITE_TERRITORY || final_status[i] == ALIVE && board[i] == WHITE) {
			if (start_color == BLACK) result -= 1;
			else result += 1;
		}
	}
	return result;
}

void GoBoard::libertiesCal() {
	int verified[BOARD_SIZE*BOARD_SIZE];
	bool counted[BOARD_SIZE*BOARD_SIZE];
	memset(counted, false, sizeof(counted));
	for (int m = 0; m < BOARD_SIZE*BOARD_SIZE; ++m)
	{
		if (board[m] == EMPTY)
			liberties[m] = -1;
		else if (!counted[m])
		{
			int tmp = m;
			int result = 0;
			do {
				int ai = I(m);
				int aj = J(m);
				int k;
				for (k = 0; k < 4; k++) {
					int bi = ai + deltai[k];
					int bj = aj + deltaj[k];
					if (on_board(bi, bj) && get_board(bi, bj) == EMPTY
						&& (!libertyCount[POS(bi, bj)])) {
						libertyCount[POS(bi, bj)] = true;
						verified[result++] = POS(bi, bj);
					}
				}
				counted[m] = true;
				m = next_stone[m];
			} while (m != tmp);
			//m = tmp;
			for (int k = 0; k < result; ++k) {
				libertyCount[verified[k]] = false;
			}
			do {
				liberties[m] = result;
				m = next_stone[m];
			} while (m != tmp);
		}
	}
}


void GoBoard::libertiesUpdate(int pos) {
	int result = NumOfLiberties(I(pos), J(pos));
	int tmp = pos;
	do {
		liberties[pos] = result;
		pos = next_stone[pos];
	} while (pos != tmp);

}


void GoBoard::generateAllMovesWithPriority(int color, int moves[]) {
	memset(moves, 0, sizeof(int)*BOARD_SIZE*BOARD_SIZE); // initial as zero
	int get_moves[BOARD_SIZE*BOARD_SIZE];
	int capture_moves[BOARD_SIZE*BOARD_SIZE];
	int pattern_moves[BOARD_SIZE * BOARD_SIZE];
	int num_moves = 0;
	int num_capture_moves = 0;

	//Get legal moves and RANDOM moves by the way.
	for (int ai = 0; ai < BOARD_SIZE; ai++)
	for (int aj = 0; aj < BOARD_SIZE; aj++) {
		/* Consider moving at (ai, aj) if it is legal and not suicide. */
		if (legal_move(ai, aj, color)
			&& !suicide(ai, aj, color)) {
			/* Further require the move not to be suicide for the opponent... */
			if (!suicide(ai, aj, OTHER_COLOR(color))) {
				capture_moves[num_capture_moves++] = POS(ai, aj);
				get_moves[num_moves++] = POS(ai, aj);
				//RANDOM
				moves[POS(ai, aj)] = RANDOM;
			}
			else {
				capture_moves[num_capture_moves++] = POS(ai, aj);
				/* ...however, if the move captures at least one stone,
				* consider it anyway.
				*/
				for (int k = 0; k < 4; k++) {
					int bi = ai + deltai[k];
					int bj = aj + deltaj[k];
					if (on_board(bi, bj) && get_board(bi, bj) == OTHER_COLOR(color)) {
						get_moves[num_moves++] = POS(ai, aj);
						//RANDOM
						moves[POS(ai, aj)] = RANDOM;
						break;
					}
				}
			}
		}
	}

	// ATARI
	for (int n = 0; n < num_moves; ++n) {
		for (int k = 0; k < 4; ++k) {
			int i = I(get_moves[n]);
			int j = J(get_moves[n]);
			int ai = i + deltai[k];
			int aj = j + deltaj[k];
			if (on_board(ai, aj) && get_board(ai, aj) == OTHER_COLOR(color) && liberties[POS(ai, aj)] == 2) {
				if (moves[POS(i, j)] > ATARI)
					continue;
				int lib = 0;
				for (int k = 0; k < 4; ++k) {
					int bi = i + deltai[k];
					int bj = j + deltaj[k];
					if (on_board(bi, bj) && get_board(bi, bj) == EMPTY)
						++lib;
				}
				if (lib > 1)
					moves[POS(i, j)] = ATARI;
			}
		}
	}

	//ATARI_DEFENCE
	bool flag = true;
	for (int k = 0; k < 4; ++k) {
		int ai = li + deltai[k];
		int aj = lj + deltaj[k];
		//in atari
		if (on_board(ai, aj) && get_board(ai, aj) == color && liberties[POS(ai, aj)] == 1) {
			//defense by capturing
			int tmp = POS(ai, aj);
			do {
				if (!flag)
					break;
				//find the opponent's stone whose liberty is 1
				for (int k = 0; k < 4; ++k) {
					if (!flag)
						break;
					int bi = I(tmp) + deltai[k];
					int bj = J(tmp) + deltaj[k];
					if (on_board(bi, bj) && get_board(bi, bj) == OTHER_COLOR(color) && liberties[POS(bi, bj)] == 1) {
						int tmp2 = POS(bi, bj);
						do {
							if (!flag)
								break;
							for (int k = 0; k < 4; ++k) {
								if (!flag)
									break;
								int ci = I(tmp2) + deltai[k];
								int cj = J(tmp2) + deltaj[k];
								if (on_board(ci, cj) && get_board(ci, cj) == EMPTY) {
									flag = false;
									if (moves[POS(ci, cj)] > ATARI_DEFENCE)
										break;
									if (legal_move(ci, cj, color)) {
										//return POS(ci, cj);
										int lib = 0;
										for (int k = 0; k < 4; ++k) {
											int di = ci + deltai[k];
											int dj = cj + deltaj[k];
											if (on_board(di, dj) && get_board(di, dj) == EMPTY)
												++lib;
										}
										if (lib > 1)
											moves[POS(ci, cj)] = ATARI_DEFENCE;
									}
								}
							}

							tmp2 = next_stone[tmp2];
						} while (tmp2 != POS(bi, bj));
					}
				}

				tmp = next_stone[tmp];
			} while (tmp != POS(ai, aj));

			//just defense
			tmp = POS(ai, aj);
			do {
				for (int k = 0; k < 4; ++k) {
					int bi = I(tmp) + deltai[k];
					int bj = J(tmp) + deltaj[k];
					if (on_board(bi, bj) && get_board(bi, bj) == EMPTY) {
						if (moves[POS(bi, bj)] > ATARI_DEFENCE)
							break;
						int lib = 0;
						for (int k = 0; k < 4; ++k) {
							int ci = bi + deltai[k];
							int cj = bj + deltaj[k];
							if (on_board(ci, cj) && get_board(ci, cj) == EMPTY)
								++lib;
						}
						if (lib > 1)
							moves[POS(bi, bj)] = ATARI_DEFENCE;
					}
				}
				tmp = next_stone[tmp];
			} while (tmp != POS(ai, aj));
		}
	}

	//PATTERN
	memset(pattern_moves, 0, sizeof(pattern_moves));
	int num_pattern = pattern_matching(1, color, pattern_moves, moves);
	for (int i = 0; i < num_pattern; ++i){
		if (moves[pattern_moves[i]] < PATTERN) moves[pattern_moves[i]] = PATTERN;
	}
	
	//CAPTURE
	for (int n = 0; n < num_capture_moves; ++n) {
		for (int k = 0; k < 4; ++k) {
			int i = I(capture_moves[n]);
			int j = J(capture_moves[n]);
			int ai = i + deltai[k];
			int aj = j + deltaj[k];
			if (on_board(ai, aj) && get_board(ai, aj) == OTHER_COLOR(color) && liberties[POS(ai, aj)] == 1) {
				if (moves[POS(i, j) > CAPTURE])
					continue;
				//continue;
				//final_moves[num_final_move++] = POS(i, j);
				int lib = 0;
				for (int k = 0; k < 4; ++k) {
					int bi = i + deltai[k];
					int bj = j + deltaj[k];
					if (on_board(bi, bj) && get_board(bi, bj) == EMPTY)
						++lib;
				}
				if (lib > 1)
					moves[POS(i, j)] = CAPTURE;
			}
		}
	}

	//RANDOM
	//RANDOM moves have been set above

	//INVALID nothing need to write
}


int GoBoard::defaultPolicy(int color) {
	int i, j;
	generate_move(&i, &j, color);
	return POS(i, j);
}

vector<int> GoBoard::generate_all_moves(int color)
{
	int moves[BOARD_SIZE * BOARD_SIZE];
	int num_moves = 0;
	int ai, aj;
	int k;
	int start = clock();

	memset(moves, 0, sizeof(moves));
	for (ai = 0; ai < BOARD_SIZE; ai++)
	for (aj = 0; aj < BOARD_SIZE; aj++) {
		/* Consider moving at (ai, aj) if it is legal and not suicide. */
		if (legal_move(ai, aj, color)
			&& !suicide(ai, aj, color)) {
			/* Further require the move not to be suicide for the opponent... */
			if (!suicide(ai, aj, OTHER_COLOR(color)))
				moves[num_moves++] = POS(ai, aj);
			else {
				/* ...however, if the move captures at least one stone,
				* consider it anyway.
				*/
				for (k = 0; k < 4; k++) {
					int bi = ai + deltai[k];
					int bj = aj + deltaj[k];
					if (on_board(bi, bj) && get_board(bi, bj) == OTHER_COLOR(color)) {
						moves[num_moves++] = POS(ai, aj);
						break;
					}
				}
			}
		}
	}

	vector<int> res;
	for (int i = 0; i < num_moves; ++i) {
		res.push_back(moves[i]);
	}
	return res;
}

int GoBoard::blackWins() {
	compute_final_status();
	double result = -KOMI;
	for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i) {
		if (final_status[i] == BLACK_TERRITORY || final_status[i] == ALIVE && board[i] == BLACK) {
			result += 1;
		}
		if (final_status[i] == WHITE_TERRITORY || final_status[i] == ALIVE && board[i] == WHITE) {
			result -= 1;
		}
	}
	return result > 0 ? 1 : 0;
}

int GoBoard::lastMove() {
	return POS(li, lj);
}

/*
* Local Variables:
* tab-width: 8
* c-basic-offset: 2
* End:
*/
