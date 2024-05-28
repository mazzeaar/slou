#pragma once

#include <string>
#include <iostream>

#include "config.h"
#include "board/board.h"
#include "move.h"
#include "move_generator/move_generation.h"
#include "eval.h"
#include "game.h"

class CommandManager {
private:
    std::string _fen = STARTPOS;
    Game game;

    std::string& to_lower(std::string& s) { for ( char c : s ) { c = std::tolower(c); } return s; }
    Move makeMoveFromString(const std::string& moveStr, const Board& board);

public:
    CommandManager()
    {
        game = Game();
    }

    void parseCommand();
};
