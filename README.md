# Building

Required `cmake` only  
_(Installing it should also install `make` and `gcc` automatically)_

``` bash
sh ./setup.sh               # Download SDL and SDL_image source code
cmake -S . -B build/
cmake --build build/        # OR just run make in build/ directory
```

The game is compiled in `./build/simple_sdl`, run this.

# Game Idea

Player shoots bullets to opponents

# Project Structure

#### Root:
- `entry.c` -> Entry point of entire game
- `src/` -> All `.c` files
- `include/` -> All `.h` files

#### Directory `src/` & `include/`:
- `include/def.h` -> A defintion file containing constants to use
- `Vector` -> *~~TO FILL~~*
- `Control` -> *~~TO FILL~~*
- `Gameplay` -> *~~TO FILL~~*

#### Other files not in repo:
- `vendored/` -> `SDL3` and its `satellite libraries` source code is placed here after running `sh setup.sh`
- `build/` ->Build folder