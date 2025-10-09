# **C++ PERFT-DEBUGGER-CLI**  

An interactive command-line tool to find move generation bugs in your chess engine by comparing its perft results against Stockfish.

This tool streamlines the often tedious process of perft debugging by providing a powerful REPL-style interface to set custom positions, run detailed comparisons, and systematically drill down into move lines where discrepancies are found.

## **About The Project**
Debugging a chess engine's move generator is one of the most challenging parts of its development. A single, subtle bug—like an incorrect castling flag update or a pawn promotion error—can be incredibly difficult to find. "Perft" (Performance Test) is the standard method for verifying a move generator's correctness. It works by counting the total number of legal moves to a certain depth and comparing that number to a known-correct result.

When a perft count is wrong at a high depth, it’s like trying to find a needle in a haystack of millions of positions. This tool automates and simplifies that exact workflow, turning a frustrating manual task into an efficient, guided process.

## **How It Works**
Instead of manually comparing text outputs and walking the move tree, this tool acts as a semi-automatic debugger. It keeps track of your position in the game tree, invokes both your engine and Stockfish with the correct parameters, and then presents a color-coded "diff" of the results, instantly highlighting discrepancies so they are easy to spot.

## **Features**
Interactive CLI: A simple and intuitive command-line prompt for a fast, iterative debugging cycle.

Direct Stockfish Comparison: Automatically uses Stockfish as a source of ground truth to validate your engine's output.

Drill-Down Debugging: Use move, unmove, and root commands to effortlessly navigate the move tree and isolate bugs.

# **Installation**
## Prerequisites

Before you begin, ensure you have the following installed:

- **C++17 compliant compiler** (e.g., `g++`, `clang++`)
- **Stockfish**: The Stockfish executable must be in your system's PATH.

### Downloading and Setting Up Stockfish

1. **Download**  
   Get the latest version from the [Stockfish Website](https://stockfishchess.org/download/).  

2. **Extract**  
   Unzip the downloaded file to find the `stockfish` executable.

3. **Add to System PATH**  

   - **Linux & macOS:** Copy the executable to `/usr/local/bin`:
     ```bash
     sudo cp stockfish /usr/local/bin/
     ```
   - **Windows:** Add the folder containing `stockfish.exe` to the PATH environment variable.


## **Compilation**
1. Clone this repository or place `perft_debugger.cpp` and your engine's `main.cpp` in a new folder.

2. Open a terminal in the project directory.

3. Compile the debugger tool:

```bash
g++ perft_debugger.cpp -o perft_debugger -std=c++17 -O2
```


## **Usage**
This section details how the debugger interacts with your engine and how to run it.

Your Engine's Executable `(MyChessEngine)`
The debugger requires your engine's executable to follow a specific command-line interface. The provided main.cpp template is already set up to handle this interaction correctly.

## Command-Line Invocation
The debugger will launch your engine with three arguments:

```bash
./MyChessEngine <depth> "<fen>" "[moves]"
```

`depth`: The depth parameter for the perft function.

`fen`: The FEN string of the "base" position.

`moves` (Optional): A space-separated string of moves in UCI notation from the base position.

### Expected Output Format
Your engine must print its results to standard output in the following format:

1. For each legal move, print a line containing the move in UCI notation, a space, and the node count for that branch.

2. After the list of all moves, print a single blank line.

3. Finally, print the total perft result on its own line.

**Example output for `perft(3)` from the starting position:**
```
a2a3 - 380  
a2a4 - 420  
b1a3 - 400  
b1c3 - 440  
b2b3 - 420  
b2b4 - 421  
c2c3 - 420  
c2c4 - 441  
d2d3 - 539  
d2d4 - 560  
e2e3 - 599  
e2e4 - 600  
f2f3 - 380  
f2f4 - 401  
g1f3 - 440  
g1h3 - 400  
g2g3 - 420  
g2g4 - 421  
h2h3 - 380  
h2h4 - 420  
Total: 8902  
```

**Running the Debugger**  
For the tool to work, both executables (`perft_debugger` and `MyChessEngine`) must be in the same directory.

1. Navigate your terminal into that directory.

2. Run the debugger with the following command:

```bash
./perft_debugger ./MyChessEngine
```

   You will be greeted with a help menu and a > prompt, ready for your commands.

## **Commands**
The interactive debugger is controlled by a set of simple commands:


| Command      | Example            | Description                                         |
|-------------|------------------|-----------------------------------------------------|
| `diff`      | `diff`            | Run comparison at the current position and depth  |
| `depth <N>` | `depth 5`         | Set the total perft depth to N                     |
| `fen <FEN>` | `fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1` | Set board position. This action clears all moves  |
| `move <MOVE>` | `move e2e4`      | Make a move in UCI notation to go one level deeper |
| `unmove`    | `unmove`          | Go back one move to explore other branches        |
| `root`      | `root`            | Reset to the initial FEN and clear all moves      |
| `help`      | `help`            | Show this help message again                       |
| `exit / quit` | `quit`          | Exit the debugger                                  |


Imagine your engine is failing `perft(4)` from the starting position.

## **Example Workflow: Finding a Bug**
1. **Start the debugger:**

```bash
./perft_debugger ./MyChessEngine
```

2. **Set the depth:**

`depth 4`

`Total depth set to 4.`

3. **Run the comparison:**

`diff`

The tool runs both engines. Imagine the output shows that the move `d2d4` has an incorrect node count, highlighted in orange. This tells you the bug lies somewhere in the tree of moves after `d2d4` is played.

4. **Drill down into the problematic move:**

`move d2d4`

`Made move: d2d4`

The debugger's internal board state is now updated. The effective depth for the next test is automatically 3.

5. **Run the comparison again:**

`diff`

You can continue this process until you reach depth 1, which will isolate the exact source of the error.

## REFERENCE VIDEO
<video src="https://github.com/Maverick-KVB37/PERFT-DEBUGGER-CLI/raw/main/PERFT_DEBUGEER_CLI.mkv" controls width="640">
  Your browser does not support the video tag.
</video>

## License
This project is licensed under the MIT `License`. See the [LICENSE][./LICENSE] file for details.