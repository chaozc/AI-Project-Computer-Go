#pragma once
#ifndef UCTSEARCH_H
#define UCTSEARCH_H

#include "GoBoard.h"

struct UctGameInfo {
	int move, visits;
	double wins;
	UctGameInfo* child, *sibling;

	inline void update(int val) {
		++visits;
		wins += val;
	}

	UctGameInfo(int move) {
		this->move = move;
		wins = visits = 0;
		child = sibling = NULL;
	}

	void destroy(); 
};

class UctSearch
{
public:
	UctSearch();
	UctSearch(const GoBoard& board, int color);
	~UctSearch();

	int selectBySearch(int timeLimit);
private:
	UctGameInfo* root;
	GoBoard initialBoard;
	int initialColor;


	void playOneSquence();
	UctGameInfo* decendByUCB1(UctGameInfo*, GoBoard&, int&);
	void creatChildren(UctGameInfo*, GoBoard&,const int&);
};

#endif // UCTSEARCH_H

