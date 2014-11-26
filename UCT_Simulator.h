#pragma once
#ifndef UCT_SIMULATOR_H
#define UCT_SIMULATOR_H

#include <vector>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include "GoBoard.h"
#include <iostream>
#include <cmath>

const double INFINITY_DOUBLE = 1e20;
const double TIME_LIMIT = 2;
#define POS(i, j) ((i) * GoBoard::BOARD_SIZE + (j))
#define I(pos) ((pos) / GoBoard::BOARD_SIZE)
#define J(pos) ((pos) % GoBoard::BOARD_SIZE)

typedef struct Node* Tree;
struct Node {
	static const int BUFFER_SIZE = 100;
	static Node bufferNode[BUFFER_SIZE];
	static int tail;

	bool visited;
	double value;
	int nb, move_i, move_j;
	GoBoard board;
	std::vector<Tree> children;

	static Tree newNode(const GoBoard& board, int move_i, int move_j){
		Tree  tree = bufferNode + tail++;
		tree->visited = false;
		tree->value = 0;
		tree->nb = 0;
		tree->board = board;
		tree->move_i = move_i;
		tree->move_j = move_j;
		return tree;
	}



	double getValueByMC(int color) {
		static GoBoard test;
		test = board;
		double res = test.simulateRandomly(color);
		if (res > 0) return 1+res/1e3;
		else return res/1e3;
	}

	Tree descendByUCB1() {
		double maxV = -INFINITY_DOUBLE;
		Tree res = NULL;
		for (int i = 0; i < (int)children.size(); ++i) {
			double v = INFINITY_DOUBLE;
			if (children[i]->nb > 0) {
				v = -children[i]->value / children[i]->nb + sqrt(2 * log(nb) / children[i]->nb);
			}
			if (v > maxV) {
				maxV = v;
				res = children[i];
			}
		}
		return res;
	}

	void forkChildren(int color);

	void selectMove(int* i, int* j) {
		double maxNB = -INFINITY_DOUBLE;
		for (int k = 0; k < (int)children.size(); ++k) {
			if (children[k]->value / children[k]->nb > maxNB) {
				maxNB = children[k]->value / children[k]->nb;
				*i = children[k]->move_i;
				*j = children[k]->move_j;
			}
		}
		std::cout << maxNB << std::endl;
	}
};


class UCT_Simulator
{
public:
	UCT_Simulator();
	~UCT_Simulator();
	void initialize(GoBoard initialBoard, int color) {
		Node::tail = 0;
		root = Node::newNode(initialBoard, -2, -2);
		this->initialColor = color;
	}

	void getResult(int* i, int* j) {
		double st_clock = clock();
		int cnt = 0;
		while (clock() - st_clock < TIME_LIMIT * CLOCKS_PER_SEC && Node::tail + 169 < Node::BUFFER_SIZE) {
			//std::cout << "step 1" << std::endl;
			playOneSequence();
			
			
		}
		root->selectMove(i, j);
	}
private:
	Tree root;
	int initialColor;

	void playOneSequence() {
		static Tree sequence[400];
		sequence[0] = root;
		double st = clock();

		int i = 0;
		int color = initialColor;
		int cnt = 0;
		while (sequence[i] != NULL && sequence[i]->visited && ++cnt <200) {
			sequence[i + 1] = sequence[i]->descendByUCB1();
			++i;
			color = GoBoard::other_color(color);
		}
		//std::cout << "step 2" << std::endl;
		
		double value = sequence[i]->getValueByMC(color);
		//std::cout << "step 3" << std::endl;
		sequence[i]->forkChildren(color);
		//sequence[i]->board.show();
		
		//system("pause");
		//std::cout << "step 4" << std::endl;
		sequence[i]->visited = true;
		for (int j = i; j >= 0; --j, value = -value) {
			sequence[j]->value += value;
			++sequence[j]->nb;
		}
	}

	
};

#endif
