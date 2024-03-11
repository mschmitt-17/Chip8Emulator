# Chip8-Emulator

Created referencing http://devernay.free.fr/hacks/chip8/C8TECH10.HTM for Chip8 specs and https://austinmorlan.com/posts/chip8_emulator/ for necessary SDL commands. To run, compile with the flag `sdl2-config --cflags --libs` and 3 arguments: videoScale (actual pixels to emulator pixels), cycleDelay (milliseconds between Chip8 operations), and path to chip8 ROM (ROMs can be found at https://github.com/dmatlack/chip8/tree/master/roms)
