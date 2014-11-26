#pragma once

#include "FastBoard.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

static int deltai[4] = { -1, 1, 0, 0 };
static int deltaj[4] = { 0, 0, -1, 1 };
static int deltai22[4] = { 0, 0, 1, 1 };
static int deltaj22[4] = { 0, 1, 0, 1 };
static int deltai22ET1[4] = { 0, 1, -1, 0 };
static int deltaj22ET1[4] = { 1, 0, 0, -1 };

class ComplexEvaluator
{
public:

	ComplexEvaluator();

	ComplexEvaluator(FastBoard* fasrboard) {
		board = fasrboard;
	}
	~ComplexEvaluator();

	FastBoard* board;

	static const int BLACK = FastBoard::BLACK;
	static const int WHITE = FastBoard::WHITE;
	static const int MAX_BOARD = FastBoard::BOARD_SIZE;
	
	int get_board(int i, int j) {
		return board->board[i * MAX_BOARD + j];
	}

	bool on_board(int i, int j) {
		return i >= 0 && i < MAX_BOARD && j >= 0 && j < MAX_BOARD;
	}

	int POS(int i, int j) {
		return i * MAX_BOARD + j;
	}
	

	static const int POS_HANE = 0;
	static const int POS_WALL = 1;
	static const int POS_EMPTY_TRIANGLE1 = 2;
	static const int POS_EMPTY_TRIANGLE2 = 3;
	static const int POS_KOSUMI = 4;
	static const int POS_KATA = 5;
	static const int POS_CONNECT = 6;
	static const int POS_PEEP_CONNECT = 7;
	static const int POS_BAD_KOGEIMA = 8;
	static const int POS_KOGEIMA = 9;
	static const int POS_BAD_TOBI = 10;

	int evaluation_Hane(int area, int neg_area, int color){
		int pattern[8] = { 9, 33, 6, 132, 144, 18, 96, 72 };
		int cross_point[8] = { 1, 2, 0, 3, 3, 0, 2, 1 };
		for (int i = 0; i < 8; ++i){
			if (((color == BLACK) && (area == pattern[i])) ||
				((color == WHITE) && (neg_area == pattern[i]))){
				return cross_point[i];
			}
		}
		return -1;
	}
	int evaluation_Wall(int area, int neg_area, int color){
		int pattern[8] = { 24, 36, 66, 129, 129, 66, 36, 24 };
		int cross_point[8] = { 0, 0, 1, 1, 1, 1, 0, 0 };
		for (int i = 0; i < 8; ++i){
			if (((color == BLACK) && (area == pattern[i])) ||
				((color == WHITE) && (neg_area == pattern[i]))){
				return cross_point[i];
			}
		}
		return -1;
	}
	int evaluation_Empty_Triangle1(int area, int neg_area, int color){
		int pattern[8] = { 160, 136, 160, 34, 10, 136, 10, 34 };
		int cross_point[8] = { 3, 1, 3, 2, 0, 1, 0, 2 };
		for (int i = 0; i < 8; ++i){
			if (((color == BLACK) && (area == pattern[i])) ||
				((color == WHITE) && (neg_area == pattern[i]))){
				return cross_point[i];
			}
		}
		return -1;
	}
	int evaluation_Empty_Triangle2(int area, int neg_area, int color){
		int pattern[8] = { 130, 130, 40, 40, 40, 40, 130, 130 };
		int cross_point[8] = { 1, 1, 0, 0, 0, 0, 1, 1 };
		for (int i = 0; i < 8; ++i){
			if (((color == BLACK) && (area == pattern[i])) ||
				((color == WHITE) && (neg_area == pattern[i]))){
				return cross_point[i];
			}
		}
		return -1;
	}
	int evaluation_Kosumi(int area, int neg_area, int color){
		int pattern[8] = { 32, 8, 128, 2, 2, 128, 8, 32 };
		int cross_point[8] = { 2, 1, 3, 0, 0, 3, 1, 2 };
		for (int i = 0; i < 8; ++i){
			if (((color == BLACK) && (area == pattern[i])) ||
				((color == WHITE) && (neg_area == pattern[i]))){
				return cross_point[i];
			}
		}
		return -1;
	}
	int evaluation_Kata(int area, int neg_area, int color){
		int pattern[8] = { 16, 4, 64, 1, 1, 64, 4, 16 };
		int cross_point[8] = { 2, 1, 3, 0, 0, 3, 1, 2 };
		for (int i = 0; i < 8; ++i){
			if (((color == BLACK) && (area == pattern[i])) ||
				((color == WHITE) && (neg_area == pattern[i]))){
				return cross_point[i];
			}
		}
		return -1;
	}
	int evaluation_Connect23(int area, int neg_area, int color){
		int pattern[4] = { 1665, 2340, 390, 1065 };
		int cross_point[4] = { 1, 0, 1, 0 };
		for (int i = 0; i < 4; ++i){
			if (((color == BLACK) && (area == pattern[i])) ||
				((color == WHITE) && (neg_area == pattern[i]))){
				return cross_point[i];
			}
		}
		return -1;
	}
	int evaluation_Peep_Connect23(int area, int neg_area, int color){
		int pattern[4] = { 578, 2072, 578, 2072 };
		int cross_point[4] = { 1, 0, 1, 0 };
		for (int i = 0; i < 4; ++i){
			if (((color == BLACK) && (area == pattern[i])) ||
				((color == WHITE) && (neg_area == pattern[i]))){
				return cross_point[i];
			}
		}
		return -1;
	}
	int evaluation_Bad_Kogeima23(int area, int neg_area, int color){
		int pattern[4] = { 513, 2052, 258, 1032 };
		int cross_point[4] = { 4, 5, 0, 1 };
		for (int i = 0; i < 4; ++i){
			if (((color == BLACK) && (area == pattern[i])) ||
				((color == WHITE) && (neg_area == pattern[i]))){
				return cross_point[i];
			}
		}
		return -1;
	}
	int evaluation_Kogeima23(int area, int neg_area, int color){
		int pattern[4] = { 512, 2048, 2, 8 };
		int cross_point[4] = { 4, 5, 0, 1 };
		for (int i = 0; i < 4; ++i){
			if (((color == BLACK) && (area == pattern[i])) ||
				((color == WHITE) && (neg_area == pattern[i]))){
				return cross_point[i];
			}
		}
		return -1;
	}
	int evaluation_Bad_Tobi23(int area, int neg_area, int color){
		int pattern[4] = { 2064, 576, 24, 66 };
		int cross_point[4] = { 4, 5, 0, 1 };
		for (int i = 0; i < 4; ++i){
			if (((color == BLACK) && (area == pattern[i])) ||
				((color == WHITE) && (neg_area == pattern[i]))){
				return cross_point[i];
			}
		}
		return -1;
	}
	int evaluation_Peep_Connect32(int area, int neg_area, int color){
		int pattern[4] = { 2180, 2180, 290, 290 };
		int cross_point[4] = { 1, 1, 4, 4 };
		for (int i = 0; i < 4; ++i){
			if (((color == BLACK) && (area == pattern[i])) ||
				((color == WHITE) && (neg_area == pattern[i]))){
				return cross_point[i];
			}
		}
		return -1;
	}
	int evaluation_Connect32(int area, int neg_area, int color){
		int pattern[4] = { 594, 1569, 1161, 2136 };
		int cross_point[4] = { 4, 4, 1, 1 };
		for (int i = 0; i < 4; ++i){
			if (((color == BLACK) && (area == pattern[i])) ||
				((color == WHITE) && (neg_area == pattern[i]))){
				return cross_point[i];
			}
		}
		return -1;
	}
	int evaluation_Bad_Kogeima32(int area, int neg_area, int color){
		int pattern[4] = { 2112, 1152, 33, 18 };
		int cross_point[4] = { 5, 3, 2, 0 };
		for (int i = 0; i < 4; ++i){
			if (((color == BLACK) && (area == pattern[i])) ||
				((color == WHITE) && (neg_area == pattern[i]))){
				return cross_point[i];
			}
		}
		return -1;
	}
	int evaluation_Kogeima32(int area, int neg_area, int color){
		int pattern[4] = { 2048, 128, 32, 2 };
		int cross_point[4] = { 5, 3, 2, 0 };
		for (int i = 0; i < 4; ++i){
			if (((color == BLACK) && (area == pattern[i])) ||
				((color == WHITE) && (neg_area == pattern[i]))){
				return cross_point[i];
			}
		}
		return -1;
	}
	int evaluation_Bad_Tobi32(int area, int neg_area, int color){
		int pattern[4] = { 288, 258, 2052, 132 };
		int cross_point[4] = { 5, 3, 2, 0 };
		for (int i = 0; i < 4; ++i){
			if (((color == BLACK) && (area == pattern[i])) ||
				((color == WHITE) && (neg_area == pattern[i]))){
				return cross_point[i];
			}
		}
		return -1;
	}
	void board_evaluation(int evaluation[], int color){
		int area22, area23, area32, neg_area22, neg_area23, neg_area32;
		int ai, aj;
		int base, value;
		int result;

		memset(evaluation, 0, sizeof(evaluation));
		//out << "====================" << endl;
		for (int i = 0; i < MAX_BOARD; i++){
			for (int j = 0; j < MAX_BOARD; j++){
				if (on_board(i + 1, j + 1)){
					base = 1 << 6;
					area22 = 0;
					neg_area22 = 0;
					for (int k = 0; k < 4; k++){
						ai = i + deltai22[k];
						aj = j + deltaj22[k];
						value = get_board(ai, aj);
						area22 += base * value;
						if (value > 0){
							neg_area22 += base * (3 - value);
						}
						base /= 4;
					}
					result = evaluation_Hane(area22, neg_area22, color);
					if (result >= 0){
						evaluation[POS(i + deltai22[result], j + deltaj22[result])] |= (1 << POS_HANE);
						//out << i + deltai22[result] <<"   "<<j + deltaj22[result]<<endl;
					}
					result = evaluation_Wall(area22, neg_area22, color);
					if (result >= 0){
						evaluation[POS(i + deltai22[result], j + deltaj22[result])] |= (1 << POS_WALL);
						evaluation[POS(i + deltai22[3 - result], j + deltaj22[3 - result])] |= (1 << POS_WALL);
						//out << i + deltai22[result] <<"   "<<j + deltaj22[result]<<endl;
						//out << i + deltai22[3 - result] <<"   "<<j + deltaj22[3 - result]<<endl;
					}
					result = evaluation_Empty_Triangle1(area22, neg_area22, color);
					if (result >= 0){
						evaluation[POS(i + deltai22[result], j + deltaj22[result])] |= (1 << POS_EMPTY_TRIANGLE1);
						evaluation[POS(i + deltai22[result] + deltai22ET1[result], j + deltaj22[result] + deltaj22ET1[result])] |= (1 << POS_EMPTY_TRIANGLE1);
						//out << i + deltai22[result] <<"   "<<j + deltaj22[result]<<endl;
						//out << i + deltai22[result] + deltai22ET1[result] <<"   "<<j + deltaj22[result] + deltaj22ET1[result]<<endl;
					}
					result = evaluation_Empty_Triangle2(area22, neg_area22, color);
					if (result >= 0){
						evaluation[POS(i + deltai22[result], j + deltaj22[result])] |= (1 << POS_EMPTY_TRIANGLE2);
						evaluation[POS(i + deltai22[3 - result], j + deltaj22[3 - result])] |= (1 << POS_EMPTY_TRIANGLE2);
						//out << i + deltai22[result] <<"   "<<j + deltaj22[result]<<endl;
						//out << i + deltai22[3 - result] <<"   "<<j + deltaj22[3 - result]<<endl;
					}
					result = evaluation_Kosumi(area22, neg_area22, color);
					if (result >= 0){
						evaluation[POS(i + deltai22[result], j + deltaj22[result])] |= (1 << POS_KOSUMI);
						//out << i + deltai22[result] <<"   "<<j + deltaj22[result]<<endl;
					}
					result = evaluation_Kata(area22, neg_area22, color);
					if (result >= 0){
						evaluation[POS(i + deltai22[result], j + deltaj22[result])] |= (1 << POS_KATA);
						//out << i + deltai22[result] <<"   "<<j + deltaj22[result]<<endl;
					}
					if (on_board(i + 2, j + 1)){
						area23 = area22 << 4;
						neg_area23 = neg_area22 << 4;
						base = 4;
						for (int k = 0; k < 2; ++k){
							value = get_board(i + 2, j + k);
							area23 += base * value;
							if (value > 0){
								neg_area23 += base * (3 - value);
							}
							base /= 4;
						}
						result = evaluation_Connect23(area23, neg_area23, color);
						if (result >= 0){
							evaluation[POS(i + 1, j + result)] |= (1 << POS_CONNECT);
							//out << i + 1 <<"   "<<j + result<<endl;
						}
						result = evaluation_Peep_Connect23(area23, neg_area23, color);
						if (result >= 0){
							evaluation[POS(i + 1, j + result)] |= (1 << POS_PEEP_CONNECT);
							//out << i + 1 <<"   "<<j + result<<endl;
						}
						result = evaluation_Bad_Kogeima23(area23, neg_area23, color);
						if (result >= 0){
							evaluation[POS(i + result / 2, j + result % 2)] |= (1 << POS_BAD_KOGEIMA);
							//out << i + result / 2 <<"   "<<j + result % 2<<endl;
						}
						result = evaluation_Kogeima23(area23, neg_area23, color);
						if (result >= 0){
							evaluation[POS(i + result / 2, j + result % 2)] |= (1 << POS_KOGEIMA);
							//out << i + result / 2 <<"   "<<j + result % 2<<endl;
						}
						result = evaluation_Bad_Tobi23(area23, neg_area23, color);
						if (result >= 0){
							evaluation[POS(i + result / 2, j + result % 2)] |= (1 << POS_BAD_TOBI);
							//out << i + result / 2 <<"   "<<j + result % 2<<endl;
						}
					}
					if (on_board(i + 1, j + 2)){
						area32 = ((area22 / 16) << 8) + ((area22 % 16) << 2);
						neg_area32 = ((neg_area22 / 16) << 8) + ((neg_area22 % 16) << 2);
						base = 1 << 6;
						for (int k = 0; k < 2; ++k){
							value = get_board(i + k, j + 2);
							area32 += base * value;
							if (value > 0){
								neg_area32 += base * (3 - value);
							}
							base /= 64;
						}
						result = evaluation_Peep_Connect32(area32, neg_area32, color);
						if (result >= 0){
							evaluation[POS(i + result / 3, j + result % 3)] |= (1 << POS_PEEP_CONNECT);
							//out << i + result / 3 <<"   "<<j + result % 3<<endl;
						}
						result = evaluation_Connect32(area32, neg_area32, color);
						if (result >= 0){
							evaluation[POS(i + result / 3, j + result % 3)] |= (1 << POS_CONNECT);
							//out << i + result / 3 <<"   "<<j + result % 3<<endl;
						}
						result = evaluation_Bad_Kogeima32(area32, neg_area32, color);
						if (result >= 0){
							evaluation[POS(i + result / 3, j + result % 3)] |= (1 << POS_BAD_KOGEIMA);
							//out << i + result / 3 <<"   "<<j + result % 3<<endl;
						}
						result = evaluation_Kogeima32(area32, neg_area32, color);
						if (result >= 0){
							evaluation[POS(i + result / 3, j + result % 3)] |= (1 << POS_KOGEIMA);
							//out << i + result / 3 <<"   "<<j + result % 3<<endl;
						}
						result = evaluation_Bad_Tobi32(area32, neg_area32, color);
						if (result >= 0){
							evaluation[POS(i + result / 3, j + result % 3)] |= (1 << POS_BAD_TOBI);
							//out << i + result / 3 << "   " << j + result % 3 << endl;
						}
					}
				}
			}
		}
	}

	
};

