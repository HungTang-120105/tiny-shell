#pragma once

#include <vector>
#include <string>

namespace Minesweeper {

enum class CellState {
    HIDDEN,
    REVEALED,
    FLAGGED
};

struct Cell {
    bool has_mine = false;
    CellState state = CellState::HIDDEN;
    int adjacent_mines = 0; // Number of mines in 8 surrounding cells
};

class MinesweeperGame {
public:
    MinesweeperGame(int rows, int cols, int mines);
    void play();

private:
    void initialize_board();
    void place_mines();
    void calculate_adjacent_mines();
    void print_board(bool game_over = false) const;
    bool is_valid(int r, int c) const;
    void open_cell(int r, int c);
    void toggle_flag(int r, int c);
    bool check_win() const;
    void reveal_all_mines();

    int rows_;
    int cols_;
    int mines_;
    std::vector<std::vector<Cell>> board_;
    bool game_over_;
    bool win_;
    int hidden_cells_remaining_;
    int flags_placed_;

    // For cursor-based input
    int cursor_row_ = 0;
    int cursor_col_ = 0;
};

// Main function to launch and play the game
void play_minesweeper_game();

} // namespace Minesweeper 