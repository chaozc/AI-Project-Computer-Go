#include "UCT_Simulator.h"
#include <cmath>

UCT_Simulator::UCT_Simulator()
{
}


UCT_Simulator::~UCT_Simulator()
{
}

Node Node::bufferNode[Node::BUFFER_SIZE];
int Node::tail;

void Node::forkChildren(int color) {
	static bool next[GoBoard::BOARD_SIZE * GoBoard::BOARD_SIZE];
	//board.show();
	//system("pause");
	//board.generate_all_moves(color, next);
	//std::cout << "W1" << std::endl;
	for (int i = 0; i < 169; ++i) {
		if (next[i]) {
			GoBoard child_board = board;
			//if (i == 28) std::cout << i << ' ' << I(i) << ' ' << J(i) << ' ' << color << std::endl;
			//if (i == 28) child_board.show();
			child_board.play_move(I(i), J(i), color);
			
			//if (i == 28) std::cout << "M" << std::endl;
			children.push_back(newNode(child_board, I(i), J(i)));
			//if (i == 28) std::cout << "T" << std::endl;
		}
	}
}