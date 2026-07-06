// tetris.cpp - console tetris in C++17
// colors, ghost piece, next piece preview, score, level, pause/restart
// controls: A/D move, S soft drop, W rotate, Space hard drop, P pause, Q quit

#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>

#if defined(_WIN32)
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

using namespace std;

const int BOARD_W = 10;
const int BOARD_H = 20;

int board[BOARD_H][BOARD_W]; // 0 = empty, otherwise a piece id (1-7)

// all 7 pieces, 4 rotations each, each rotation is a 4x4 grid
int pieces[7][4][4][4] = {
    // I
    {
        { {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0} },
        { {0,0,1,0}, {0,0,1,0}, {0,0,1,0}, {0,0,1,0} },
        { {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0} },
        { {0,0,1,0}, {0,0,1,0}, {0,0,1,0}, {0,0,1,0} }
    },
    // O
    {
        { {0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} }
    },
    // T
    {
        { {0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,1,0,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0} },
        { {0,0,0,0}, {1,1,1,0}, {0,1,0,0}, {0,0,0,0} },
        { {0,1,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0} }
    },
    // S
    {
        { {0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,0,0} },
        { {0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,0,0} }
    },
    // Z
    {
        { {1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,0,1,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0} },
        { {1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,0,1,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0} }
    },
    // J
    {
        { {1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,1,1,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0} },
        { {0,0,0,0}, {1,1,1,0}, {0,0,1,0}, {0,0,0,0} },
        { {0,1,0,0}, {0,1,0,0}, {1,1,0,0}, {0,0,0,0} }
    },
    // L
    {
        { {0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,0,0} },
        { {0,0,0,0}, {1,1,1,0}, {1,0,0,0}, {0,0,0,0} },
        { {1,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0} }
    }
};

int curType;     // which piece (0-6)
int curRotation;  // which rotation (0-3)
int curX, curY;   // top left of the piece's 4x4 box
int nextType;

long score = 0;
int level = 1;
int linesCleared = 0;
bool paused = false;
bool gameOver = false;
bool running = true;

// keyboard input stuff, different on windows vs linux/mac
#if defined(_WIN32)

void setupTerminal() {
    // conio.h just works on windows, nothing to set up
}

int readKey() {
    if (!_kbhit()) return 0;
    int c = _getch();
    if (c == 0 || c == 224) { // arrow keys, ignore them
        _getch();
        return 0;
    }
    return c;
}

#else

termios savedTermios;

void setupTerminal() {
    termios raw;
    tcgetattr(STDIN_FILENO, &savedTermios);
    raw = savedTermios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void restoreTerminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &savedTermios);
}

int readKey() {
    char c;
    int n = read(STDIN_FILENO, &c, 1);
    if (n <= 0) return 0;
    return (int)c;
}

#endif

// true if this piece at this spot hits a wall, the floor, or another block
bool collides(int type, int rotation, int x, int y) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (pieces[type][rotation][i][j] == 0) continue;
            int row = y + i;
            int col = x + j;
            if (col < 0 || col >= BOARD_W) return true;
            if (row >= BOARD_H) return true;
            if (row >= 0 && board[row][col] != 0) return true;
        }
    }
    return false;
}

void spawnPiece() {
    curType = nextType;
    nextType = rand() % 7;
    curRotation = 0;
    curX = 3;
    curY = -1;

    if (collides(curType, curRotation, curX, curY)) {
        gameOver = true;
    }
}

// adds the current piece onto the board
void lockPiece() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (pieces[curType][curRotation][i][j] == 1) {
                int row = curY + i;
                int col = curX + j;
                if (row >= 0 && row < BOARD_H && col >= 0 && col < BOARD_W) {
                    board[row][col] = curType + 1;
                }
            }
        }
    }
}

// removes any full rows and shifts everything above down
void clearLines() {
    int cleared = 0;
    for (int row = BOARD_H - 1; row >= 0; row--) {
        bool full = true;
        for (int col = 0; col < BOARD_W; col++) {
            if (board[row][col] == 0) { full = false; break; }
        }
        if (full) {
            cleared++;
            for (int r = row; r > 0; r--)
                for (int col = 0; col < BOARD_W; col++)
                    board[r][col] = board[r - 1][col];
            for (int col = 0; col < BOARD_W; col++)
                board[0][col] = 0;
            row++; // recheck this row since everything shifted down
        }
    }

    if (cleared > 0) {
        int points[] = { 0, 100, 300, 500, 800 };
        score += points[cleared] * level;
        linesCleared += cleared;
        level = 1 + linesCleared / 10;
    }
}

void moveLeftRight(int dx) {
    if (!collides(curType, curRotation, curX + dx, curY)) {
        curX += dx;
    }
}

void moveDown() {
    if (!collides(curType, curRotation, curX, curY + 1)) {
        curY += 1;
    }
    else {
        lockPiece();
        clearLines();
        spawnPiece();
    }
}

void rotatePiece() {
    int newRotation = (curRotation + 1) % 4;
    // try shifting left/right a bit in case the rotation hits a wall
    int kicks[] = { 0, -1, 1, -2, 2 };
    for (int k : kicks) {
        if (!collides(curType, newRotation, curX + k, curY)) {
            curRotation = newRotation;
            curX += k;
            return;
        }
    }
    // no room to rotate, just skip it
}

void hardDrop() {
    while (!collides(curType, curRotation, curX, curY + 1)) {
        curY++;
        score += 2;
    }
    lockPiece();
    clearLines();
    spawnPiece();
}

// where the piece would land if you hard dropped right now
int ghostY() {
    int y = curY;
    while (!collides(curType, curRotation, curX, y + 1)) y++;
    return y;
}

int fallDelayMs() {
    int delay = 700 - (level - 1) * 60;
    if (delay < 100) delay = 100;
    return delay;
}

string colorFor(int pieceId) {
    switch (pieceId) {
    case 1: return "\x1b[46m"; // I - cyan
    case 2: return "\x1b[43m"; // O - yellow
    case 3: return "\x1b[45m"; // T - magenta
    case 4: return "\x1b[42m"; // S - green
    case 5: return "\x1b[41m"; // Z - red
    case 6: return "\x1b[44m"; // J - blue
    case 7: return "\x1b[48;5;208m"; // L - orange
    default: return "\x1b[47m";
    }
}
const string RESET = "\x1b[0m";

void draw() {
    // copy the board and add the ghost + falling piece on top
    int grid[BOARD_H][BOARD_W];
    for (int i = 0; i < BOARD_H; i++)
        for (int j = 0; j < BOARD_W; j++)
            grid[i][j] = board[i][j];

    int gy = ghostY();
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (pieces[curType][curRotation][i][j] == 1) {
                int row = gy + i, col = curX + j;
                if (row >= 0 && row < BOARD_H && col >= 0 && col < BOARD_W && grid[row][col] == 0)
                    grid[row][col] = -1; // -1 = ghost
            }

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (pieces[curType][curRotation][i][j] == 1) {
                int row = curY + i, col = curX + j;
                if (row >= 0 && row < BOARD_H && col >= 0 && col < BOARD_W)
                    grid[row][col] = curType + 1;
            }

    string out;
    out += "\x1b[H"; // move cursor to top left instead of clearing, less flicker
    out += "\x1b[1;36mT E T R I S\x1b[0m\n\n";

    out += "  +";
    for (int j = 0; j < BOARD_W; j++) out += "--";
    out += "+   NEXT\n";

    for (int row = 0; row < BOARD_H; row++) {
        out += "  |";
        for (int col = 0; col < BOARD_W; col++) {
            int cell = grid[row][col];
            if (cell == 0) out += "  ";
            else if (cell == -1) out += "\x1b[2m..\x1b[0m";
            else { out += colorFor(cell); out += "  "; out += RESET; }
        }
        out += "|";

        // side panel: next piece preview + stats + controls
        if (row == 1) {
            out += "   +--------+";
        }
        else if (row >= 2 && row <= 5) {
            out += "   |";
            for (int j = 0; j < 4; j++) {
                if (pieces[nextType][0][row - 2][j]) {
                    out += colorFor(nextType + 1); out += "  "; out += RESET;
                }
                else {
                    out += "  ";
                }
            }
            out += "|";
        }
        else if (row == 6) {
            out += "   +--------+";
        }
        else if (row == 8) {
            out += "   Score: " + to_string(score);
        }
        else if (row == 9) {
            out += "   Level: " + to_string(level);
        }
        else if (row == 10) {
            out += "   Lines: " + to_string(linesCleared);
        }
        else if (row == 12) {
            out += "   A/D move";
        }
        else if (row == 13) {
            out += "   W rotate";
        }
        else if (row == 14) {
            out += "   S soft drop";
        }
        else if (row == 15) {
            out += "   Space hard drop";
        }
        else if (row == 16) {
            out += "   P pause  Q quit";
        }

        out += "\n";
    }

    out += "  +";
    for (int j = 0; j < BOARD_W; j++) out += "--";
    out += "+\n";

    if (gameOver) out += "\n\x1b[1;31m  GAME OVER -- press R to restart, Q to quit\x1b[0m\n";
    else if (paused) out += "\n\x1b[1;33m  PAUSED -- press P to resume\x1b[0m\n";
    else out += "\n";

    out += "\x1b[J"; // clear any leftovers from a taller previous frame

    cout << out;
    cout.flush();
}

void restart() {
    for (int i = 0; i < BOARD_H; i++)
        for (int j = 0; j < BOARD_W; j++)
            board[i][j] = 0;
    score = 0;
    linesCleared = 0;
    level = 1;
    gameOver = false;
    paused = false;
    nextType = rand() % 7;
    spawnPiece();
}

void handleInput(int key) {
    if (key == 0) return;
    if (key == 'q' || key == 'Q') { running = false; return; }
    if (key == 'p' || key == 'P') { paused = !paused; return; }
    if (gameOver) {
        if (key == 'r' || key == 'R') restart();
        return;
    }
    if (paused) return;

    if (key == 'a' || key == 'A') moveLeftRight(-1);
    else if (key == 'd' || key == 'D') moveLeftRight(1);
    else if (key == 's' || key == 'S') { moveDown(); score += 1; }
    else if (key == 'w' || key == 'W') rotatePiece();
    else if (key == ' ') hardDrop();
}

int main() {
    srand((unsigned)time(0));
    setupTerminal();
    cout << "\x1b[2J\x1b[H\x1b[?25l"; // clear screen, hide cursor

    nextType = rand() % 7;
    spawnPiece();

    auto lastFall = chrono::steady_clock::now();

    while (running) {
        int key = readKey();
        handleInput(key);

        // piece falls on its own every so often
        if (!paused && !gameOver) {
            auto now = chrono::steady_clock::now();
            long elapsed = chrono::duration_cast<chrono::milliseconds>(now - lastFall).count();
            if (elapsed >= fallDelayMs()) {
                moveDown();
                lastFall = now;
            }
        }

        draw();
        this_thread::sleep_for(chrono::milliseconds(16));
    }

    cout << "\x1b[?25h\x1b[0m\n"; // show cursor again before exiting
#if !defined(_WIN32)
    restoreTerminal();
#endif

    return 0;
}