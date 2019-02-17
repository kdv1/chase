#### Port of Chase NES game, originally written by Shiru (Public Domain)

This game initially was written in the C programming language, which makes\
porting to PC is almost trivial with minimum changes.

Now game works as follow:\
&nbsp; &nbsp; &nbsp; &nbsp; For music and sound effects it uses SDL2_Mixer instead of APU.\
&nbsp; &nbsp; &nbsp; &nbsp; For graphics - [P3](https://github.com/kdv1/p3.git), simple PPU emulator.\
&nbsp; &nbsp; &nbsp; &nbsp; For displaying and input - SDL2.\
&nbsp; &nbsp; &nbsp; &nbsp; For low-level things - minimal port of neslib.\
&nbsp; &nbsp; &nbsp; &nbsp; For stable fixed 60 fps - vsync

Purpose of this project is to debug P3 and create working demo.

Controls (Keyboard):\
&nbsp; &nbsp; &nbsp; &nbsp; Up: W\
&nbsp; &nbsp; &nbsp; &nbsp; Down: S\
&nbsp; &nbsp; &nbsp; &nbsp; Left: A\
&nbsp; &nbsp; &nbsp; &nbsp; Right: D\
&nbsp; &nbsp; &nbsp; &nbsp; Select: Space\
&nbsp; &nbsp; &nbsp; &nbsp; Start: Enter\
&nbsp; &nbsp; &nbsp; &nbsp; A (not used in game): L\
&nbsp; &nbsp; &nbsp; &nbsp; B (not used in game): K\
Toggles:\
&nbsp; &nbsp; &nbsp; &nbsp; Red color tint: R\
&nbsp; &nbsp; &nbsp; &nbsp; Green color tint: G\
&nbsp; &nbsp; &nbsp; &nbsp; Blue color tint: B\
&nbsp; &nbsp; &nbsp; &nbsp; Grayscale mode: C\
&nbsp; &nbsp; &nbsp; &nbsp; Wave raster effect: E\
&nbsp; &nbsp; &nbsp; &nbsp; Mapper test: M\
&nbsp; &nbsp; &nbsp; &nbsp; Autofix y position of sprites: Y

Building with Visual Studio 2008:\
&nbsp; &nbsp; &nbsp; &nbsp; Place dependencies to 'lib' folder (SDL2, SDL2_Mixer, P3 header file\
&nbsp; &nbsp; &nbsp; &nbsp; with .lib files), open and build solution file in 'build/vs2008'.
