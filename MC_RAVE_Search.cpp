#define _CRT_SECURE_NO_WARNINGS
#include "MC_RAVE_Search.h"
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>

const double MC_RAVE_Search::TIME_LIMIT = 9.6;

MC_RAVE_Search::MC_RAVE_Search()
{
}


MC_RAVE_Search::~MC_RAVE_Search()
{
}


int MC_RAVE_Search::MC_RAVE(FastBoard& board, int color) {
	int start_time = clock();

	s0 = new TreeNode(1, 0, board.lastPos, 0.5, 0, 0.5);
	int cnt = 0;
	while (clock() - start_time < TIME_LIMIT * CLOCKS_PER_SEC || cnt < 1000) {
		simulate(board, color);
		if (++cnt == 11) {
			int t = 0;
			//printf("%d\n", cnt);
		}
		//printf("%d\n", cnt);
	}
	//printf("%d\n", cnt);
	//ofstream out("my_log.txt", std::fstream::app);
	
	//out << "-----------------------------------" << endl;
	//out << cnt << endl;
	//board.show();
	Tree res = selectMove(board, s0, color, true);
	//out << "next" << endl;
	selectMove(board, res, board.otherColor(color), true);
	//printf("\n%d %f\n", res->last_move, res->eval());
	//board.show();
	//out << "Black winning rate: " << res->last_move << ' ' << res->eval(color) << endl;
	//out.flush();

	int ret = res->last_move;
	delete s0;
	return ret; // some adjustment is needed here 

}

void MC_RAVE_Search::simulate(FastBoard board, int color) {
	if (rand() % 3 != 0) {
		c1 = 0.0003;
	} else {
		c1 = 0.006;
	}

	vector<Tree> s;
	vector<int> a;
	s.push_back(s0);
	a.push_back(-1);
	//printf("what?");
	simulateTree(board, color, s, a);
	//printf("pass tree\n");
	int z = simulateDefault(board, color, a);
	//printf("pass default %d\n", z);
	backup(s, a, z);
	//printf("pass back up\n");
}

int MC_RAVE_Search::simulateDefault(FastBoard& board, int& color, vector<int>& a) {
	int limit = FastBoard::STRING_SIZE - board.stringCount - 10;
	while ((int)a.size() < limit) {
		int move = board.generateMove(color);
		//printf("finished\n");
	//	printf("%d %d\n", a.size(), move);
		if (move < 0) {
			break;
		}
		a.push_back(move);
		//printf("aa");
		board.playMove(move, color);
		//printf("bb");
		color = board.otherColor(color);
	}
	return board.blackWins();
}

int MC_RAVE_Search::simulateTree(FastBoard& board, int& color, vector<Tree>& s, vector<int>& a) {
	Tree st = s0;
	while (true) {
		if (!st->inTree) {
			newNode(board, st, color);
			return 1;
		}

		Tree at = selectMove(board, st, color, false);
		if (at == NULL) return board.blackWins();
		if (s.size() == 0) {
			//cout << at->last_move << ' ' << at->N << ' ' << at->Q << ' ' << at->eval() << endl;
			//system("pause");
		}
		int move = at->last_move;
		s.push_back(at);
		a.push_back(move);
		
		board.playMove(move, color);
		st = at;
		color = board.otherColor(color);

		//board.show();
		
	}
}

Tree MC_RAVE_Search::selectMove(FastBoard& board, Tree s, int color, bool show) {

	if (show) {
		Tree best = NULL;
		for (Tree next = s->child; next != NULL; next = next->sibling) {
			//ofstream out("my_log.txt", fstream::app);
			//out << "d " << next->last_move << ' ' << next->N << ' ' << next->N_AMAF << ' ' << next->Q << ' ' << next->Q_AMAF << ' ' << next->H << ' ' << next->eval(color) << endl;
	
			if (best == NULL || next->rootEval(color) > best->rootEval(color)) {
				best = next;
			}
		}
		return best;
	}
	Tree best = NULL;
	double value = 0;
	for (Tree next = s->child; next != NULL; next = next->sibling) {
		if (best == NULL) {
			best = next;
			value = best->eval(color);
		}

		if (color == FastBoard::BLACK) {
			double tmp = next->eval(color);
			if (tmp > value) {
				value = tmp;
				best = next;
			}
		}
		else {
			double tmp = next->eval(color);
			if (tmp > value) {
				value = tmp;
				best = next;
			}
		}
	}
	return best;
}


void MC_RAVE_Search::backup(vector<Tree>& s, vector<int>& a, int z) {
	static bool visited[FastBoard::BOARD_SIZE * FastBoard::BOARD_SIZE];
	memset(visited, 0, sizeof(visited));

	for (int t = 0; t < (int)s.size(); ++t) {
		
		if (t != 0) {
			++s[t]->N;
			s[t]->Q += (z - s[t]->Q) / (s[t]->N);
		}

		for (int u = t + 1; u < (int)a.size(); u += 2) {
			visited[a[u]] = true;
		}

		//cout << endl;
		//system("pause");
		for (Tree next = s[t]->child; next != NULL; next = next->sibling) {
			if (visited[next->last_move]) {
				++next->N_AMAF;
				next->Q_AMAF += (z - next->Q_AMAF) / next->N_AMAF;
				//cout << next->Q_AMAF << ' ';
			}
		}
		//cout << endl;
		for (int u = t + 1; u < (int)a.size(); u += 2) {
			//cout << a[u] << endl;
			visited[a[u]] = false;
		}
		//system("pause");
	}
}
/*
//  Old version
void MC_RAVE_Search::newNode(GoBoard& board, Tree s, int color) {
	vector<int> legalMoves = board.generate_all_moves(color);
	s->inTree = true;
	Tree prev = s;
	for (int i = 0; i < (int)legalMoves.size(); ++i) {
		Tree next = new TreeNode(1, 0, legalMoves[i], color == GoBoard::BLACK ? 0.9 : 0.1 , 0);
		
		if (prev == s) prev->child = next;
		else prev->sibling = next;
		prev = next;
	}
}
*/

void MC_RAVE_Search::newNode(FastBoard& board, Tree s, int color) {
	static double moves[FastBoard::BOARD_SIZE * FastBoard::BOARD_SIZE];
	board.evaluate(color, moves);

	s->inTree = true;
	Tree prev = s;
	for (int i = 0; i < FastBoard::BOARD_SIZE * FastBoard::BOARD_SIZE; ++i) {
		if (moves[i] <= 0) continue;

		Tree next = new TreeNode(0, 0, i, 0, 0, color == FastBoard::BLACK ? moves[i] : 1 - moves[i]);
		if (prev == s) prev->child = next;
		else prev->sibling = next;
		prev = next;
	}
	//system("pause");
}