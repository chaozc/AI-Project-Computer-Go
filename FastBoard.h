#pragma once

#ifndef FAST_BOARD_H
#define FASR_BOARD_H

class FastBoard
{
public:
	/* colors and other properties */
	static const int BOARD_SIZE = 13;
	static const int EMPTY = 0;
	static const int WHITE = 1;
	static const int BLACK = 2;
	static const int STRING_SIZE = BOARD_SIZE * BOARD_SIZE * 3;
	static const int PASS_MOVE = -1 * BOARD_SIZE - 1;
	static const double KOMI;

	static const int UNKNOWN = 0;
	static const int BLACK_TERRITORY = 1;
	static const int WHITE_TERRITORY = 2;
	static const int ALIVE = 3;
	static const int DEAD = 4;

	/* basic operations */
	bool onBoard(int pos, int k);
	int otherColor(int color) { return BLACK + WHITE - color; }
	bool isStone(int arg) { return arg == WHITE || arg == BLACK; }
	int pos(int i, int j) { return i * BOARD_SIZE + j; }
	int getI(int pos) { return pos / BOARD_SIZE; }
	int getJ(int pos) { return pos % BOARD_SIZE; }
	int south(int pos) { return pos + BOARD_SIZE; }
	int west(int pos) { return pos - 1; }
	int north(int pos) { return pos - BOARD_SIZE; }
	int east(int pos) { return pos + 1; }
	int SW(int pos) { return pos + BOARD_SIZE - 1; }
	int NW(int pos) { return pos - BOARD_SIZE - 1; }
	int NE(int pos) { return pos - BOARD_SIZE + 1; }
	int SE(int pos) { return pos + BOARD_SIZE + 1; }

	/* global variables */
	int board[BOARD_SIZE * BOARD_SIZE];
	int koPos, lastPos;

	/* Index into list of strings. The index is only valid if there is a
	* stone at the vertex.
	*/
	int stringNumber[BOARD_SIZE * BOARD_SIZE];

	/* The stones in a string are linked together in a cyclic list.
	* These are the coordinates to the next stone in the string.
	*/
	int nextStone[BOARD_SIZE * BOARD_SIZE];

	/* Incremental string data. */
	struct string_data {
		int color;                       /* Color of string, BLACK or WHITE */
		int size;                        /* Number of stones in string. */
		int origin; /* Coordinates of "origin", i.e. */
		int tail;
		/* "upper left" stone. */
		int liberties;      /* Number of liberties. */
	};

	string_data strings[STRING_SIZE];
	int stringCount;
	int sameString(int pos1, int pos2) { return stringNumber[pos1] == stringNumber[pos2]; }

	/* Count and/or find liberties at a string . */
	bool hasLiberty[STRING_SIZE][BOARD_SIZE * BOARD_SIZE];
	bool reduceLiberty(int str, int pos);
	bool provideLiberty(int str, int pos);
	int& countLiberties(int str) { return strings[str].liberties; }
	int& countStones(int str) { return strings[str].size; }
	const int& getColor(int str) { return strings[str].color; }

	/* basic instructions */
	int removeString(int str);
	void playMove(int pos, int color);
	int connectString(int str1, int str2);

	/* >< */
	FastBoard();
	FastBoard(int board[BOARD_SIZE * BOARD_SIZE], int koPos, int lastPos);
	~FastBoard();

	/*some more difficult ones */
	bool leagelMove(int pos);
	bool suicide(int pos, int color);
	bool rationalMove(int pos, int color);
	int generateMove(int color);

	/* Atari moves */
	int whiteInAtari[BOARD_SIZE * BOARD_SIZE], nw;
	int blackInAtari[BOARD_SIZE * BOARD_SIZE], nb;
	bool isInAtari[STRING_SIZE];

	/* capture move */
	
	int captureMove(int color);

	/* debug */
	void debug();
	void show();
	void simulate();

	int checkNakade(int pos, int color);
	bool inAtari(int pos);
	int findAtariLiberty(int str);
	int numberOfEmpty(int pos);

	int nakadeMove(int color);
	int atariDefenseMove(int color);
	int fillBoardMove(int color);
	int randomMove(int color);

	/* pattern */
	int get_board(int i, int j) { return board[i * BOARD_SIZE + j]; }
	int getPos(int i, int j) { return i * BOARD_SIZE + j; }
	int on_board(int i, int j) { return i >= 0 && j >= 0 && i < BOARD_SIZE && j < BOARD_SIZE; }

	int is_pattern_for_Hane(int i, int j, int color);
	int is_pattern_for_cut1(int i, int j, int color);
	int is_pattern_for_cut2(int i, int j, int color);
	int is_pattern_for_side(int i, int j, int color);
	int pattern_matching(int len, int color, int moves[]);
	int is_pattern(int i, int j, int color);
	int patternMove(int color);

	// complex evaluation
	bool blackWins();
	void evaluate(int color, double h[]);
};




#endif
