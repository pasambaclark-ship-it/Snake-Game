#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//Snake game console based
// Platform-specific headers for keyboard detection & sleep
#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
    #define sleep_ms(x) Sleep(x)
#else
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    #define sleep_ms(x) usleep((x) * 1000)

    // Mimic Windows conio.h functions for Linux/macOS
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
        if(ch != EOF) {
            ungetc(ch, stdin);
            return 1;
        }
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

// Game Map Dimensions
#define WIDTH 20
#define HEIGHT 20

// Game State Variables
int gameOver;
int x, y, fruitX, fruitY, score;
int tailX[100], tailY[100];
int nTail;

// Movement directions
enum eDirection { STOP = 0, LEFT, RIGHT, UP, DOWN };
enum eDirection dir;

// Initialize game elements
void Setup() {
    gameOver = 0;
    dir = STOP;
    x = WIDTH / 2;
    y = HEIGHT / 2;
    
    // Seed and generate initial fruit position
    srand(time(NULL));
    fruitX = rand() % (WIDTH - 2) + 1;
    fruitY = rand() % (HEIGHT - 2) + 1;
    score = 0;
    nTail = 0;
}

// Render the map and entities in the console
void Draw() {
    // Clear screen cross-platform
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif

    // Top border
    for (int i = 0; i < WIDTH; i++) printf("#");
    printf("\n");

    // Body rows
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            // Left border
            if (j == 0 || j == WIDTH - 1) {
                printf("#");
            }
            // Snake Head
            else if (i == y && j == x) {
                printf("O");
            }
            // Fruit
            else if (i == fruitY && j == fruitX) {
                printf("F");
            }
            // Tail and Empty Spaces
            else {
                int isTail = 0;
                for (int k = 0; k < nTail; k++) {
                    if (tailX[k] == j && tailY[k] == i) {
                        printf("o");
                        isTail = 1;
                        break;
                    }
                }
                if (!isTail) printf(" ");
            }
        }
        printf("\n");
    }

    // Bottom border
    for (int i = 0; i < WIDTH; i++) printf("#");
    printf("\n");

    // Dashboard
    printf("Score: %d\n", score);
    printf("Controls: W (Up), A (Left), S (Down), D (Right) | X to Exit\n");
}

// Asynchronous keyboard input handler
void Input() {
    if (kbhit()) {
        switch (getch()) {
            case 'a': case 'A': if(dir != RIGHT) dir = LEFT;  break;
            case 'd': case 'D': if(dir != LEFT)  dir = RIGHT; break;
            case 'w': case 'W': if(dir != DOWN)  dir = UP;    break;
            case 's': case 'S': if(dir != UP)    dir = DOWN;  break;
            case 'x': case 'X': gameOver = 1;                 break;
        }
    }
}

// Game engine mechanics and boundary logic
void Logic() {
    int prevX = tailX[0];
    int prevY = tailY[0];
    int prev2X, prev2Y;
    tailX[0] = x;
    tailY[0] = y;

    // Shift tail positions forward
    for (int i = 1; i < nTail; i++) {
        prev2X = tailX[i];
        prev2Y = tailY[i];
        tailX[i] = prevX;
        tailY[i] = prevY;
        prevX = prev2X;
        prevY = prev2Y;
    }

    // Move Head
    switch (dir) {
        case LEFT:  x--; break;
        case RIGHT: x++; break;
        case UP:    y--; break;
        case DOWN:  y++; break;
        default: break;
    }

    // Wall collision checks
    if (x >= WIDTH - 1 || x < 1 || y >= HEIGHT || y < 0) {
        gameOver = 1;
    }

    // Tail collision checks
    for (int i = 0; i < nTail; i++) {
        if (tailX[i] == x && tailY[i] == y) {
            gameOver = 1;
        }
    }

    // Consuming fruit mechanic
    if (x == fruitX && y == fruitY) {
        score += 10;
        fruitX = rand() % (WIDTH - 2) + 1;
        fruitY = rand() % (HEIGHT - 2) + 1;
        nTail++;
    }
}

// Main game loop execution
int main() {
    Setup();
    while (!gameOver) {
        Draw();
        Input();
        Logic();
        sleep_ms(100); // Game tick interval speed
    }
    printf("\n*** GAME OVER ***\nFinal Score: %d\n", score);
    return 0;
}
