#pragma once
//──────────────────────────────────────────────────────────────────────────────
// ConsoleUtils.hpp ― tiny helper routines for nicer terminal UX
//
//   • strip_trailing_newlines   → remove \n / \r so we can reuse a line
//   • erase_current_line        → clear the line we’re on
//   • erase_previous_line       → clear the line above (used to hide local echo)
//
// These are header‑only (inline) because they’re tiny and have no state.
//
// 2025‑07‑23
//──────────────────────────────────────────────────────────────────────────────
#include <iostream>
#include <string>

namespace con {

/**
 * @brief Remove all trailing CR/LF characters from a string.
 *
 * Useful after `std::getline` so we can append our own newline, or when
 * re‑using the raw name as a prompt prefix.
 */
inline void strip_trailing_newlines(std::string& s)
{
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
        s.pop_back();
}

/**
 * @brief Erase the contents of the *current* console line.
 *
 * Uses the ANSI escape sequence “Erase in line” (`ESC[2K`) preceded
 * by a carriage return.
 */
inline void erase_current_line()
{
    std::cout << "\r\x1B[2K" << std::flush;
}

/**
 * @brief Erase the *previous* console line (the one right above the cursor).
 *
 * Moves the cursor up one line (`ESC[1A`), then re‑uses erase_current_line().
 */
inline void erase_previous_line()
{
    std::cout << "\x1B[1A\r\x1B[2K" << std::flush;
}

} // namespace con
