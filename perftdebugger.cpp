#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <memory>
#include <optional>
#include <iomanip>
#include <algorithm>
#include <stdexcept>

// platform-specific includes for process management
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define IS_TTY _isatty(_fileno(stdout))
#else
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#define IS_TTY isatty(fileno(stdout))
#endif

// --- ANSI Colors for Terminal Output ---
namespace Color {
    const std::string ORANGE = "\033[38;2;255;137;79m";
    const std::string PINK   = "\033[38;2;234;91;111m";
    const std::string CYAN   = "\033[38;2;10;186;181m";
    const std::string RESET  = "\033[38;2;169;178;195m";
}

// =================================================================================
// ==================== A simple cross-platform subprocess manager==================
// =================================================================================
class Subprocess {
private:
#ifdef _WIN32
    PROCESS_INFORMATION processInfo_{};
    HANDLE childStdIn_ = NULL;
    HANDLE childStdOut_ = NULL;
    std::string read_buffer_;
#else
    pid_t pid_ = -1;
    FILE* childStdIn_ = nullptr;
    FILE* childStdOut_ = nullptr;
#endif
    bool is_running_ = false;

public:
    Subprocess(const std::string& command, const std::vector<std::string>& args) {
#ifdef _WIN32
        HANDLE hChildStd_IN_Rd = NULL;
        HANDLE hChildStd_IN_Wr = NULL;
        HANDLE hChildStd_OUT_Rd = NULL;
        HANDLE hChildStd_OUT_Wr = NULL;

        SECURITY_ATTRIBUTES saAttr;
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0) ||
            !SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) {
            throw std::runtime_error("Failed to create stdout pipe");
        }
        if (!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0) ||
            !SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)) {
            throw std::runtime_error("Failed to create stdin pipe");
        }

        STARTUPINFO si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        si.hStdOutput = hChildStd_OUT_Wr;
        si.hStdInput = hChildStd_IN_Rd;
        si.dwFlags |= STARTF_USESTDHANDLES;

        std::string cmdLine = "\"" + command + "\"";
        for (const auto& arg : args) {
            cmdLine += " \"" + arg + "\"";
        }

        if (!CreateProcess(NULL, const_cast<char*>(cmdLine.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &processInfo_)) {
            throw std::runtime_error("CreateProcess failed for command: " + command);
        }
        
        CloseHandle(hChildStd_OUT_Wr);
        CloseHandle(hChildStd_IN_Rd);

        childStdIn_ = hChildStd_IN_Wr;
        childStdOut_ = hChildStd_OUT_Rd;
#else
        int stdin_pipe[2];
        int stdout_pipe[2];
        if (pipe(stdin_pipe) != 0 || pipe(stdout_pipe) != 0) {
            throw std::runtime_error("pipe() failed");
        }

        pid_ = fork();
        if (pid_ < 0) {
            throw std::runtime_error("fork() failed");
        }

        if (pid_ == 0) { // Child process
            close(stdin_pipe[1]);
            close(stdout_pipe[0]);
            dup2(stdin_pipe[0], STDIN_FILENO);
            dup2(stdout_pipe[1], STDOUT_FILENO);
            close(stdin_pipe[0]);
            close(stdout_pipe[1]);

            std::vector<char*> argv;
            argv.push_back(const_cast<char*>(command.c_str()));
            for (const auto& arg : args) {
                argv.push_back(const_cast<char*>(arg.c_str()));
            }
            argv.push_back(nullptr);
            execvp(argv[0], argv.data());
            _exit(127); // execvp only returns on error
        }

        // Parent process
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);
        childStdIn_ = fdopen(stdin_pipe[1], "w");
        childStdOut_ = fdopen(stdout_pipe[0], "r");
        if (!childStdIn_ || !childStdOut_) {
             throw std::runtime_error("fdopen() failed");
        }
#endif
        is_running_ = true;
    }

    ~Subprocess() {
        if (is_running_) {
#ifdef _WIN32
            TerminateProcess(processInfo_.hProcess, 1);
            CloseHandle(processInfo_.hProcess);
            CloseHandle(processInfo_.hThread);
            CloseHandle(childStdIn_);
            CloseHandle(childStdOut_);
#else
            kill(pid_, SIGKILL);
            waitpid(pid_, nullptr, 0);
            if(childStdIn_) fclose(childStdIn_);
            if(childStdOut_) fclose(childStdOut_);
#endif
        }
    }

    void write(const std::string& data) {
#ifdef _WIN32
        DWORD bytesWritten;
        if (!WriteFile(childStdIn_, data.c_str(), data.length(), &bytesWritten, NULL)) {
             throw std::runtime_error("WriteFile to child stdin failed");
        }
#else
        fputs(data.c_str(), childStdIn_);
        fflush(childStdIn_);
#endif
    }
    
    bool read_line(std::string& line) {
#ifdef _WIN32
        size_t newline_pos = read_buffer_.find('\n');
        while (newline_pos == std::string::npos) {
            char buffer[4096];
            DWORD bytesRead = 0;
            if (!ReadFile(childStdOut_, buffer, sizeof(buffer), &bytesRead, NULL) || bytesRead == 0) {
                if (!read_buffer_.empty()) { 
                    line = read_buffer_;
                    read_buffer_.clear();
                    return true;
                }
                return false;
            }
            read_buffer_.append(buffer, bytesRead);
            newline_pos = read_buffer_.find('\n');
        }
        line = read_buffer_.substr(0, newline_pos);
        read_buffer_.erase(0, newline_pos + 1);
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        return true;
#else
        char* buf = nullptr;
        size_t len = 0;
        ssize_t read = getline(&buf, &len, childStdOut_);
        if (read != -1) {
            line = std::string(buf);
            if (!line.empty() && line.back() == '\n') {
                line.pop_back();
            }
            free(buf);
            return true;
        }
        if (buf) free(buf);
        return false;
#endif
    }

    void wait() {
        if (!is_running_) return;
#ifdef _WIN32
        WaitForSingleObject(processInfo_.hProcess, INFINITE);
        CloseHandle(processInfo_.hProcess);
        CloseHandle(processInfo_.hThread);
#else
        waitpid(pid_, nullptr, 0);
#endif
        is_running_ = false;
    }
};

// =================================================================================
// ============================ Perft and Engine Logic =============================
// =================================================================================

struct PerftResult {
    long long total_nodes = 0;
    std::map<std::string, long long> move_nodes;
};

class Engine {
public:
    virtual ~Engine() = default;
    virtual PerftResult run_perft(const std::string& fen, const std::vector<std::string>& moves, int depth) = 0;
};

class UserEngine : public Engine {
private:
    std::string path_;
public:
    UserEngine(std::string path) : path_(std::move(path)) {}

    PerftResult run_perft(const std::string& fen, const std::vector<std::string>& moves, int depth) override {
        PerftResult result;
        std::vector<std::string> args;
        args.push_back(std::to_string(depth));
        args.push_back(fen);
        if (!moves.empty()) {
            std::stringstream ss;
            for (size_t i = 0; i < moves.size(); ++i) {
                ss << moves[i] << (i == moves.size() - 1 ? "" : " ");
            }
            args.push_back(ss.str());
        }

        Subprocess proc(path_, args);
        
        std::string line;
        while (proc.read_line(line)) {
            if (line.empty()) continue;
            std::stringstream line_ss(line);
            std::string move_str;
            long long count;
            if (line_ss >> move_str >> count) {
                result.move_nodes[move_str] = count;
            } else {
                 try {
                    result.total_nodes = std::stoll(line);
                 } catch(...) { /* ignore parse error */ }
            }
        }
        proc.wait();
        return result;
    }
};

class Stockfish : public Engine {
private:
    std::unique_ptr<Subprocess> process_;
public:
    Stockfish() {
        try {
            process_ = std::make_unique<Subprocess>("stockfish", std::vector<std::string>{});
        } catch (const std::exception& e) {
            throw std::runtime_error("CRITICAL: Failed to start the 'stockfish' process. Is it installed and in your system's PATH?");
        }
        
        process_->write("uci\n");
        std::string line;
        bool uciok_received = false;
        // Basic timeout mechanism
        for(int i=0; i < 50; ++i) { // Try reading up to 50 lines
            if(process_->read_line(line) && line.find("uciok") != std::string::npos) {
                uciok_received = true;
                break;
            }
        }
        if (!uciok_received) {
            throw std::runtime_error("CRITICAL: Did not receive 'uciok' confirmation from Stockfish. The engine may have failed to start correctly.");
        }
    }

    PerftResult run_perft(const std::string& fen, const std::vector<std::string>& moves, int depth) override {
        PerftResult result;
        std::stringstream cmd;
        cmd << "position fen " << fen;
        if (!moves.empty()) {
            cmd << " moves";
            for (const auto& move : moves) cmd << " " << move;
        }
        cmd << "\n";
        cmd << "go perft " << depth << "\n";
        process_->write(cmd.str());
        
        std::string line;
        while (process_->read_line(line)) {
            if (line.find("Nodes searched:") != std::string::npos) {
                result.total_nodes = std::stoll(line.substr(line.find(":") + 1));
                break;
            }
            std::stringstream ss(line);
            std::string move_str, count_str;
            if(ss >> move_str >> count_str && !move_str.empty() && move_str.back() == ':') {
                 move_str.pop_back();
                 result.move_nodes[move_str] = std::stoll(count_str);
            }
        }
        return result;
    }
};

// =================================================================================
// =========================== Diff and State Management ===========================
// =================================================================================

struct DiffResult {
    std::pair<long long, long long> total_nodes;
    std::map<std::string, std::pair<std::optional<long long>, std::optional<long long>>> move_nodes;

    DiffResult(const PerftResult& user_result, const PerftResult& stockfish_result) {
        total_nodes = {user_result.total_nodes, stockfish_result.total_nodes};
        for (const auto& pair : user_result.move_nodes) move_nodes[pair.first].first = pair.second;
        for (const auto& pair : stockfish_result.move_nodes) move_nodes[pair.first].second = pair.second;
    }
};

void print_diff(const DiffResult& diff) {
    size_t max_node_width = 0;
    for (const auto& pair : diff.move_nodes) {
        if (pair.second.first.has_value()) {
            max_node_width = std::max(max_node_width, std::to_string(pair.second.first.value()).length());
        }
        if (pair.second.second.has_value()) {
            max_node_width = std::max(max_node_width, std::to_string(pair.second.second.value()).length());
        }
    }
    max_node_width = std::max(max_node_width, (size_t)10);

    std::cout << std::left << std::setw(8) << "Move"
              << std::right << std::setw(max_node_width) << "YourEngine"
              << std::right << std::setw(max_node_width + 1) << "Stockfish" << std::endl;
    std::cout << std::left << std::setw(8) << "--------"
              << std::right << std::setw(max_node_width) << "----------"
              << std::right << std::setw(max_node_width + 1) << "----------" << std::endl;

    for (const auto& pair : diff.move_nodes) {
        long long lhs = pair.second.first.value_or(0);
        long long rhs = pair.second.second.value_or(0);
        
        bool missing_in_user = !pair.second.first.has_value();
        bool missing_in_stockfish = !pair.second.second.has_value();
        bool counts_differ = !missing_in_user && !missing_in_stockfish && lhs != rhs;

        if (missing_in_user) std::cout << Color::PINK;
        else if (missing_in_stockfish) std::cout << Color::CYAN;
        else if (counts_differ) std::cout << Color::ORANGE;

        std::cout << std::left << std::setw(8) << pair.first;

        if (missing_in_user) {
            std::cout << std::right << std::setw(max_node_width) << "-";
        } else {
            std::cout << std::right << std::setw(max_node_width) << lhs;
        }

        if (missing_in_stockfish) {
            std::cout << std::right << std::setw(max_node_width + 1) << "-";
        } else {
            std::cout << std::right << std::setw(max_node_width + 1) << rhs;
        }
        
        std::cout << std::endl;

        if (missing_in_user || missing_in_stockfish || counts_differ) {
            std::cout << Color::RESET;
        }
    }

    std::cout << "\n";
    bool total_is_different = diff.total_nodes.first != diff.total_nodes.second;
    if (total_is_different) std::cout << Color::ORANGE;
    std::cout << "Total    " << diff.total_nodes.first << "\t" << diff.total_nodes.second << std::endl;
    if (total_is_different) std::cout << Color::RESET;
}

class State {
private:
    std::unique_ptr<Engine> user_engine_;
    std::unique_ptr<Engine> stockfish_engine_;
    std::string fen_ = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::vector<std::string> moves_;
    int depth_ = 1;

public:
    State(const std::string& user_engine_path) {
        user_engine_ = std::make_unique<UserEngine>(user_engine_path);
        stockfish_engine_ = std::make_unique<Stockfish>();
    }

    void set_fen(std::string new_fen) { fen_ = std::move(new_fen); moves_.clear(); }
    void set_depth(int d) { depth_ = d; }
    void goto_root() { moves_.clear(); }
    void goto_parent() { if (!moves_.empty()) moves_.pop_back(); }
    void goto_child(const std::string& move) { moves_.push_back(move); }

    void run_diff() {
        int current_depth = std::max(1, depth_ - static_cast<int>(moves_.size()));
        std::cout << "\n--- Running Perft ---\n";
        std::cout << "FEN: " << fen_ << std::endl;
        std::cout << "Moves: ";
        if(moves_.empty()) std::cout << "(none)";
        else for(const auto& m : moves_) std::cout << m << " ";
        std::cout << "\nDepth: " << current_depth << std::endl << std::endl;

        PerftResult user_result = user_engine_->run_perft(fen_, moves_, current_depth);
        PerftResult stockfish_result = stockfish_engine_->run_perft(fen_, moves_, current_depth);
        
        DiffResult diff(user_result, stockfish_result);
        print_diff(diff);
    }
};

void print_help() {
    std::cout << "\n--- Perft Debugger Commands ---\n"
              << "diff          - Run comparison at the current position.\n"
              << "depth <N>     - Set the total perft depth.\n"
              << "fen <FEN>     - Set the board FEN. Clears current moves.\n"
              << "move <m>      - Make a move (e.g., move e2e4).\n"
              << "unmove        - Go back one move.\n"
              << "root          - Return to the starting FEN, clear all moves.\n"
              << "help          - Show this help message.\n"
              << "exit / quit   - Close the debugger.\n"
              << std::endl;
}

// =================================================================================
// Main Application Loop
// =================================================================================
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_your_chess_engine>" << std::endl;
        return 1;
    }
    
    print_help(); // Print help before starting, so user always sees it.

    try {
        State state(argv[1]);
        std::string line;
        while (true) {
            if (IS_TTY) std::cout << "> " << std::flush;
            if (!std::getline(std::cin, line)) break;

            std::stringstream ss(line);
            std::string command;
            ss >> command;

            if (command == "fen") {
                std::string fen_str;
                std::getline(ss, fen_str);
                fen_str.erase(0, fen_str.find_first_not_of(" \t\n\r"));
                if (fen_str.empty()) std::cerr << "Error: FEN cannot be empty." << std::endl;
                else {
                    state.set_fen(fen_str);
                    std::cout << "FEN set. Moves cleared." << std::endl;
                }
            } else if (command == "depth") {
                int d;
                if (ss >> d && d > 0) {
                    state.set_depth(d);
                    std::cout << "Total depth set to " << d << "." << std::endl;
                } else {
                    std::cerr << "Error: Please provide a positive integer for depth." << std::endl;
                }
            } else if (command == "root") {
                state.goto_root();
                std::cout << "Reset to root position." << std::endl;
            } else if (command == "unmove" || command == "parent") {
                state.goto_parent();
                std::cout << "Moved back one position." << std::endl;
            } else if (command == "move" || command == "child") {
                std::string move_to_make;
                if (ss >> move_to_make) {
                    state.goto_child(move_to_make);
                    std::cout << "Made move: " << move_to_make << std::endl;
                } else {
                    std::cerr << "Error: Please provide a move (e.g., move e2e4)." << std::endl;
                }
            } else if (command == "diff") {
                state.run_diff();
            } else if (command == "help") {
                print_help();
            } else if (command == "exit" || command == "quit") {
                break;
            } else if (!command.empty()) {
                std::cerr << "Unknown command: '" << command << "'. Type 'help' for a list of commands." << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "\n" << Color::ORANGE << "FATAL ERROR: " << e.what() << Color::RESET << std::endl;
        return 1;
    }

    return 0;
}

    

