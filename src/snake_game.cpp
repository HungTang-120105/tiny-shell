#include "../include/snake_game.h"
#include <iostream>
#include <vector>
#include <windows.h> // For console manipulation (gotoxy, colors, Sleep)
#include <conio.h>   // For _kbhit() and _getch() (non-blocking input)
#include <thread>    // For std::this_thread::sleep_for
#include <chrono>    // For std::chrono::milliseconds
#include <cstdlib>   // For rand() and srand()
#include <ctime>     // For time() to seed srand()

// Console dimensions (can be adjusted)
const int BOARD_WIDTH = 40;
const int BOARD_HEIGHT = 20;

// Game elements
struct Point {
    int x, y;
};

std::vector<Point> snake;
Point food;
int score;
char currentDirection; // 'w', 'a', 's', 'd'
bool gameOver;

HANDLE hConsole;

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(hConsole, coord);
}

void setConsoleColor(WORD color) {
    SetConsoleTextAttribute(hConsole, color);
}

void setupGame() {
    srand(time(NULL)); // Seed for random food placement
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // Hide cursor
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    gameOver = false;
    currentDirection = 'd'; // Initial direction: right
    score = 0;

    // Initial snake position (e.g., center)
    snake.clear();
    snake.push_back({BOARD_WIDTH / 2, BOARD_HEIGHT / 2});
    snake.push_back({BOARD_WIDTH / 2 - 1, BOARD_HEIGHT / 2});

    // Place initial food
    food.x = rand() % BOARD_WIDTH;
    food.y = rand() % BOARD_HEIGHT;
}

void drawBorder() {
    setConsoleColor(FOREGROUND_BLUE | FOREGROUND_GREEN); // Cyan border
    for (int i = 0; i < BOARD_WIDTH + 2; ++i) {
        gotoxy(i, 0); std::cout << "#";
        gotoxy(i, BOARD_HEIGHT + 1); std::cout << "#";
    }
    for (int i = 0; i < BOARD_HEIGHT + 1; ++i) {
        gotoxy(0, i); std::cout << "#";
        gotoxy(BOARD_WIDTH + 1, i); std::cout << "#";
    }
    setConsoleColor(7); // Reset to default
}

void drawSnake() {
    setConsoleColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Bright Green snake
    for (size_t i = 0; i < snake.size(); ++i) {
        gotoxy(snake[i].x + 1, snake[i].y + 1);
        std::cout << (i == 0 ? "O" : "o"); // Head and body
    }
    setConsoleColor(7);
}

void drawFood() {
    setConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY); // Bright Red food
    gotoxy(food.x + 1, food.y + 1);
    std::cout << "@";
    setConsoleColor(7);
}

void drawScore() {
    gotoxy(BOARD_WIDTH + 4, 2);
    std::cout << "Score: " << score;
}

void handleInput() {
    if (_kbhit()) { // Check if a key has been pressed
        char key = _getch(); // Get the pressed key
        switch (key) {
            case 'w': case 'W': if (currentDirection != 's') currentDirection = 'w'; break;
            case 's': case 'S': if (currentDirection != 'w') currentDirection = 's'; break;
            case 'a': case 'A': if (currentDirection != 'd') currentDirection = 'a'; break;
            case 'd': case 'D': if (currentDirection != 'a') currentDirection = 'd'; break;
            case 27: gameOver = true; break; // ESC key to exit
        }
    }
}

void updateGameLogic() {
    Point newHead = snake[0];
    switch (currentDirection) {
        case 'w': newHead.y--; break;
        case 's': newHead.y++; break;
        case 'a': newHead.x--; break;
        case 'd': newHead.x++; break;
    }

    // Check wall collision
    if (newHead.x < 0 || newHead.x >= BOARD_WIDTH || newHead.y < 0 || newHead.y >= BOARD_HEIGHT) {
        gameOver = true;
        return;
    }

    // Check self-collision
    for (size_t i = 1; i < snake.size(); ++i) {
        if (newHead.x == snake[i].x && newHead.y == snake[i].y) {
            gameOver = true;
            return;
        }
    }

    snake.insert(snake.begin(), newHead); // Add new head

    // Check food collision
    if (newHead.x == food.x && newHead.y == food.y) {
        score++;
        // Place new food (ensure it's not on the snake)
        bool foodOnSnake;
        do {
            foodOnSnake = false;
            food.x = rand() % BOARD_WIDTH;
            food.y = rand() % BOARD_HEIGHT;
            for (const auto& segment : snake) {
                if (food.x == segment.x && food.y == segment.y) {
                    foodOnSnake = true;
                    break;
                }
            }
        } while (foodOnSnake);
    } else {
        snake.pop_back(); // Remove tail if no food eaten
    }
}

void clear() {
    // A more robust clear screen for the game area
    for (int y = 0; y < BOARD_HEIGHT + 2; ++y) {
        for (int x = 0; x < BOARD_WIDTH + 50; ++x) { // Clear wider for score etc.
            gotoxy(x, y);
            std::cout << " ";
        }
    }
}

void playSnakeGame() {
    setupGame();
    bool firstFrame = true;

    while (!gameOver) {
        Point oldTailPosition = {-1, -1}; // Store old tail to clear it
        if (!snake.empty()) {
            oldTailPosition = snake.back();
        }

        handleInput();
        updateGameLogic();

        if (gameOver) break;

        // Drawing Phase
        if (firstFrame) {
            clear(); // Full clear for the first frame
            drawBorder();
            firstFrame = false;
        } else {
            // Clear only the old tail position
            if (oldTailPosition.x != -1) {
                gotoxy(oldTailPosition.x + 1, oldTailPosition.y + 1);
                std::cout << " ";
            }
        }

        drawSnake();
        drawFood();
        drawScore();
        
        // Adjust speed - ensure it doesn't become too fast or negative
        long long sleepDuration = 150 - (long long)score * 3;
        if (sleepDuration < 50) sleepDuration = 50; // Minimum delay
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepDuration));
    }

    // Game Over Sequence
    clear(); // Clear board for game over message
    gotoxy(BOARD_WIDTH / 2 - 5, BOARD_HEIGHT / 2 - 1);
    setConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Yellow
    std::cout << "GAME OVER!";
    gotoxy(BOARD_WIDTH / 2 - 7, BOARD_HEIGHT / 2 + 1);
    std::cout << "Final Score: " << score;
    gotoxy(BOARD_WIDTH / 2 - 15, BOARD_HEIGHT / 2 + 3);
    std::cout << "Press any key to return to shell...";
    setConsoleColor(7); // Reset color

    _getch(); // Wait for a key press

    // Restore cursor visibility
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = true;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
    
    // Clear screen one last time before returning to shell
    system("cls"); 
} 