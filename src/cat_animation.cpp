#include "cat_animation.h"
#include <iostream>
#include <vector>
#include <string>
#include <thread>        // For std::this_thread::sleep_for
#include <chrono>        // For std::chrono::milliseconds
#include <conio.h>       // For _kbhit and _getch (Windows)
#include <windows.h>     // For system("cls") and SetConsoleCursorPosition (Windows)

// Helper function to set cursor position (Windows specific)
void gotoxy_cat(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void clear_screen_cat() {
#ifdef _WIN32
    system("cls");
#else
    // Assume POSIX
    system("clear");
#endif
}

namespace CatAnimation {

// ASCII art frames for a simple jumping cat
const std::vector<std::string> cat_frames = {
    // Frame 1: Crouching
    "    /\\_/\\\n" 
    "   ( o.o )\n" 
    "    > ^ <\n" 
    "   (     )\n" 
    "  (_______)",

    // Frame 2: Mid-jump
    "    /\\_/\\\n" 
    "   ( o.o )\n" 
    "    > ^ <\n" 
    "     ) (\n" 
    "    (   )",

    // Frame 3: Peak jump / Stretched
    "    /\\_/\\\n" 
    "   ( o.o )\n" 
    "    > ^ <\n" 
    "    /   \\\n" 
    "   (____)",

    // Frame 4: Coming down (same as mid-jump or slightly different)
    "    /\\_/\\\n" 
    "   ( o.o )\n" 
    "    > ^ <\n" 
    "     ) (\n" 
    "    (   )"
};

void play_nyancat_animation() {
    clear_screen_cat();
    std::cout << "Starting Nyan Cat Animation... Press any key to stop." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));

    int current_frame = 0;
    int console_width = 80; // Approximate console width for centering
    int cat_art_width = 11; // Approximate width of the cat art
    int start_x = (console_width - cat_art_width) / 2;
    if (start_x < 0) start_x = 0;

    // Get console height to attempt vertical centering or placement
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int console_height = 25; // Default
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        console_height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
    int start_y = (console_height - 5) / 2; // 5 lines for the cat art
    if (start_y < 0) start_y = 0;

    while (true) {
        clear_screen_cat();
        gotoxy_cat(start_x, start_y); // Position the cat art
        
        // Print frame with manual newlines for gotoxy compatibility
        std::string frame = cat_frames[current_frame];
        size_t pos = 0;
        int current_line_y = start_y;
        while ((pos = frame.find('\n')) != std::string::npos) {
            gotoxy_cat(start_x, current_line_y++);
            std::cout << frame.substr(0, pos);
            frame.erase(0, pos + 1);
        }
        gotoxy_cat(start_x, current_line_y);
        std::cout << frame; // Print the last line

        gotoxy_cat(0, console_height -1); // Move cursor to bottom for message
        std::cout << "Press any key to stop the animation...";

        // Check for key press to exit (non-blocking)
        if (_kbhit()) { // Windows specific
            _getch();    // Consume the key
            break;
        }

        current_frame = (current_frame + 1) % cat_frames.size();
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Animation speed
    }

    clear_screen_cat();
    std::cout << "Nyan Cat Animation stopped." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    clear_screen_cat(); // Final clear before returning to shell
}

} // namespace CatAnimation 