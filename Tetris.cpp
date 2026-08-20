// Tetris.cpp
// CS 202 - Final Project
// controls: a/d to move, s to drop faster, w to rotate, x quits

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <windows.h>
using namespace std;

const int width = 10;
const int height = 20;
int board[height][width];

int score = 0;
bool gameOver = false;

// 4x4 grids for each rotation of each piece. took forever to type these out by hand
int shapes[7][4][4][4] = {
    {
        {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
        {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}},
        {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
        {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}}
    },
    {
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}}
    },
    {
        {{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{0,1,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}}
    },
    {
        {{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,1,0},{0,0,1,0},{0,0,0,0}},
        {{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,1,0},{0,0,1,0},{0,0,0,0}}
    },
    {
        {{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,0,1,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,0,1,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}}
    },
    {
        {{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{0,0,1,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0}}
    },
    {
        {{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{1,0,0,0},{0,0,0,0}},
        {{1,1,0,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}}
    }
};

int shapeNum;
int rotation;
int pieceX;
int pieceY;

char getKey() {
    if (_kbhit())
        return _getch();
    return 0;
}

// moves the cursor back to the top left instead of clearing the whole
// screen every frame. system("cls") every frame is what was causing
// the flickering/glitchy look
void resetCursor() {
    COORD pos = { 0, 0 };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(console, pos);
}

// turns off the blinking cursor so it doesn't show up in the middle
// of the board while the game redraws every frame
void hideCursor() {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(console, &info);
}

bool checkCollision(int shape, int rot, int x, int y) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (shapes[shape][rot][r][c] == 1) {
                int bx = x + c;
                int by = y + r;

                if (bx < 0 || bx >= width) return true;
                if (by >= height) return true;
                if (by >= 0 && board[by][bx] != 0) return true;
            }
        }
    }
    return false;
}

void newPiece() {
    shapeNum = rand() % 7;
    rotation = 0;
    pieceX = 3;
    pieceY = -1;

    if (checkCollision(shapeNum, rotation, pieceX, pieceY))
        gameOver = true;
}

void placePiece() {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (shapes[shapeNum][rotation][r][c] == 1) {
                int bx = pieceX + c;
                int by = pieceY + r;
                if (by >= 0 && by < height && bx >= 0 && bx < width)
                    board[by][bx] = 1;
            }
        }
    }
}

// clears full rows, kinda brute force but it works
void checkLines() {
    for (int row = height - 1; row >= 0; row--) {
        bool full = true;
        for (int c = 0; c < width; c++) {
            if (board[row][c] == 0) full = false;
        }

        if (full) {
            score += 100;

            for (int r = row; r > 0; r--) {
                for (int c = 0; c < width; c++)
                    board[r][c] = board[r - 1][c];
            }
            for (int c = 0; c < width; c++)
                board[0][c] = 0;

            row++; // row above just got shifted down into this spot, so check it again
        }
    }
}

void moveDown() {
    if (!checkCollision(shapeNum, rotation, pieceX, pieceY + 1)) {
        pieceY++;
    }
    else {
        placePiece();
        checkLines();
        newPiece();
    }
}

void moveLeft() {
    if (!checkCollision(shapeNum, rotation, pieceX - 1, pieceY))
        pieceX--;
}

void moveRight() {
    if (!checkCollision(shapeNum, rotation, pieceX + 1, pieceY))
        pieceX++;
}

void rotatePiece() {
    int newRot = (rotation + 1) % 4;

    if (!checkCollision(shapeNum, newRot, pieceX, pieceY))
        rotation = newRot;
}

void drawBoard() {
    int display[height][width];
    for (int r = 0; r < height; r++)
        for (int c = 0; c < width; c++)
            display[r][c] = board[r][c];

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (shapes[shapeNum][rotation][r][c] == 1) {
                int bx = pieceX + c;
                int by = pieceY + r;
                if (by >= 0 && by < height && bx >= 0 && bx < width)
                    display[by][bx] = 1;
            }
        }
    }

    resetCursor();

    cout << "TETRIS\nScore: " << score << "  \n\n";

    for (int r = 0; r < height; r++) {
        cout << "|";
        for (int c = 0; c < width; c++) {
            if (display[r][c] == 1)
                cout << "[]";
            else
                cout << "  ";
        }
        cout << "|\n";
    }

    for (int c = 0; c < width + 1; c++) cout << "--";
    cout << "\n";

    if (gameOver)
        cout << "GAME OVER, press x to quit          \n";
    else
        cout << "a/d move, s drop, w rotate, x quit\n";
}

int main() {
    // make sure the console window is tall enough to fit everything we print
    // each frame (score + board + status line). if the window is too short,
    // windows scrolls it to keep up, and since we always reset the cursor
    // to the very top of the buffer, that scrolling makes it look like
    // everything is jumping/bouncing around
    system("mode con: cols=40 lines=32");
    hideCursor();

    srand((unsigned int)time(0));

    for (int r = 0; r < height; r++)
        for (int c = 0; c < width; c++)
            board[r][c] = 0;

    newPiece();

    int frame = 0;
    bool playing = true;

    while (playing) {
        char key = getKey();
        if (key == 'x') playing = false;

        if (!gameOver) {
            if (key == 'a') moveLeft();
            else if (key == 'd') moveRight();
            else if (key == 's') moveDown();
            else if (key == 'w') rotatePiece();

            frame++;
            if (frame >= 25) { // adjust this to change fall speed
                moveDown();
                frame = 0;
            }
        }

        drawBoard();
        Sleep(20);
    }

    return 0;
}
