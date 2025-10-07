C++ Perft Debugger
An interactive command-line tool designed to find move generation bugs in your chess engine by comparing its performance test (perft) results against the proven accuracy of Stockfish.

This tool streamlines the often tedious and frustrating process of perft debugging by providing a powerful REPL-style interface to set custom positions, run detailed comparisons, and systematically drill down into move lines where discrepancies are found.

About The Project
Debugging a chess engine's move generator is one of the most challenging parts of its development. A single, subtle bug—like an incorrect castling flag update or a pawn promotion error—can be incredibly difficult to find. "Perft" (Performance Test) is the standard method for verifying a move generator's correctness. It works by counting the total number of legal moves to a certain depth and comparing that number to a known-correct result.

When a perft count is wrong at a high depth (e.g., depth 5), it’s like trying to find a needle in a haystack of millions of positions. The only way to find the bug is to trace it down, level by level, through each branch of the move tree. This tool automates and simplifies that exact workflow, turning a frustrating manual task into an efficient, guided process.

Features
Interactive CLI: A simple and intuitive command-line prompt that allows for a fast, iterative debugging cycle without needing to recompile code.

Direct Stockfish Comparison: Automatically runs the same perft test with the Stockfish engine, using it as a source of ground truth to instantly validate your engine's output.

Drill-Down Debugging: The core of the tool. Use move, unmove, and root commands to effortlessly navigate through the game's move tree, allowing you to chase a bug down from a high depth to the specific move that causes the error.

Prerequisites
Before you begin, ensure you have the following installed on your system:

A C++17 compliant compiler (e.g., g++, clang++).

Stockfish: The stockfish executable must be in your system's PATH.

Downloading and Installing Stockfish
Download: Get the latest version from the Stockfish Website. Choose the appropriate version for your operating system (e.g., AVX2 for most modern computers).

Extract: Unzip the downloaded file. Inside, you will find the stockfish executable.

Add to System PATH: For the debugger to find and launch Stockfish automatically, its executable must be in a directory listed in your system's PATH environment variable.

For Linux & macOS: The standard place for user-installed command-line tools is /usr/local/bin. Open a terminal and move the executable there. This directory is almost always included in the system PATH by default.

# Example: assuming the executable is in your Downloads folder
sudo mv ~/Downloads/stockfish /usr/local/bin/stockfish

You can verify it's working by opening a new terminal window and typing stockfish.

For Windows:

Create a stable folder for your command-line tools, for example, C:\Tools. Avoid using temporary folders like Downloads.

Move the stockfish.exe file into C:\Tools.

Search for "Edit the system environment variables" in the Start Menu and open it.

Click the "Environment Variables..." button.

In the "System variables" section, find and select the Path variable, then click "Edit...".

Click "New" and add the path to your folder (e.g., C:\Tools).

Click OK on all windows to save the changes.

Open a new Command Prompt or PowerShell window and type stockfish to verify it works.

Installation & Compilation
Clone the repository or place both perft_debugger.cpp and your engine's main.cpp in a new folder.

Open a terminal in the project directory.

Compile the debugger tool:

g++ perft_debugger.cpp -o perft_debugger -std=c++17 -O2

Compile your chess engine. Make sure its executable is named MyChessEngine.

# If your engine is just main.cpp
g++ main.cpp -o MyChessEngine -std=c++17 -O2

# If you have other files (e.g., board.cpp, movegen.cpp)
g++ main.cpp position.cpp chess.cpp -o MyChessEngine -std=c++17 -O2

Setting Up Your Engine (main.cpp)
The provided main.cpp file is a template. It is designed to handle all the command-line argument parsing and communication with the debugger. Your job is to integrate your own engine's logic into it.

Include Your Headers: At the top of main.cpp, remove the placeholder structs (Position, Move, Moves) and include the headers for your own engine's classes.

Integrate Your Logic: Replace the placeholder logic inside the template's functions with calls to your own engine's functions. The key areas to modify are clearly marked with comments. Specifically:

In main():

Replace pos.parseFEN(fen); with your FEN parser.

Replace pos.generateLegalMoves<...>(); with your move generator.

Replace pos.makemove<...>(move); with your move-making function.

In perft() and runPerftAndPrint():

Replace the placeholder logic with your actual recursive perft implementation.

Preserve the Output Format: The most important rule is that your engine must print its results to the console in the exact format the debugger expects. The template already does this, so as long as you replace the logic without changing the std::cout structure, it will work correctly. The required format is:

A list of moves, one per line: <move_uci> <node_count> (e.g., e2e4 13160).

An empty line.

The total node count on a final line.

How to Run
For the debugger to work, both executables (perft_debugger and MyChessEngine) must be in the same directory, and you must run the command from within that directory.

Navigate your terminal into the directory containing the executables using the cd command.

Run the debugger and provide the relative path to your engine's executable. This command launches the debugger, which will, in turn, launch your engine as a child process when needed.

./perft_debugger ./MyChessEngine

You will be greeted with a help menu and a > prompt, ready to accept commands.

Debugger Commands
Command

Example

Description

diff

diff

Run comparison at the current position and depth.

depth <N>

depth 5

Set the total perft depth to N. The effective depth decreases as you make moves.

fen <FEN>

fen rnbqkb1r/...

Set board position using a FEN string. This action clears all moves.

move <MOVE>

move e2e4

Make a move in UCI notation to go one level deeper into the move tree.

unmove

unmove

Go back one move, allowing you to explore other branches.

root

root

Reset to the initial FEN and clear all moves made.

help

help

Show this help message again.

exit / quit

quit

Exit the debugger.

Example Workflow: Finding a Bug
Let's say your engine is failing perft(4) from the starting position.

Start the debugger:

./perft_debugger ./MyChessEngine

Set the depth:

> depth 4
Total depth set to 4.

Run the comparison:

> diff

The tool runs both engines. Imagine the output shows that the move d2d4 has an incorrect node count, highlighted in red. This tells you the bug does not lie with d2d4 itself, but somewhere in the tree of moves after d2d4 is played.

Drill down into the problematic move:

> move d2d4
Made move: d2d4

The debugger's internal board state is now after the move d2d4. The effective depth for the next test is automatically calculated as 4 - 1 = 3.

Run the comparison again:

> diff

You continue this process—finding a colored line, making that move with the move command, and running diff again. Eventually, you will trace the error down to depth 1. At that point, the diff output will show you the exact illegal move being generated or the legal move that is being missed by your engine, pinpointing the source of the bug.