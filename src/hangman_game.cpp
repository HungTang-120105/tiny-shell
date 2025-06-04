#include "hangman_game.h"
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm> // For std::transform and std::find
#include <cctype>    // For std::tolower
#include <ctime>     // For std::time for random seed
#include <limits>    // For std::numeric_limits
#include <windows.h> // For system("cls") and Sleep
#include <conio.h>   // ADDED: For _getch, _kbhit (Windows)

namespace Hangman {

// Word list - can be expanded or loaded from a file later
const std::vector<std::string> words = {
    "apple", "banana", "orange", "grape", "strawberry", "watermelon",
    "computer", "keyboard", "mouse", "monitor", "programming", "tinyhsell",
    "hangman", "puzzle", "game", "summer", "winter", "spring", "autumn",
    "mountain", "river", "ocean", "forest", "desert", "flower"
};

void clear_screen_hangman() {
#ifdef _WIN32
    system("cls");
#else
    // Assume POSIX
    system("clear");
#endif
}

// Function to print the hangman based on incorrect guesses
void print_hangman(int incorrect_guesses) {
    std::cout << "  +---+\n";
    std::cout << "  |   |\n";
    std::cout << "  |   " << (incorrect_guesses >= 1 ? "O" : "") << "\n";
    std::cout << "  |  " << (incorrect_guesses >= 3 ? "/" : " ") << (incorrect_guesses >= 2 ? "|" : " ") << (incorrect_guesses >= 4 ? "\\" : "") << "\n";
    std::cout << "  |  " << (incorrect_guesses >= 5 ? "/" : " ") << " " << (incorrect_guesses >= 6 ? "\\" : "") << "\n";
    std::cout << "  |\n";
    std::cout << "=========\n";
}

std::string choose_random_word() {
    static std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
    std::uniform_int_distribution<size_t> dist(0, words.size() - 1);
    return words[dist(rng)];
}

void display_game_state(const std::string& word_to_guess, const std::string& guessed_letters, int incorrect_guesses, const std::vector<char>& tried_letters) {
    print_hangman(incorrect_guesses);
    std::cout << "\nWord: ";
    for (char letter : word_to_guess) {
        if (guessed_letters.find(letter) != std::string::npos) {
            std::cout << letter << " ";
        } else {
            std::cout << "_ ";
        }
    }
    std::cout << "\n\nIncorrect guesses remaining: " << MAX_INCORRECT_GUESSES - incorrect_guesses << "\n";
    std::cout << "Tried letters: ";
    for(char c : tried_letters) {
        std::cout << c << " ";
    }
    std::cout << "\n";
}

// Returns true if the guess was a new, correct letter, false otherwise (or if already guessed)
bool process_guess(char guess, const std::string& word_to_guess, std::string& guessed_letters, int& incorrect_guesses, std::vector<char>& tried_letters) {
    guess = std::tolower(guess);
    if (!std::isalpha(guess)) {
        std::cout << "Please enter a letter." << std::endl;
        Sleep(1000);
        return false; // Not a valid guess type
    }

    if (std::find(tried_letters.begin(), tried_letters.end(), guess) != tried_letters.end()) {
        std::cout << "You already tried '" << guess << "'." << std::endl;
        Sleep(1000);
        return false; // Already tried this letter
    }

    tried_letters.push_back(guess);
    std::sort(tried_letters.begin(), tried_letters.end()); // Keep tried letters sorted

    if (word_to_guess.find(guess) != std::string::npos) {
        guessed_letters += guess;
        return true; // Correct guess
    } else {
        incorrect_guesses++;
        std::cout << "'" << guess << "' is not in the word." << std::endl;
        Sleep(1000);
        return false; // Incorrect guess
    }
}

bool is_word_guessed(const std::string& word_to_guess, const std::string& guessed_letters) {
    for (char letter : word_to_guess) {
        if (guessed_letters.find(letter) == std::string::npos) {
            return false;
        }
    }
    return true;
}

bool is_game_over(int incorrect_guesses, const std::string& word_to_guess, const std::string& guessed_letters) {
    return incorrect_guesses >= MAX_INCORRECT_GUESSES || is_word_guessed(word_to_guess, guessed_letters);
}

void play_hangman_game() {
    std::string word_to_guess = choose_random_word();
    std::string guessed_letters = ""; // Stores correctly guessed unique letters
    std::vector<char> tried_letters;  // Stores all unique letters tried by the player
    int incorrect_guesses = 0;
    char guess_input;

    clear_screen_hangman();
    std::cout << "Welcome to Hangman!" << std::endl;
    std::cout << "Try to guess the word. You have " << MAX_INCORRECT_GUESSES << " incorrect attempts allowed." << std::endl;
    Sleep(2000);

    while (!is_game_over(incorrect_guesses, word_to_guess, guessed_letters)) {
        clear_screen_hangman();
        display_game_state(word_to_guess, guessed_letters, incorrect_guesses, tried_letters);
        
        std::cout << "\nEnter your guess (a letter): ";
        std::cin >> guess_input;

        // Clear the input buffer in case of multiple characters or invalid input
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 

        process_guess(guess_input, word_to_guess, guessed_letters, incorrect_guesses, tried_letters);
    }

    clear_screen_hangman();
    display_game_state(word_to_guess, guessed_letters, incorrect_guesses, tried_letters); // Show final state

    if (is_word_guessed(word_to_guess, guessed_letters)) {
        std::cout << "\nCongratulations! You guessed the word: " << word_to_guess << std::endl;
    } else {
        std::cout << "\nGame Over! The word was: " << word_to_guess << std::endl;
        print_hangman(MAX_INCORRECT_GUESSES); // Show full hangman on loss
    }
    std::cout << "\nPress any key to return to the shell..." << std::endl;
    // Use _getch() or similar for cleaner input without needing Enter, but cin is fine for now
    // For Windows, _getch() from conio.h is better
    #ifdef _WIN32
        _getch();
    #else
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear potential leftover newline
        std::cin.get(); 
    #endif
    clear_screen_hangman();
}

} // namespace Hangman 