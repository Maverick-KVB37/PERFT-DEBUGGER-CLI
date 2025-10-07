//============================= main.cpp of chess engine =====================

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>
#include <cassert>
#include <iomanip>
#include <array>
#include <cmath>


/*
#include "chess.h"
#include "evaluation.h"
#include "search.h"
#include "psqt.h"
#include "uci.h"
using namespace std;


// =============================================================================
// ================================= PERFT LOGIC ===============================
// =============================================================================

uint64_t perft(Position& pos, int depth) {
    if (depth == 0) {
        return 1;
    }
    
    Moves legal_moves;
    if (pos.sideToMove == Color::White) {
        legal_moves = pos.generateLegalMoves<Color::White>();
    } else {
        legal_moves = pos.generateLegalMoves<Color::Black>();
    }

    uint64_t nodes = 0;
    if (legal_moves.count == 0) {
        return 0;
    }

    for (int i = 0; i < legal_moves.count; i++) {
        Move move = legal_moves.moves[i];
        Position new_pos = pos;
        if (new_pos.sideToMove == Color::White) {
            new_pos.makemove<Color::White>(move);
        } else {
            new_pos.makemove<Color::Black>(move);
        }
        nodes += perft(new_pos, depth - 1);
    }
    return nodes;
}

void runPerftAndPrint(Position& pos, int depth) {
    if (depth == 0) {
        std::cout << "\n1" << std::endl;
        return;
    }

    Moves legal_moves;
    if (pos.sideToMove == Color::White) {
        legal_moves = pos.generateLegalMoves<Color::White>();
    } else {
        legal_moves = pos.generateLegalMoves<Color::Black>();
    }
    
    uint64_t total_nodes = 0;

    for (int i = 0; i < legal_moves.count; i++) {
        Move move = legal_moves.moves[i];
        Position new_pos = pos;

        if (new_pos.sideToMove == Color::White) {
            new_pos.makemove<Color::White>(move);
        } else {
            new_pos.makemove<Color::Black>(move);
        }
        
        uint64_t nodes = perft(new_pos, depth - 1);
        total_nodes += nodes;
        
        std::cout << move.toUci() << " " << nodes << std::endl;
    }

    std::cout << std::endl;
    std::cout << total_nodes << std::endl;
}


// =============================================================================
// =============================== MAIN FUNCTION ===============================
// =============================================================================
int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./MyChessEngine <depth> <fen> [moves]" << std::endl;
        return 1;
    }

    // --- Argument Parsing ---
    int depth = std::stoi(argv[1]);
    std::string fen = argv[2];
    std::vector<std::string> moves_vec;
    if (argc > 3) {
        std::stringstream ss(argv[3]);
        std::string move_str;
        while (ss >> move_str) {
            moves_vec.push_back(move_str);
        }
    }

    // --- Engine Setup ---
    Position pos;
    pos.parseFEN(fen);

    // Make the initial moves passed by the debugger
    for (const std::string& move_uci : moves_vec) {
        Moves legal_moves;
        if (pos.sideToMove == Color::White) {
            legal_moves = pos.generateLegalMoves<Color::White>();
        } else {
            legal_moves = pos.generateLegalMoves<Color::Black>();
        }
        for (int i = 0; i < legal_moves.count; i++) {
            Move move = legal_moves.moves[i];
            if (move.toUci() == move_uci) {
                if (pos.sideToMove == Color::White) {
                    pos.makemove<Color::White>(move);
                } else {
                    pos.makemove<Color::Black>(move);
                }
                break;
            }
        }
    }

    // --- Run and Print Perft ---
    runPerftAndPrint(pos, depth);

    // --- THE FIX ---
    // Force all the output above to be sent to the debugger before we exit.
    fflush(stdout);

    return 0;
}


*/





