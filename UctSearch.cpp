#include "UctSearch.h"

#include <ctime>
#include <iostream>
#include <cmath>
void UctGameInfo::destroy() {
	for (UctGameInfo* next = child; next != NULL;) {
		next->destroy();
		UctGameInfo* tmp = next->sibling;
		delete next;
		next = tmp;
	}
}

UctSearch::UctSearch()
{
	
}

UctSearch::UctSearch(const GoBoard& board, int color)
{
	initialBoard = board;
	initialColor = color;
	root = new UctGameInfo(-1);
}


UctSearch::~UctSearch()
{
	root->destroy();
	delete root;
}

UctGameInfo* UctSearch::decendByUCB1(UctGameInfo* current, GoBoard& board, int& color) {
	UctGameInfo* res = NULL;
	double best = -1000000;
	for (UctGameInfo* next = current->child; next != NULL; next = next->sibling) {
		double v = 100000 + rand();
		if (next->visits > 0) {
			v = 1.0 * next->wins / next->visits + 5 * sqrt(log(current->visits) / next->visits);
		}
		if (v > best) {
			best = v;
			res = next;
		}
	}
	if (best >= 0) board.play_move(res->move, color);
	color = GoBoard::other_color(color);
	return res;
}

void UctSearch::creatChildren(UctGameInfo* current, GoBoard& board, const int& color) {
	static bool next[169];
//	board.generate_all_moves(color, next);
	UctGameInfo* from = current;
	for (int i = 0; i < 169; ++i){
		if (next[i]) {
			if (from == current) {
				from->child = new UctGameInfo(i);
				from = from->child;
			}
			else {
				from->sibling = new UctGameInfo(i);
				from = from->sibling;
			}
		}
	}
}

void UctSearch::playOneSquence() {
	GoBoard board = initialBoard;
	int color = initialColor;
	
	UctGameInfo* sequence[400];
	sequence[0] = root;
	int tail = 0;
	while (sequence[tail] != NULL && sequence[tail]->visits > 0 && tail < 300) {
		sequence[tail + 1] = decendByUCB1(sequence[tail], board, color);
		++tail;
	}
	if (tail >= 300) return;
	if (sequence[tail] == NULL) return;
	creatChildren(sequence[tail], board, color);

	int result = board.simulateRandomly(color)> 0 ? 1 : 0;
	if (result > 1) result = 1;
	if (result < 0) result = 0;

	for (int i = tail; i >= 0; --i) {
		sequence[i]->update(result);
		result = 1 - result;
	}
}

int UctSearch::selectBySearch(int timeLimit) {
	int st = clock();
	while (clock() - st < timeLimit * CLOCKS_PER_SEC) {
		playOneSquence();
	}
	srand((unsigned int)time(0));
	int res = -14;
	double maxV = 1000000000;
	int cnt = 0;
	for (UctGameInfo* next = root->child; next != NULL; next = next->sibling) {
		cnt += next->visits;
		//printf("%d\n", next->visits);
		double tmp = 1.0 * next->wins / next->visits;
		if (tmp < maxV) {
			maxV = tmp;
			res = next->move;
		}
	}

	if (maxV < 10) {
		std::cerr << maxV << std::endl;
		GoBoard test = initialBoard;
		test.play_move(res, initialColor);
		std::cerr << test.simulateRandomly(GoBoard::other_color(initialColor)) << std::endl;
		//std::cerr << initialBoard.simulateRandomly(res) << std::endl;
	}
	printf("%d %d %d\n", cnt, res, maxV);
	
	return res;
}