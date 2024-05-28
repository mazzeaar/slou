# SLOU
playable and winnable (as of now)

## Speed
- [Gigantua](https://github.com/Gigantua/Gigantua) achieves 2Bnps without multithreading, but it takes ~10m to compile. it does basically everything at compiletime.
- [Charon](https://github.com/RedBedHed/Charon) is at 350Mnps. it does a lot at compiletime but the movegen is still mostly at runtime.

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
- [ ] try to implement a fully templated state, similar to [Gigantua](https://github.com/Gigantua/Gigantua)
  - this will need specific template instantiation for the possible states and probably also for the possible movegen functions
  - i dont quite understand yet how i can achieve this. templates are hard lol.
- [ ] clean up the code
  - [ ] add comments to everything thats not obvious
  - [ ] implementations always in `.hpp` or `.cpp` files, never in the header
  - [ ] standardized variable, class and function naming conventions
  - [ ] add debug asserts with clean errors
- [ ] [UCI](https://www.chessprogramming.org/UCI), based on [this specification](https://gist.github.com/DOBRO/2592c6dad754ba67e6dcaec8c90165bf)
  - implement the basics to play with a [GUI](https://www.chessprogramming.org/GUI)
  - maybe implement commands by inheriting from a base command? this would simplify looking up and running a command and make it trivial to print them all for a `help` command?

## Side note
three files will be overwritten when compiling for the first time. this is probably extremely bad practice, but im the only one that uses this anyway.
- `src/magic/bishop_magics.cpp`
- `src/magic/rook_magics.cpp`
- `src/magic/meta_magic.cpp`

this way i dont have to push all magic numbers to github, but they still only have to be calculated once.
