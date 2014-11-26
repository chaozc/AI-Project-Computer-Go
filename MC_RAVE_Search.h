#pragma once
#ifndef MC_RAVE_SEARCH_H
#define MC_RAVE_SEARCH_H

#include "FastBoard.h"
#include <cstdlib>
#include <vector>
using namespace std;

static double c1 = 0.001;
static double c2 = 1.8;
static double C = 0.1;


/* fast log2 */
typedef unsigned long uint32;
typedef long int32;

static inline int32 ilog2(uint32 x) {
	return ilog2((float)x);
}

// integer log2 of a float
static inline int32 ilog2(float x)
{
	uint32 ix = (uint32&)x;
	uint32 exp = (ix >> 23) & 0xFF;
	int32 log2 = int32(exp) - 127;

	return log2;
}


typedef struct TreeNode* Tree;
struct TreeNode {
	bool inTree;
	int N, N_AMAF, last_move;
	double Q, Q_AMAF, H;
	Tree child, sibling;

	TreeNode(int N, int N_AMAF, int last_move,  double Q, double Q_AMAF, double H) {
		this->N = N;
		this->N_AMAF = N_AMAF;
		this->last_move = last_move;
		this->Q = Q;
		this->Q_AMAF = Q_AMAF;
		this->H = H;

		child = sibling = NULL;
		inTree = false;
	}

	double eval(int color) { 
		if (N == 0 || N_AMAF == 0) return (color == FastBoard::BLACK ? H : 1 - H) + 1000 + 1e-12 * (rand() & 65535);
		double beta = N_AMAF / (N_AMAF + N + c1 * N * N_AMAF);
		double gama = c2 / (N + N_AMAF);
		double alpha = 1 - beta - gama;
		if (alpha < 0) alpha = 0;
		
		return color == FastBoard::BLACK ? alpha * Q + beta * Q_AMAF + (gama + C / (ilog2(float(2 + N)))) * H 
			: alpha * (1 - Q) + beta * (1 - Q_AMAF) + (gama + C / (ilog2(float(2 + N)))) * (1 - H);
	}

	double rootEval(int color) {
		c1 = 0.015;
		if (N < 500) return -1;
		double beta = N_AMAF / (N_AMAF + N + c1 * N * N_AMAF);
		double gama = c2 / (N + N_AMAF);
		double alpha = 1 - beta - gama;
		if (alpha < 0) alpha = 0;

		return color == FastBoard::BLACK ? alpha * Q + beta * Q_AMAF + gama + C * H
			: alpha * (1 - Q) + beta * (1 - Q_AMAF) +  C  * (1 - H);
	}

	~TreeNode() {
		for (Tree next = child; next != NULL; ) {
			Tree tmp = next->sibling;
			delete next;
			next = tmp;
		}
	}
};

class MC_RAVE_Search
{
public:
	static const double TIME_LIMIT;

	MC_RAVE_Search();
	~MC_RAVE_Search();

	int MC_RAVE(FastBoard& board, int color);

private:
	Tree s0;
	int initialCplor;
	

	void simulate(FastBoard board, int color);
	int simulateDefault(FastBoard& board, int& color, vector<int>& a);
	int simulateTree(FastBoard& board, int& color, vector<Tree>& s, vector<int>& a);
	Tree selectMove(FastBoard& board, Tree s, int color, bool show);
	void backup(vector<Tree>& s, vector<int>& a, int z);
	void newNode(FastBoard& board, Tree s, int color);
}; 

#endif // MC_RAVE_SEARCH_H