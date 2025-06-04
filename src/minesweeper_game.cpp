#include "minesweeper_game.h"
#include <iostream>
#include <vector>
#include <random>
#include <algorithm> // For std::shuffle
#include <ctime>     // For std::time
#include <conio.h>   // For _getch (Windows)
#include <windows.h> // For console manipulation (Windows)

// Helper function to set cursor position (Windows specific)
void gotoxy_ms(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// Helper function to set text color (Windows specific)
void set_color_ms(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

namespace Minesweeper {

MinesweeperGame::MinesweeperGame(int rows, int cols, int mines)
    : rows_(rows), cols_(cols), mines_(mines), game_over_(false), win_(false) {
    if (mines_ >= rows_ * cols_) {
        // Ensure mines are less than total cells, can make this more robust
        mines_ = rows_ * cols_ - 1;
    }
    board_.resize(rows_, std::vector<Cell>(cols_));
    hidden_cells_remaining_ = rows_ * cols_ - mines_;
    flags_placed_ = 0;
    cursor_row_ = 0;
    cursor_col_ = 0;
    initialize_board();
    place_mines();
    calculate_adjacent_mines();
}

void MinesweeperGame::initialize_board() {
    for (int i = 0; i < rows_; ++i) {
        for (int j = 0; j < cols_; ++j) {
            board_[i][j].has_mine = false;
            board_[i][j].state = CellState::HIDDEN;
            board_[i][j].adjacent_mines = 0;
        }
    }
}

void MinesweeperGame::place_mines() {
    std::vector<int> positions(rows_ * cols_);
    for (int i = 0; i < rows_ * cols_; ++i) {
        positions[i] = i;
    }

    std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
    std::shuffle(positions.begin(), positions.end(), rng);

    for (int i = 0; i < mines_; ++i) {
        int r = positions[i] / cols_;
        int c = positions[i] % cols_;
        board_[r][c].has_mine = true;
    }
}

void MinesweeperGame::calculate_adjacent_mines() {
    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            if (board_[r][c].has_mine) {
                continue;
            }
            int mine_count = 0;
            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {
                    if (dr == 0 && dc == 0) continue;
                    int nr = r + dr;
                    int nc = c + dc;
                    if (is_valid(nr, nc) && board_[nr][nc].has_mine) {
                        mine_count++;
                    }
                }
            }
            board_[r][c].adjacent_mines = mine_count;
        }
    }
}

bool MinesweeperGame::is_valid(int r, int c) const {
    return r >= 0 && r < rows_ && c >= 0 && c < cols_;
}

void MinesweeperGame::print_board(bool show_all) const {
    system("cls"); // Clear screen (Windows specific)
    gotoxy_ms(0, 0);
    std::cout << "Minesweeper Game\n";
    std::cout << "Mines: " << mines_ << "  Flags: " << flags_placed_ << "\n\n";

    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            if (r == cursor_row_ && c == cursor_col_) {
                set_color_ms(BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY); // Bright White on Blue
            } else {
                 set_color_ms(7); // Default: White on Black
            }

            if (show_all && board_[r][c].has_mine) {
                std::cout << " * ";
            } else {
                switch (board_[r][c].state) {
                    case CellState::HIDDEN:
                        std::cout << " . ";
                        break;
                    case CellState::REVEALED:
                        if (board_[r][c].has_mine) {
                            set_color_ms(FOREGROUND_RED | FOREGROUND_INTENSITY);
                            std::cout << " X "; // Should only happen on game over if show_all is false
                        } else if (board_[r][c].adjacent_mines > 0) {
                            set_color_ms(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                            std::cout << " " << board_[r][c].adjacent_mines << " ";
                        } else {
                            std::cout << "   "; // Empty revealed cell
                        }
                        break;
                    case CellState::FLAGGED:
                        set_color_ms(FOREGROUND_RED | FOREGROUND_INTENSITY);
                        std::cout << " P "; // P for Planted flag
                        break;
                }
            }
            if (r == cursor_row_ && c == cursor_col_) {
                 set_color_ms(7); // Reset color
            }
        }
        std::cout << std::endl;
    }
    set_color_ms(7); // Reset to default color
    std::cout << "\nControls: Arrow keys to move, Space to open, 'f' to flag, 'q' to quit.\n";
    if (game_over_) {
        if (win_) {
            std::cout << "YOU WIN! :)\n";
        } else {
            std::cout << "GAME OVER! :( \n";
        }
    }
}

// --- Placeholder / To be implemented --- 
void MinesweeperGame::open_cell(int r, int c) {
    // Logic for opening a cell, recursive opening for empty cells
    // Update game_over_, win_, hidden_cells_remaining_
    // For now, just mark as revealed for testing print_board
    if (is_valid(r,c) && board_[r][c].state == CellState::HIDDEN) {
        board_[r][c].state = CellState::REVEALED;
        if(board_[r][c].has_mine) {
            game_over_ = true;
            win_ = false;
            reveal_all_mines();
        } else {
            hidden_cells_remaining_--;
             if (board_[r][c].adjacent_mines == 0) {
                // Recursively open neighbors
                for (int dr = -1; dr <= 1; ++dr) {
                    for (int dc = -1; dc <= 1; ++dc) {
                        if (dr == 0 && dc == 0) continue;
                        open_cell(r + dr, c + dc);
                    }
                }
            }
            if (check_win()) {
                game_over_ = true;
                win_ = true;
            }
        }
    }
}

void MinesweeperGame::toggle_flag(int r, int c) {
    // Logic for flagging/unflagging a cell
    // Update flags_placed_
    if(is_valid(r,c) && !game_over_){
        if(board_[r][c].state == CellState::HIDDEN){
            board_[r][c].state = CellState::FLAGGED;
            flags_placed_++;
        } else if (board_[r][c].state == CellState::FLAGGED) {
            board_[r][c].state = CellState::HIDDEN;
            flags_placed_--;
        }
    }
}

bool MinesweeperGame::check_win() const {
    return hidden_cells_remaining_ == 0;
}

void MinesweeperGame::reveal_all_mines() {
    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            if (board_[r][c].has_mine) {
                board_[r][c].state = CellState::REVEALED;
            }
        }
    }
}

void MinesweeperGame::play() {
    // Main game loop: get input, update state, print board
    // This will be implemented in the next step.
    // For now, we can test with some manual calls
    // open_cell(0,0); // example test

    char input;
    print_board(); // Initial print

    while (!game_over_) {
        gotoxy_ms(cursor_col_ * 3, cursor_row_ + 3); // Position cursor (approx)
        input = _getch(); // Windows specific

        switch (input) {
            case 72: // Up arrow
                if (cursor_row_ > 0) cursor_row_--;
                break;
            case 80: // Down arrow
                if (cursor_row_ < rows_ - 1) cursor_row_++;
                break;
            case 75: // Left arrow
                if (cursor_col_ > 0) cursor_col_--;
                break;
            case 77: // Right arrow
                if (cursor_col_ < cols_ - 1) cursor_col_++;
                break;
            case ' ': // Space bar
                open_cell(cursor_row_, cursor_col_);
                break;
            case 'f':
            case 'F':
                toggle_flag(cursor_row_, cursor_col_);
                break;
            case 'q':
            case 'Q':
                game_over_ = true;
                win_ = false; // Consider it a loss if quit
                std::cout << "Quitting game." << std::endl;
                return; // Exit play loop
        }
        print_board(game_over_ && !win_); // Show all mines if game over and lost
    }
    // Final print to show win/loss message
    print_board(true); 
}

// Main function to launch and play the game (as declared in .h)
void play_minesweeper_game() {
    // For now, let's use some default values.
    // Later, we can ask the user for difficulty/board size.
    int rows = 10;
    int cols = 10;
    int mines = 15;

    std::cout << "Starting Minesweeper (Default: " << rows << "x" << cols << ", " << mines << " mines)..." << std::endl;
    Sleep(1000); // Pause for a moment

MinesweeperGame game(rows, cols, mines);
    game.play();

    std::cout << "Press any key to return to shell." << std::endl;
    _getch(); // Wait for key press
    system("cls"); // Clear screen before returning
}

} // namespace Minesweeper 