### Algorithms:

Based on Monte-Carlo algorithms, leverage UCT to implement the simulation. Each node in the tree represents a game-status, and records the winning probalility. simulate thousands of steps and pick up the step maximizing the winning probability

Start from the root node, pick up the child node maximizing the winning probalility to be the next step. When it comes to leaf node, randomly simulate thousands of steps to count the winning probability

### Usage:

Firstly setup GoGUI(http://gogui.sourceforge.net/) Then complie all the cpp files, import the executable files to GUI. Enjoy it.

updated by chaozc
