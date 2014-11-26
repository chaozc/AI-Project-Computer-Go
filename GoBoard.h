#pragma once

#ifndef GO_BOARD_H
#define GO_BOARD_H

#include <cstring>
#include <cstdlib>
#include <vector>
#include <list>
using namespace std;



class GoBoard
{
public:
	static const int BOARD_SIZE = 13;
	static const int EMPTY = 0;
	static const int WHITE = 1;
	static const int BLACK = 2;
	static const int DEAD = 0;
	static const int ALIVE = 1;
	static const int SEKI = 2;
	static const int WHITE_TERRITORY = 3;
	static const int BLACK_TERRITORY = 4;
	static const int UNKNOWN = 5;

	static const int INVALID = 0;
	static const int RANDOM = 1;
	static const int CAPTURE = 2;
	static const int PATTERN = 3;
	static const int ATARI_DEFENCE = 4;
	static const int ATARI = 5;

	static double KOMI;


	GoBoard();
	~GoBoard();

	GoBoard(int board[], int next_stone[], int ko_i, int ko_j, int li, int lj) {
		memcpy(this->board, board, sizeof(this->board));
		memset(libertyCount, 0, sizeof(libertyCount));
		memset(liberties, -1, sizeof(liberties));
		memcpy(this->next_stone, next_stone, sizeof(this->next_stone));
		this->ko_i = ko_i;
		this->ko_j = ko_j;
		this->li = li;
		this->lj = lj;
		libertiesCal();
	}

	int li;
	int lj;

	int get_board(int i, int j);
	int get_string(int i, int j, int *stonei, int *stonej);
	int legal_move(int i, int j, int color);
	void play_move(int i, int j, int color);
	void play_move(int move, int color);
	void generate_move(int *i, int *j, int color);
	vector<int> generate_all_moves(int color);
	int defaultPolicy(int color);

	void compute_final_status(void);
	int get_final_status(int i, int j);
	void set_final_status(int i, int j, int status);
	static int other_color(int color) {
		return WHITE + BLACK - color;
	}
	double simulateRandomly(int color);
	void show();


	int NumOfLiberties(int i, int j);
	void libertiesCal();
	double simulation(int color);
	int AtariCapture(int moves[], int num_moves, int color);
	int AtariDefense(int moves[], int num_moves, int color);
	int Capture(int moves[], int num_moves, int color);


	int blackWins();
	int lastMove();
	void generateAllMovesWithPriority(int color, int moves[]);

	int board[BOARD_SIZE * BOARD_SIZE];
	int liberties[BOARD_SIZE * BOARD_SIZE];

private:
	int next_stone[BOARD_SIZE * BOARD_SIZE];
	int final_status[BOARD_SIZE * BOARD_SIZE];
	int ko_i, ko_j;
	bool libertyCount[BOARD_SIZE * BOARD_SIZE];

	void clear_board();
	int board_empty();
	int on_board(int, int);
	int has_additional_liberty(int i, int j, int libi, int libj);
	int provides_liberty(int ai, int aj, int i, int j, int color);
	int suicide(int i, int j, int color);
	int remove_string(int i, int j);
	int same_string(int pos1, int pos2);
	void set_final_status_string(int pos, int status);
	void libertiesUpdate(int pos);

	int is_pattern_for_Hane(int i, int j, int color);
	int is_pattern_for_cut1(int i, int j, int color);
	int is_pattern_for_cut2(int i, int j, int color);
	int is_pattern_for_side(int i, int j, int color);
	int pattern_matching(int *i, int *j, int color);
	int pattern_matching(int len, int color, int moves[], int valid[]);
	int is_pattern(int i, int j, int color);
};


#endif
