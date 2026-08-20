Tetris
A simple Tetris game built from scratch using C++ that runs in the windows console. No graphics just text.

How to run it:
Open Tetris.cpp in Visual Studio (or any Windows C++ compiler) and hit build/run. That's it, no extra setup or libraries to install.

Controls:
a = move left
d = move right
s = move down faster
w = rotate
x = quit

Tech Stack
C++
conio.h for reading key presses without needing to hit enter
windows.h for controlling the console cursor
Everything is in one file, no external libraries.

Problems I ran into while coding the game

1. Wrong headers for the platform:
I originally used POSIX headers like termios.h for reading keyboard input, since a lot of console-game tutorials use that approach. But those only exist on Linux/macOS. Windows doesn't have them, so I had to switch to Windows own version of this which is called conio.h. What makes conio.h useful is that it doesn't have the same job as reading a keypress without waiting for Enter, but it works through the windows API instead.

2. Screen Flicker:
My redraw loop was calling system(cls) every frame to clear the screen before drawing the next one. That works, but at 50 frames a second it means the console is being wiped blank which creates a visible flicker. I fixed it by not clearing the screen at all but instead I moved the cursor back to the top and just overwrite the previous frame's text with the new frames.

3. The stray cursor:
Once I stopped clearing the screen, I noticed something that looked like a glitch a small cursor shaped mark kept bouncing around the board. That turned out to be the console's own blinking text cursor, which was still visible and jumping to wherever the last character had been printed each frame. The fix was to explicitly hide the cursor using the windows Console API (SetConsoleCursorInfo) at the start of the program.

Conclusion:
Overall, most of the bugs weren't in the game logic itself, they were all about getting the console to redraw the screen correctly. That ended up being the trickiest part of the whole project.






