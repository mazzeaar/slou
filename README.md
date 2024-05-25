# SLOU
as of now this is only a move generator. i will implement eval and search someday.
it has a rudementary UCI, a good (maye?) testing framework and it should be bugfree.
i will cleanup movegen and try to improve the speed to 100Mnps. 
after this i will add [zobrist hashing](https://www.chessprogramming.org/Zobrist_Hashing), then [transposition tables](https://www.chessprogramming.org/Transposition_Table)
and after this eval and search. 

to implement eval and search cleanly i probably also have to cleanup the interfaces for the game and board.

## Speed
my first bugfree movegen was 3Mnps, so the speedup is already ~10x, only 2x to go.
- [Gigantua](https://github.com/Gigantua/Gigantua) achieves 2Bnps without multithreading, but it takes ~10m to compile. it does basically everything at compiletime.
- [Charon](https://github.com/RedBedHed/Charon) is at 350Mnps. it does a lot at compiletime but the movegen is still mostly at runtime.

my target is at least 100Mnps. matching Charon would be nice, although hard.

- 25.05.2024</br>
  Implemented Zobrist hashing and transposition tables to avoid calculating the same position twice.
  ```
  --------------------------------------
  Duration:  4782ms    
  Nodes:     311,143,032
  NPS:       65,058,685
  --------------------------------------
  ````

- 22.05.2024</br>
  Cleaned up the movegenerator and moved some choices to compiletime.
  ```
  --------------------------------------
  Duration:  7037ms    
  Nodes:     311,143,032
  NPS:       44,214,415
  --------------------------------------
  ```

## Todos
- [x] [bitboards + mailbox]([https://www.chessprogramming.org/Move_Generation](https://www.chessprogramming.org/Bitboards)) for efficiency
  - bitboards are now used for faster movegen
  - mailbox is used for faster piece lookup (bitboards required a for loop over all 64 bits of the 12 bitboard (worstcase: 768 iterations per lookup)
- [x] [bugfree movegen](https://www.chessprogramming.org/Move_Generation)
  - reformatted the move generation for each piece
  - standardized the format of generating a move
- [x] easily runnable tests
  - added `test.sh`, which runs perft tests for 152 position automatically. it also tracks [nps](https://www.chessprogramming.org/Nodes_per_Second#:~:text=a%20measure%20of%20the%20chess,different%20schemes%20of%20counting%20nodes.)
- [ ] try to implement a fully templated state, similar to [Gigantua](https://github.com/Gigantua/Gigantua)
  - this will need specific template instantiation for the possible states and probably also for the possible movegen functions
  - i dont quite understand yet how i can achieve this. templates are hard lol.
- [ ] clean up the code
  - [ ] add comments to everything thats not obvious
  - [ ] implementations always in `.hpp` or `.cpp` files, never in the header
  - [ ] standardized variable, class and function naming conventions
  - [ ] add debug asserts with clean errors
- [ ] [zobrist hashing](https://www.chessprogramming.org/Zobrist_Hashing)
  - is already implemented, but not used. the implementation is not bugfree yet
- [ ] [transposition tables](https://www.chessprogramming.org/Transposition_Table)
  - need a cleaner and fixed interface. this will only make sense when i know what architecture i use for move eval
- [ ] [move evaluation](https://www.chessprogramming.org/Evaluation)
- [ ] [UCI](https://www.chessprogramming.org/UCI), based on [this specification](https://gist.github.com/DOBRO/2592c6dad754ba67e6dcaec8c90165bf)
  - implement the basics to play with a [GUI](https://www.chessprogramming.org/GUI)
  - maybe implement commands by inheriting from a base command? this would simplify looking up and running a command and make it trivial to print them all for a `help` command?
- [ ] add support for multiple operating systems, as i have never done this and it would be a nice challenge

## Side note
three files will be overwritten when compiling for the first time. this is probably extremely bad practice, but im the only one that uses this anyway.
- `src/magic/bishop_magics.cpp`
- `src/magic/rook_magics.cpp`
- `src/magic/meta_magic.cpp`

this way i dont have to push all magic numbers to github, but they still only have to be calculated once.
