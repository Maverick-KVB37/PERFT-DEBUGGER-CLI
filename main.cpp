#include <iostream>
#include <string>
#include <vector>
#include <sstream>

int main(int argc, char* argv[]) {
    // argc is the count of arguments.
    // argv is an array of the argument strings.
    // argv[0] is the program name itself.

    if (argc < 3) {
        std::cerr << "Error: Not enough arguments provided." << std::endl;
        std::cerr << "Usage: ./your_engine <depth> <fen> [moves]" << std::endl;
        return 1; // Exit with an error
    }

    // --- Read the required arguments ---

    // Argument 1: Depth (convert string to integer)
    int depth = std::stoi(argv[1]);

    // Argument 2: FEN String
    std::string fen = argv[2];

    // --- Read the optional moves argument ---
    std::vector<std::string> moves;
    if (argc > 3) {
        std::string moves_line = argv[3];
        std::stringstream ss(moves_line);
        std::string single_move;
        // Split the string of moves into individual moves
        while (ss >> single_move) {
            moves.push_back(single_move);
        }
    }

    // --- Now you have the data in variables ---
    std::cout << "Engine received:" << std::endl;
    std::cout << "  Depth: " << depth << std::endl;
    std::cout << "  FEN: " << fen << std::endl;
    std::cout << "  Moves to make: ";
    for (const std::string& move : moves) {
        std::cout << move << " ";
    }
    std::cout << std::endl;


    return 0;
}