# SLOU

This is my $(n+1)^\text{th}$ try at creating a chess engine.
Its slow, has no UCI (yet) and has some bugs.

Move generation almost works bug free. All [perft tests](https://www.chessprogramming.org/Perft_Results) run perfectly until perft 5, but some bugs emerge if i go deeper.

## Usage
### First build

`mkdir build && cd build && cmake .. && cmake --build .`

### Commands
- `--perft` runs detailed perft tests
- `--speedtest FEN DEPTH` to test speed for a given position
    - example: `bin/slou "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -" 4`
- `--debug FEN DEPTH` to get a detailed perft output
    - example: `bin/slou "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -" 4`

## Side note
Three files will be overwritten when compiling for the first time. 
- `src/magic/bishop_magics.cpp`
- `src/magic/rook_magics.cpp`
- `src/magic/meta_magic.cpp`

This way I dont have to push all magic numbers to github, but they still only have to be calculated once.