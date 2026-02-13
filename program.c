
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// --- OS DETECTION BLOCK ---
#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
    #define CLEAR "cls"
    #define SLEEP(ms) Sleep(ms)
#else
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    #define CLEAR "clear"
    #define SLEEP(ms) usleep(ms * 1000) // Convert ms to microseconds
#endif

// ANSI Colors
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define RESET "\x1B[0m"

// Game Variables
int width = 40, height = 20;
int x, y, fruitX, fruitY, score, gameOver;
enum eDirection { STOP = 0, LEFT, RIGHT, UP, DOWN } dir;
int tailX[100], tailY[100], nTail;

// --- CROSS-PLATFORM INPUT LOGIC ---
#ifndef _WIN32
int kbhit(void) {
    struct termios oldt, newt;
    int ch, oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if(ch != EOF) { ungetc(ch, stdin); return 1; }
    return 0;
}
int getch(void) {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif

void Setup() {
    gameOver = 0;
    dir = STOP;
    x = width / 2;
    y = height / 2;
    fruitX = rand() % width;
    fruitY = rand() % height;
    score = 0;
    nTail = 0;
}

void Draw() {
    printf("\033[H"); // Cursor to top
    printf(YEL);
    for (int i = 0; i < width + 2; i++) printf("#");
    printf(RESET "\n");

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (j == 0) printf(YEL "#" RESET); 
            if (i == y && j == x) printf(GRN "O" RESET);
            else if (i == fruitY && j == fruitX) printf(RED "F" RESET);
            else {
                int isTail = 0;
                for (int k = 0; k < nTail; k++) {
                    if (tailX[k] == j && tailY[k] == i) {
                        printf(GRN "o" RESET);
                        isTail = 1;
                        break;
                    }
                }
                if (!isTail) printf(" ");
            }
            if (j == width - 1) printf(YEL "#" RESET); 
        }
        printf("\n");
    }
    printf(YEL);
    for (int i = 0; i < width + 2; i++) printf("#");
    printf(RESET "\nScore: %d | Use WASD | X to Quit\n", score);
}

void Input() {
    #ifdef _WIN32
    if (_kbhit()) {
        switch (_getch()) {
    #else
    if (kbhit()) {
        switch (getch()) {
    #endif
            case 'a': if(dir != RIGHT) dir = LEFT; break;
            case 'd': if(dir != LEFT) dir = RIGHT; break;
            case 'w': if(dir != DOWN) dir = UP; break;
            case 's': if(dir != UP) dir = DOWN; break;
            case 'x': gameOver = 1; break;
        }
    }
}

void Logic() {
    if (dir == STOP) return;
    int prevX = tailX[0], prevY = tailY[0], prev2X, prev2Y;
    tailX[0] = x; tailY[0] = y;
    for (int i = 1; i < nTail; i++) {
        prev2X = tailX[i]; prev2Y = tailY[i];
        tailX[i] = prevX; tailY[i] = prevY;
        prevX = prev2X; prevY = prev2Y;
    }
    switch (dir) {
        case LEFT: x--; break;
        case RIGHT: x++; break;
        case UP: y--; break;
        case DOWN: y++; break;
        default: break;
    }
    if (x < 0 || x >= width || y < 0 || y >= height) gameOver = 1;
    for (int i = 0; i < nTail; i++) if (tailX[i] == x && tailY[i] == y) gameOver = 1;
    if (x == fruitX && y == fruitY) {
        score += 10;
        fruitX = rand() % width; fruitY = rand() % height;
        nTail++;
    }
}

int main() {
    char choice;
    srand(time(NULL));
    printf("\e[?25l"); // Hide cursor

    do {
        Setup();
        while (!gameOver) {
            Draw();
            Input();
            Logic();
            // set the speed
            int d = 200 - (score / 2); 

            //Speed Limit
            if (d < 120) {
                d = 120;
            }

            SLEEP(d);
        }
        printf(RED "\n  GAME OVER! Score: %d\n" RESET, score);
        printf("  Play again? (y/n): ");
        printf("\e[?25h"); // Show cursor
        scanf(" %c", &choice);
        if (choice == 'y' || choice == 'Y') printf("\e[?25l");
    } while (choice == 'y' || choice == 'Y');

    return 0;
}
