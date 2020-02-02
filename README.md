# c8
Chip-8 emulator & disassembler with ncurses

# Images
[Video](https://i.imgur.com/IwbRz1g.mp4)
<p float="left">
  <img src="https://i.imgur.com/534MML6.png" width="400" />
  <img src="https://i.imgur.com/EsQdKm7.png" width="400" />
  <img src="https://i.imgur.com/tkIl9f5.png" width="400" />
  <img src="https://i.imgur.com/Ypi1Ty9.png" width="400" />
</p>

# Using
## Command line
- Programs should be run using `c8.elf [-d] path/to/rom`.
- The `-d` flag controls emission of disassembled code.

## Controls
 - Controls are designed for an AZERTY keyboard.
 - If necessary, edit the switch/case in `src/window.hpp`.
 - Press space to pause/unpause.
 - Exit with Ctrl+C.

# Building
- Building requires the libraries ncurses (terminal interface) and SDL2 (audio).
- Simply run `make`, output with be located in `out`.
