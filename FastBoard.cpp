#include "FastBoard.h"
#include "ComplexEvaluator.h"
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <algorithm>
using namespace std;
const double FastBoard::KOMI = -6.5;

const int delta[] = { FastBoard::BOARD_SIZE, -FastBoard::BOARD_SIZE, 1, -1,
FastBoard::BOARD_SIZE - 1, FastBoard::BOARD_SIZE + 1, -FastBoard::BOARD_SIZE - 1, -FastBoard::BOARD_SIZE + 1 };

FastBoard::FastBoard()
{

}

FastBoard::FastBoard(int initialBoard[], int koPos, int lastPos) {
	
	stringCount = 0;
	nb = nw = 0;
	memset(board, 0, sizeof(board));
	memset(hasLiberty, 0, sizeof(hasLiberty));
	memset(isInAtari, 0, sizeof(isInAtari));

	this->koPos = this->lastPos = -2;
	for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i) {
		if (initialBoard[i] != EMPTY) {
			playMove(i, initialBoard[i]);
		}
	}

	this->koPos = koPos;
	this->lastPos = lastPos;
}

FastBoard::~FastBoard()
{
}

inline
bool FastBoard::onBoard(int pos, int k) {
	int i = pos / 13;
	int j = pos % 13;
	int ti = i + delta[k] / 13;
	int tj = j + delta[k] % 13;
	return ti >= 0 && ti < 13 && tj >= 0 && tj < 13;
}

/* make sure that board[pos] == EMPTY */
inline bool FastBoard::provideLiberty(int str, int pos) {
	if (hasLiberty[str][pos]) return false;
	++strings[str].liberties;
	hasLiberty[str][pos] = true;
	return true;
}

inline bool FastBoard::reduceLiberty(int str, int pos) {
	if (!hasLiberty[str][pos]) return false;
	--strings[str].liberties;
	hasLiberty[str][pos] = false;
	return true;
}

void FastBoard::playMove(int pos, int color) {
	if (pos == PASS_MOVE) return;

	int other = otherColor(color);
	board[pos] = color;

	int s = ++stringCount;
	stringNumber[pos] = s;
	strings[s].color = color;
	strings[s].origin = strings[s].tail = pos;
	nextStone[pos] = pos;
	strings[s].size = 1;

	/* initialize the liberty */
	strings[s].liberties = 0;
	int N = north(pos); if (onBoard(pos, 1) && board[N] == EMPTY) provideLiberty(s, N);
	int S = south(pos); if (onBoard(pos, 0) && board[S] == EMPTY) provideLiberty(s, S);
	int W = west(pos);  if (onBoard(pos, 3) && board[W] == EMPTY) provideLiberty(s, W);
	int E = east(pos);  if (onBoard(pos, 2) && board[E] == EMPTY) provideLiberty(s, E);

	/* capture simulation */
	int captureNumber = 0, captured;
	for (int k = 0; k < 4; ++k) {
		int neighbor = pos + delta[k];
		if (onBoard(pos, k) && board[neighbor] != EMPTY) {
			int str = stringNumber[pos + delta[k]];
			if (reduceLiberty(str, pos) && board[neighbor] == other) { // here both color&other 's liberty would be reduced
				int liberty = countLiberties(str);
				if (liberty == 0) {
					/* remove the string */
					captureNumber += removeString(str);
					captured = pos + delta[k];
				}
				if (liberty == 1 && !isInAtari[str]) {
					/* in atari */
					isInAtari[str] = true;
					if (other == BLACK) {
						blackInAtari[nb++] = str;
					} else {
						whiteInAtari[nw++] = str;
					}
				}
			}
		}
	}

	
	/* connect if needed*/
	for (int k = 0; k < 4; ++k) {
		int neighbor = pos + delta[k];
		if (onBoard(pos, k) && board[neighbor] == color && stringNumber[pos] != stringNumber[neighbor]) {
			s = connectString(s, stringNumber[neighbor]);
		}
	}

	/* check koPos & lastPos*/
	koPos = -1;
	lastPos = pos;
	if (captureNumber == 1 && countLiberties(s) == 1) {
		koPos = captured;
	}

	/*capture information*/
	if (countLiberties(s) == 1 && !isInAtari[s]) {
		isInAtari[s] = true;
		if (color == BLACK) {
			blackInAtari[nb++] = s;
		}
		else {
			whiteInAtari[nw++] = s;
		}
	}
}

/* remove a string, the stringNumber is not changed */
int FastBoard::removeString(int str) {
	//ofstream out("my_log.txt", std::fstream::app);
	//out << "Remmoving!" << endl;
	int origin = strings[str].origin;
	int pos = origin;
	do {
		int N = north(pos); if (onBoard(pos, 1) && board[N] != EMPTY) provideLiberty(stringNumber[N], pos);
		int S = south(pos); if (onBoard(pos, 0) && board[S] != EMPTY) provideLiberty(stringNumber[S], pos);
		int W = west(pos);  if (onBoard(pos, 3) && board[W] != EMPTY) provideLiberty(stringNumber[W], pos);
		int E = east(pos);  if (onBoard(pos, 2) && board[E] != EMPTY) provideLiberty(stringNumber[E], pos);
		board[pos] = EMPTY;
		pos = nextStone[pos];
	} while (pos != origin);
	
	strings[str].liberties = 0;

	return strings[str].size;
}

/* return the connected number of the result string*/

int FastBoard::connectString(int str1, int str2) {

	/* ensure that str1 is larger*/

	if (strings[str1].size < strings[str2].size) {
		int tmp = str1;
		str1 = str2;
		str2 = tmp;
	}

	string_data& s1 = strings[str1];
	string_data& s2 = strings[str2];

	/* mark all nodes in str2 */
	int origin = s2.origin;
	int pos = origin;
	do {
		stringNumber[pos] = str1;

		int N = north(pos); if (onBoard(pos, 1) && board[N] == EMPTY) provideLiberty(str1, N);
		int S = south(pos); if (onBoard(pos, 0) && board[S] == EMPTY) provideLiberty(str1, S);
		int W = west(pos);  if (onBoard(pos, 3) && board[W] == EMPTY) provideLiberty(str1, W);
		int E = east(pos);  if (onBoard(pos, 2) && board[E] == EMPTY) provideLiberty(str1, E);

		pos = nextStone[pos];
	} while (pos != origin);

	/* connect the two circle links */
	nextStone[s1.tail] = s2.origin;
	nextStone[s2.tail] = s1.origin;

	/* update information */
	s1.tail = s2.tail;
	s1.size += s2.size;
	s2.liberties = 0; // mark as removes

	return str1;
}

bool FastBoard::rationalMove(int pos, int color) {
	int other = otherColor(color);
	if (leagelMove(pos)) {

		for (int k = 0; k < 4; ++k) {
			int neighbor = pos + delta[k];
			if (onBoard(pos, k)) {
				if (board[neighbor] == EMPTY) return true;
			}
		}

		bool moreliberty = false, co_moreliberty = false, capture = false, co_capture = false;
		for (int k = 0; k < 4; ++k) {
			if (onBoard(pos, k)) {
				string_data& s = strings[stringNumber[pos + delta[k]]];
				if (s.color == color) {
					if (s.liberties > 1) {
						moreliberty = true;
					}
					else {
						co_capture = true;
					}
				}
				else {
					if (s.liberties > 1) {
						co_moreliberty = true;
					}
					else {
						capture = true;
					}
				}
			}
		}
		if (!moreliberty && !capture) return false; // suicide
		if (!(!co_moreliberty && !co_capture)) return true; // opponent not suicided
		// capture
		int N = north(pos); if (onBoard(pos, 1) && board[N] == other) return true;
		int S = south(pos); if (onBoard(pos, 0) && board[S] == other) return true;
		int W = west(pos);  if (onBoard(pos, 3) && board[W] == other) return true;
		int E = east(pos);  if (onBoard(pos, 2) && board[E] == other) return true;
	}
	return false;
}

int FastBoard::generateMove(int color) {
	int move;

	if ((move = nakadeMove(color)) != PASS_MOVE) return move;
	if ((move = atariDefenseMove(color)) != PASS_MOVE) return move;
	if ((move = fillBoardMove(color)) != PASS_MOVE) return move;
	if ((move = patternMove(color)) != PASS_MOVE) return move;
	if ((move = captureMove(color)) != PASS_MOVE) return move;
	
	return randomMove(color);
}


int FastBoard::nakadeMove(int color) {
	if (lastPos == PASS_MOVE)
		return PASS_MOVE;

	for (int k = 0; k < 4; ++k) {
		int neighbor = lastPos + delta[k];
		if (onBoard(lastPos, k)) {
			int res = checkNakade(neighbor, color);
			if (res != -1)
				return res;
		}
	}

	return PASS_MOVE;
}


int FastBoard::checkNakade(int pos, int color) {
	if (board[pos] != EMPTY)
		return -1;

	int hole = 0;
	int lastPos = PASS_MOVE;
	int res;
	int availDir;

	do {
		++hole;
		if (hole > 3)
			return -1;

		availDir = 0;
		int currentPos = pos;
		int lastPos_tmp = lastPos;
		for (int k = 0; k < 4; ++k) {
			int neighbor = currentPos + delta[k];
			/* Not surrounded. */
			if (onBoard(currentPos, k) && board[neighbor] == color)
				return -1;

			if (onBoard(currentPos, k) && neighbor != lastPos_tmp && board[neighbor] == EMPTY) {
				++availDir;
				lastPos = currentPos;
				pos = neighbor;
			}
		}
		/* Not surrounded. */
		if (availDir > 1)
			return -1;

		if (hole == 2)
			res = currentPos;
	} while (availDir != 0);

	if (hole == 3)
		return res;

	return -1;
}


int FastBoard::atariDefenseMove(int color) {
	if (lastPos == PASS_MOVE)
		return PASS_MOVE;

	static int moves[BOARD_SIZE * BOARD_SIZE];
	static bool in[STRING_SIZE];
	static bool visited[STRING_SIZE];

	int moveNumber = 0;

	for (int k = 0; k < 4; ++k) {
		int neighbor = lastPos + delta[k];
		/* In atari. */
		if (onBoard(lastPos, k) && board[neighbor] == color && inAtari(neighbor)) {
			/* Find out the unique liberty. */

			in[stringNumber[neighbor]] = true;

			int res = findAtariLiberty(stringNumber[neighbor]);
			/* Whether it is futile*/
			int lib = 0;
			for (int m = 0; m < 4; ++m) {
				if (!onBoard(res, m))
					continue;
				int neighbor2 = res + delta[m];
				if (board[neighbor2] == EMPTY) {
					++lib;
					continue;
				}

				int str = stringNumber[neighbor2];
				if (visited[str])
					continue;

				visited[str] = true;
				
				if (board[neighbor2] == color)
					lib += countLiberties(str) - 1;
			}
			/* restore */
			for (int m = 0; m < 4; ++m) {
				if (!onBoard(res, m))
					continue;
				int neighbor2 = res + delta[m];
				if (board[neighbor2] == EMPTY) continue;
				visited[stringNumber[neighbor2]] = false;
			}

			if (lib > 1)
				moves[moveNumber++] = res;
		}
	}

	/* capture defence */
	int *inAtaris, *n;
	if (otherColor(color) == BLACK) {
		inAtaris = blackInAtari;
		n = &nb;
	}
	else {
		inAtaris = whiteInAtari;
		n = &nw;
	}

	int tn = 0;
	for (int k = 0; k < *n; ++k) {
		int str = inAtaris[k];
		if (countLiberties(str) == 1 && board[strings[str].origin] != EMPTY) {
			inAtaris[tn] = str;
			++tn;
		}
		else {
			isInAtari[str] = false;
		}
	}
	*n = tn;

	for (int k = 0; k < tn; ++k) {

		int str = inAtaris[k];
		int pos = strings[str].origin;
		int capture = PASS_MOVE;
		bool save = false;
		do {
			int N = north(pos);
			if (onBoard(pos, 1)) {
				if (board[N] == EMPTY && N != koPos) capture = N;
				if (board[N] == color && in[stringNumber[N]]) save = true;
			}
			int S = south(pos);
			if (onBoard(pos, 0)) {
				if (board[S] == EMPTY && S != koPos) capture = S;
				if (board[S] == color && in[stringNumber[S]]) save = true;
			}
			int W = west(pos);
			if (onBoard(pos, 3)) {
				if (board[W] == EMPTY && W != koPos) capture = W;
				if (board[W] == color && in[stringNumber[W]]) save = true;
			}
			int E = east(pos);
			if (onBoard(pos, 2)) {
				if (board[E] == EMPTY && E != koPos) capture = E;
				if (board[E] == color && in[stringNumber[E]]) save = true;
			}
			pos = nextStone[pos];
		} while (pos != strings[str].origin);// ko

		if (capture != PASS_MOVE && save) {
			moves[moveNumber++] = capture;
		}
	}

	/*restore*/
	for (int k = 0; k < 4; ++k) {
		int neighbor = lastPos + delta[k];
		if (onBoard(lastPos, k) && board[neighbor] == color && inAtari(neighbor)) {
			in[stringNumber[neighbor]] = false;
		}
	}


	/* collect */
	if (moveNumber > 0) {
		return moves[rand() % moveNumber];
	}
	return PASS_MOVE;
}



bool FastBoard::inAtari(int pos) {
	return strings[stringNumber[pos]].liberties <= 1;
}


int FastBoard::findAtariLiberty(int str) {
	int origin = strings[str].origin;
	int pos = origin;
	do {
		int N = north(pos); if (onBoard(pos, 1) && board[N] == EMPTY) return N;
		int S = south(pos); if (onBoard(pos, 0) && board[S] == EMPTY) return S;
		int W = west(pos);  if (onBoard(pos, 3) && board[W] == EMPTY) return W;
		int E = east(pos);  if (onBoard(pos, 2) && board[E] == EMPTY) return E;
		pos = nextStone[pos];
	} while (pos != origin);

	return PASS_MOVE;
}

/* return the number of empty around pos*/
int FastBoard::numberOfEmpty(int pos) {
	int empty = 0;

	for (int k = 0; k < 4; ++k) {
		int neighbor = pos + delta[k];
		if (onBoard(pos, k) && board[neighbor] == EMPTY)
			++empty;
	}

	return empty;
}


int FastBoard::fillBoardMove(int color) {
	for (int n = 0; n < 6; ++n) {
		int move = rand() % 169;
		if (!leagelMove(move))
			continue;

		/* Ensure not on the line of death. */
		if (getI(move) == 0 || getI(move) == BOARD_SIZE - 1
			|| getJ(move) == 0 || getJ(move) == BOARD_SIZE - 1) {
			continue;
		}

		int empty = 0;
		for (int k = 0; k < 8; ++k) {
			int neighbor = move + delta[k];
			if (board[neighbor] == EMPTY)
				++empty;
		}
		if (empty == 8)
			return move;
	}

	return PASS_MOVE;
}


int FastBoard::randomMove(int color) {
	for (int k = 0; k < 18; ++k) {
	int r = rand() % 169;
	if (rationalMove(r, color)) return r;
	}

	static int moves[BOARD_SIZE * BOARD_SIZE];
	int moveNumber = 0;
	for (int i = 0; i < 169; ++i) {
		if (rationalMove(i, color)) {
			moves[moveNumber++] = i;
		}
	}
	if (moveNumber == 0) return PASS_MOVE;
	return moves[rand() % moveNumber];
}


/* it's different with brown why ? */
inline
bool FastBoard::leagelMove(int pos) {
	return pos == PASS_MOVE || (board[pos] == EMPTY && pos != koPos);
}


void FastBoard::show() {
	ofstream out("my_log.txt", std::fstream::app);
	out << "------------------------\n";
	for (int i = 0; i < BOARD_SIZE; ++i) {
		for (int j = 0; j < BOARD_SIZE; ++j) {
			if (board[i * BOARD_SIZE + j] == EMPTY) out << '.';
			if (board[i * BOARD_SIZE + j] == BLACK) out << 'X';
			if (board[i * BOARD_SIZE + j] == WHITE) out << 'O';
		}
		out << endl;
	}
	out << endl;
	/*out << "***********************\n";
	char f[] = "0123456789ZXCVBNMLKJHGFDSAQWERTYUIOPqwertyuiooplkjhgfdsazxcvbnm";
	for (int i = 0; i < BOARD_SIZE; ++i) {
		for (int j = 0; j < BOARD_SIZE; ++j) {
			if (board[i * BOARD_SIZE + j] == EMPTY) out << '.';

			else out << f[stringNumber[(i * BOARD_SIZE + j)] % 50];
		}
		out << endl;
	}
	out << endl;
	
	for (int i = 0; i < BOARD_SIZE; ++i) {
		for (int j = 0; j < BOARD_SIZE; ++j) {
			if (board[i * BOARD_SIZE + j] == EMPTY) out << '.';

			else out << strings[stringNumber[(i * BOARD_SIZE + j)]].liberties;
		}
		out << endl;
	}
	out << endl;*/
}

void FastBoard::debug() {
	int color = BLACK;
	while (true) {
		int move = generateMove(color);
		if (move == PASS_MOVE) break;
		cout << move / 13 << ' ' << move % 13 << endl;
		playMove(move, color);
		show();
		color = otherColor(color);
	}
}

void FastBoard::simulate() {
	int color = BLACK;
	int cnt = 0;
	while (true) {
		int move = generateMove(color);
		if (move == PASS_MOVE) break;
		playMove(move, color);
		color = otherColor(color);
		if (++cnt > STRING_SIZE - 20) break;
	}
}

int FastBoard::captureMove(int color) {
	int *inAtari, *n;
	if (otherColor(color) == BLACK) {
		inAtari = blackInAtari;
		n = &nb;
	} else {
		inAtari = whiteInAtari;
		n = &nw;
	}

	int tn = 0;
	for (int k = 0; k < *n; ++k) {
		int str = inAtari[k];
		if (countLiberties(str) == 1 && board[strings[str].origin] != EMPTY) {
			inAtari[tn] = str;
			++tn;
			
		} else {
			isInAtari[str] = false;
		}
	}
	*n = tn;

	if (tn > 0) {
		int str = inAtari[rand() % tn];
		int pos = strings[str].origin;
		do {
			int N = north(pos); if (onBoard(pos, 1) && board[N] == EMPTY && N != koPos) {
				return N; // for leagal only ko is needed 
			}
			int S = south(pos); if (onBoard(pos, 0) && board[S] == EMPTY && S != koPos) {
				return S;
			}
			int W = west(pos);  if (onBoard(pos, 3) && board[W] == EMPTY && W != koPos) {
				return W;
			}
			int E = east(pos);  if (onBoard(pos, 2) && board[E] == EMPTY && E != koPos) {
				return E;
			}
			pos = nextStone[pos];
		} while (pos != strings[str].origin);// ko
	}
	return PASS_MOVE;
}

/* pattern */
static const int square3i[9] = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };
static const int square3j[9] = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };

int  FastBoard::is_pattern_for_Hane(int i, int j, int color){
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

int FastBoard::is_pattern_for_cut1(int i, int j, int color){
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

int  FastBoard::is_pattern_for_cut2(int i, int j, int color){
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

int  FastBoard::is_pattern_for_side(int i, int j, int color){
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
int FastBoard::is_pattern(int i, int j, int color){
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



int FastBoard::pattern_matching(int len, int color, int moves[]){
	int num_moves = 0;
	for (int ii = -len; ii <= len; ++ii)
	for (int jj = -len; jj <= len; ++jj){
		int ai = getI(lastPos) + ii;
		int aj = getJ(lastPos) + jj;
		int pos = getPos(ai, aj);
		if (on_board(ai, aj) && rationalMove(pos, color) && is_pattern(ai, aj, color)){
			moves[num_moves++] = pos;
		}
	}
	return num_moves;
}

int FastBoard::patternMove(int color) {
	static int moves[10];
	int num_moves = pattern_matching(1, color, moves);
	if (num_moves == 0) return PASS_MOVE;
	return moves[rand() % num_moves];
}

/* evaluation here */
void FastBoard::evaluate(int color, double h[]) {
	static int features[BOARD_SIZE * BOARD_SIZE];
	ComplexEvaluator(this).board_evaluation(features, color);

	for (int pos = 0; pos < BOARD_SIZE * BOARD_SIZE; ++pos) {
		// the move must be rational
		if (!rationalMove(pos, color)) {
			h[pos] = -1;
			continue;
		}

		// initialize
		double ability = 1;
		int i = pos / BOARD_SIZE;
		int j = pos % BOARD_SIZE;


		
		// distance to the edge 
		int distance = min(min(i, BOARD_SIZE - i - 1), min(j, BOARD_SIZE - j - 1));
		if (distance == 0) ability *= 0.7;
		if (distance == 2) ability *= 0.9;
		if (distance == 3) ability *= 1.2;
		if (distance == 4) ability *= 1.5;
		if (distance > 4)  ability *= 1;

		// distance to the lastPoint
		int distance2 = abs(i - getI(lastPos)) + abs(j - getJ(lastPos));
		if (distance2 < 2) ability *= 1.5;
		if (distance2 >= 2 && distance2 < 5) ability *= 1.3;
		if (distance2 >= 5 && distance2 < 9) ability *= 1.1;

		ability = min(ability, 1.5);

		// atari, defence and capture
		bool atari = false;
		bool defence = false;
		bool capture = false;
		bool neighborInAtalri = false;

		static bool visited[STRING_SIZE];
		int totalLiberties = 0;

		for (int k = 0; k < 4; ++k) {
			// out of board
			if (!onBoard(pos, k)) continue;

			int neighbor = pos + delta[k];
			
			// empty position
			if (board[neighbor] == EMPTY) {
				totalLiberties++;
				continue;
			}

			int str = stringNumber[neighbor];
			if (visited[str]) continue;
			visited[str] = true;

			if (board[neighbor] == color) {
				totalLiberties += countLiberties(str) - 1; // connection
				if (countLiberties(str) == 1) {
					neighborInAtalri = true;
				}
			}
			else {
				if (countLiberties(str) == 2) {
					// atari
					atari = true;
				}
				if (countLiberties(str) == 1) {
					// capture and maybe defence
					capture = true;

					// find if the capture has a defence effect
					int pos = strings[str].origin;
					do {
						int N = north(pos);
						if (onBoard(pos, 1) && board[N] == color && countLiberties(stringNumber[N]) == 1) {
							defence = true;
						}
						int S = south(pos);
						if (onBoard(pos, 0) && board[S] == color && countLiberties(stringNumber[S]) == 1) {
							defence = true;
						}
						int W = west(pos);
						if (onBoard(pos, 3) && board[W] == color && countLiberties(stringNumber[W]) == 1) {
							defence = true;
						}
						int E = east(pos);
						if (onBoard(pos, 2) && board[E] == color && countLiberties(stringNumber[E]) == 1) {
							defence = true;
						}
						pos = nextStone[pos];
					} while (!defence && pos != strings[str].origin);
				}
			}
		}


		// restore
		for (int k = 0; k < 4; ++k) {
			// out of board
			if (!onBoard(pos, k)) continue;
			int neighbor = pos + delta[k];
			// empty position
			if (board[neighbor] == EMPTY) {
				continue;
			}
			int str = stringNumber[neighbor];
			visited[str] = false;
		}

		if (totalLiberties >= 2 && neighborInAtalri) {
			defence = true;
		}

		if (!capture && totalLiberties <= 1) {
			// self atari
			ability *= 0.29;
		}

		if (atari) ability *= 3.8;
		if (defence) ability *= 5.2;
		if (!defence && !atari && capture) ability *= 1.7;
		
		// features
		
		

		double fp = 1;
		if (features[pos] & (1 << ComplexEvaluator::POS_BAD_KOGEIMA)) {
			fp *= 0.7;
		}
		if (features[pos] & (1 << ComplexEvaluator::POS_BAD_TOBI)) {
			fp *= 0.7;
		}
		if (features[pos] & (1 << ComplexEvaluator::POS_CONNECT)) {
			fp *= 1.6;
		}
		if (features[pos] & (1 << ComplexEvaluator::POS_EMPTY_TRIANGLE1)) {
			fp *= 0.7;
		}
		if (features[pos] & (1 << ComplexEvaluator::POS_EMPTY_TRIANGLE2)) {
			fp *= 0.7;
		}
		if (features[pos] & (1 << ComplexEvaluator::POS_HANE)) {
			fp *= 1.6;
		}
		if (features[pos] & (1 << ComplexEvaluator::POS_KATA)) {
			fp *= 1.6;
		}
		if (features[pos] & (1 << ComplexEvaluator::POS_KOGEIMA)) {
			fp *= 1.5;
		}
		if (features[pos] & (1 << ComplexEvaluator::POS_KOSUMI)) {
			fp *= 1.5;
		}
		if (features[pos] & (1 << ComplexEvaluator::POS_PEEP_CONNECT)) {
			fp *= 1.7;
		}
		if (features[pos] & (1 << ComplexEvaluator::POS_WALL)) {
			fp *= 1.45;
		}

		if (!capture && !defence && !atari) ability *= min(1.5, fp);
		
		
		// collection
		h[pos] = ability / (1 + ability);
	}
}



bool FastBoard::blackWins() {
	int i, j;
	int pos;
	int k;
	
	static int final_status[BOARD_SIZE * BOARD_SIZE];

	for (pos = 0; pos < BOARD_SIZE * BOARD_SIZE; pos++) {
		if (board[pos] != EMPTY) {
			final_status[pos] = (countLiberties(stringNumber[pos]) > 1 ? ALIVE : DEAD);
		}
		else {
			final_status[pos] = UNKNOWN;
		}
	}

	for (i = 0; i < BOARD_SIZE; i++)
	for (j = 0; j < BOARD_SIZE; j++)
	if (get_board(i, j) == EMPTY)
	for (k = 0; k < 4; k++) {
		int ai = i + deltai[k];
		int aj = j + deltaj[k];
		if (!on_board(ai, aj))
			continue;

		/* Set the final status of the (i, j) vertex to either black
		* or white territory.
		*/
		if (final_status[getPos(i, j)] == UNKNOWN) {
			if ((final_status[getPos(ai, aj)] == ALIVE) ^ (get_board(ai, aj) == WHITE))
				final_status[getPos(i, j)] = BLACK_TERRITORY;
			else
				final_status[getPos(i, j)] = WHITE_TERRITORY;
		}
	}

	double res = KOMI;
	for (int pos = 0; pos < BOARD_SIZE * BOARD_SIZE; ++pos) {
		if (board[pos] == EMPTY) {
			if (final_status[pos] == BLACK_TERRITORY) {
				res += 1;
			}
			else {
				res -= 1;
			}
		}
		else {
			if ((final_status[pos] == ALIVE) ^ (board[pos] == WHITE)) {
				res += 1;
			}
			else {
				res -= 1;
			}
		}
	}

	/*for (int i = 0; i < BOARD_SIZE; ++i) {
		for (int j = 0; j < BOARD_SIZE; ++j)
			cout << board[i * 13 + j];
		cout << endl;
	}
	cout << endl;
	for (int i = 0; i < BOARD_SIZE; ++i) {
		for (int j = 0; j < BOARD_SIZE; ++j)
			cout << final_status[i * 13 + j];
		cout << endl;
	}
	cout << res << endl;
	*/
	return res > 0;
}