C++ Perft Debugger
An interactive command-line tool to find move generation bugs in your chess engine by comparing its perft results against Stockfish.

This tool streamlines the often tedious process of perft debugging by providing a powerful REPL-style interface to set custom positions, run detailed comparisons, and systematically drill down into move lines where discrepancies are found.

About The Project
Debugging a chess engine's move generator is one of the most challenging parts of its development. A single, subtle bug—like an incorrect castling flag update or a pawn promotion error—can be incredibly difficult to find. "Perft" (Performance Test) is the standard method for verifying a move generator's correctness. It works by counting the total number of legal moves to a certain depth and comparing that number to a known-correct result.

When a perft count is wrong at a high depth, it’s like trying to find a needle in a haystack of millions of positions. This tool automates and simplifies that exact workflow, turning a frustrating manual task into an efficient, guided process.

How It Works
Instead of manually comparing text outputs and walking the move tree, this tool acts as a semi-automatic debugger. It keeps track of your position in the game tree, invokes both your engine and Stockfish with the correct parameters, and then presents a color-coded "diff" of the results, instantly highlighting discrepancies so they are easy to spot.

Features
Interactive CLI: A simple and intuitive command-line prompt for a fast, iterative debugging cycle.

Direct Stockfish Comparison: Automatically uses Stockfish as a source of ground truth to validate your engine's output.

Drill-Down Debugging: Use move, unmove, and root commands to effortlessly navigate the move tree and isolate bugs.

Installation
1. Prerequisites
Before you begin, ensure you have the following installed on your system:

A C++17 compliant compiler (e.g., g++, clang++).

Stockfish: The stockfish executable must be in your system's PATH.

Downloading and Setting Up Stockfish
Download: Get the latest version from the Stockfish Website. Choose the appropriate version for your system (e.g., AVX2 for most modern computers).

Extract: Unzip the downloaded file to find the stockfish executable.

Add to System PATH: For the debugger to find Stockfish, its executable must be in a directory listed in your system's PATH.

For Linux & macOS: The standard location is /usr/local/bin.

# Move the executable to the standard location
sudo mv ~/Downloads/stockfish /usr/local/bin/stockfish

For Windows:

Create a stable folder, for example, C:\Tools.

Move stockfish.exe into that folder.

Search for "Edit the system environment variables" in the Start Menu, go to "Environment Variables...", select the Path variable, and click "Edit...".

Click "New" and add the path to your folder (e.g., C:\Tools).

Open a new terminal to verify it works by typing stockfish.

2. Compilation
Clone this repository or place perft_debugger.cpp and your engine's main.cpp in a new folder.

Open a terminal in the project directory.

Compile the debugger tool:

g++ perft_debugger.cpp -o perft_debugger -std=c++17 -O2

Compile your chess engine, ensuring its executable is named MyChessEngine.

g++ main.cpp [your other engine files like board.cpp] -o MyChessEngine -std=c++17 -O2

Usage
Your Engine's Executable (MyChessEngine)
The debugger requires your engine's executable to follow a specific command-line interface. The main.cpp template is already set up to do this.

Command-Line Invocation
The debugger will run your engine like this:

./MyChessEngine <depth> "<fen>" "[moves]"

<depth>: The depth parameter for the perft function.

<fen>: The FEN string of the "base" position.

[moves]: An optional space-separated string of moves in UCI notation from the base position.

Expected Output
Your engine must print its results to standard output in the following format:

For each legal move from the current position, print the move in UCI notation, a space, and the node count for that branch (i.e., the result of perft(depth - 1)).

After the list of moves, print a single blank line.

Finally, print the total perft result for the current position on its own line.

Example output for perft(3) from the starting position:

a2a3 380
b2b3 420
...
g1f3 440

8902

Running the Debugger
Both executables (perft_debugger and MyChessEngine) must be in the same directory.

Navigate your terminal into that directory.

Run the debugger:

./perft_debugger ./MyChessEngine

You will be greeted with a help menu and a > prompt.

Commands
Command

Example

Description

diff

diff

Run comparison at the current position and depth.

depth <N>

depth 5

Set the total perft depth to N.

fen <FEN>

fen rnbqkb1r/...

Set board position. This action clears all moves.

move <MOVE>

move e2e4

Make a move in UCI notation to go one level deeper.

unmove

unmove

Go back one move to explore other branches.

root

root

Reset to the initial FEN and clear all moves.

help

help

Show this help message again.

exit / quit

quit

Exit the debugger.

Example Workflow: Finding a Bug
Imagine your engine is failing perft(4) from the starting position.

Start the debugger:

./perft_debugger ./MyChessEngine

Set the depth:

> depth 4
Total depth set to 4.

Run the comparison:

> diff

The tool runs both engines. Imagine the output shows that the move d2d4 has an incorrect node count, highlighted in red. This tells you the bug lies somewhere in the tree of moves after d2d4 is played.

Drill down into the problematic move:

> move d2d4
Made move: d2d4

The debugger's internal board state is now updated. The effective depth for the next test is automatically 3.

Run the comparison again:

> diff
