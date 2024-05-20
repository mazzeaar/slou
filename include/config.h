#pragma once

#define TODO            std::cerr << RED << "TODO: " << RESET
#define STARTPOS        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define ENABLE_LOGGER   

#define ENABLE_DEBUG    0

// as testing for checks and mates is quite expensive i have added an option to disable them
#ifndef SIMPLE_TEST
#define SIMPLE_TEST     1
#endif

// PRINT STUFF
#define COL_SPACING     18
#define TABLE_WIDTH     (5 * COL_SPACING)
#define FAT_LINE        std::string(TABLE_WIDTH, '=')
#define THIN_LINE       std::string(TABLE_WIDTH, '-')
#define DOTTED_LINE     std::string(TABLE_WIDTH, '.')

// ANSI color codes
#ifdef NO_COLOR_OUTPUT
#define RESET           ""
#define RED             ""
#define GREEN           ""
#define MAGENTA         ""
#define BLUE            ""
#define YELLOW          ""
#define WHITE           ""
#define WHITE_BG        ""
#else
#define RESET           "\e[0m"
#define RED             "\e[0;31m"
#define GREEN           "\e[0;32m"
#define MAGENTA         "\e[0;35m"
#define BLUE            "\033[34m"
#define YELLOW          "\033[33m"
#define WHITE           "\033[37m"
#define WHITE_BG        "\e[47m"
#endif
