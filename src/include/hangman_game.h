#pragma once

#include <string>
#include <vector>
#include <iostream> // For cout in play_hangman_game, can be removed if that func moves to cpp

namespace Hangman {

void print_hangman(int incorrect_guesses);
std::string choose_random_word();
void display_game_state(const std::string& word_to_guess, const std::string& guessed_letters, int incorrect_guesses);
bool process_guess(char guess, const std::string& word_to_guess, std::string& guessed_letters, int& incorrect_guesses, std::vector<char>& tried_letters);
bool is_word_guessed(const std::string& word_to_guess, const std::string& guessed_letters);
bool is_game_over(int incorrect_guesses, const std::string& word_to_guess, const std::string& guessed_letters);

// Main function to launch and play the game
void play_hangman_game();

const int MAX_INCORRECT_GUESSES = 7; // Or 6 for a more traditional hangman

} // namespace Hangman 