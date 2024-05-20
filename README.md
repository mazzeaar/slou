# SLOU

This is my $(n+1)^\text{th}$ try at creating a chess engine.
Its slow, has no UCI (yet) and has some bugs.

All [perft tests](https://www.chessprogramming.org/Perft_Results) (+127 other positions) run bugfree now.

## Speed
Movegen is bugfree now.
- 04.05.2024: ~2Mnps
- 06.05.2024: ~3Mnps
- 20.05.2024: ~45Mnps

## Todos
- [ ] a cleaner zobrist implementation
- [ ] transposition tables for lookup
- [ ] move eval lol
- [ ] multithreaded uci implementation for selfplay

## Side note
Three files will be overwritten when compiling for the first time. 
- `src/magic/bishop_magics.cpp`
- `src/magic/rook_magics.cpp`
- `src/magic/meta_magic.cpp`

This way I dont have to push all magic numbers to github, but they still only have to be calculated once.
