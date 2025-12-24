/*
  A simple recreation of Minesweeper on the command line. Type the row number + column letter + action.
  Use . to reveal (e.g., 5a.) or ! to toggle a flag (e.g., 5a!).
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MINE -1

char *numcolors[] = {
    "\033[0;90m",
    "\033[0;94m",
    "\033[32m",
    "\033[0;91m",
    "\033[34m",
    "\033[31m",
    "\033[36m",
    "", // normally black
    "\033[0;95m"
};

typedef struct {
    int width, height;
    char **board;
    char **revealed;
    char **flagged;
    int mines;
    int numRevealed;
} MSBoard;

#include <sys/time.h>

long long get_epoch_millis(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000LL + tv.tv_usec / 1000LL;
}

void refreshMineCounts(MSBoard *msb) {
    // for each non-mine spot, put the number of mines that surround it
    for (int y = 0; y < msb->height; y++) {
        for (int x = 0; x < msb->width; x++) {
            if (msb->board[y][x] == MINE) continue;

            int surroundingMines = 0;
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    if (i == 0 && j == 0) continue;
                    if (i + y < 0 || i + y >= msb->height) continue;
                    if (j + x < 0 || j + x >= msb->width) continue;
                    if (msb->board[i+y][j+x] == MINE) surroundingMines++;
                }
            }
            msb->board[y][x] = surroundingMines;
        }
    }
}

void revealBoard(MSBoard *msb) {
    for (int i = 0; i < msb->height; i++) {
        for (int j = 0; j < msb->width; j++) {
            msb->revealed[i][j] = 1;
        }
    }
}

MSBoard createBoard(int width, int height, int mines) {
    // create board
    MSBoard msb = {
        .width = width,
        .height = height,
        .mines = mines,
        .board = malloc(sizeof(char*)*height),
        .revealed = malloc(sizeof(char*)*height),
        .flagged = malloc(sizeof(char*)*height),
        .numRevealed = 0
    };

    for (int i = 0; i < height; i++) {
        msb.board[i] = calloc(width, 1);
        msb.revealed[i] = calloc(width, 1);
        msb.flagged[i] = calloc(width, 1);
    }

    // put mines
    for (int i = 0; i < msb.mines; i++) {
        int x, y;
        do {
            x = rand() % width;
            y = rand() % height;
        } while (msb.board[y][x] == MINE);
        msb.board[y][x] = MINE; // -1 = mine
    }

    refreshMineCounts(&msb);

    return msb;
}

int revealMine(MSBoard *msb, int x, int y) {
    msb->revealed[y][x] = 1;
    msb->numRevealed++;
    if (msb->board[y][x] == MINE) return 1;

    if (msb->board[y][x] > 0) return 0; 

    // Flood-fill reveal all of the connecting non-mine blocks
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;
            if (i + y < 0 || i + y >= msb->height) continue;
            if (j + x < 0 || j + x >= msb->width) continue;
            if (msb->revealed[y+i][x+j]) continue;
            revealMine(msb, x+j, y+i);
        }
    }

    return 0;
}

void printBoard(MSBoard msb) {
    printf("   | ");
    for (int i = 0; i < msb.width; i++) {
        printf("%c ", i >= 26 ? 'A' + i-26 : 'a' + i);
    }
    printf("\n---+");
    for (int i = 0; i < msb.width; i++) {
        printf("--");
    }
    printf("\n");

    for (int y = 0; y < msb.height; y++) {
        for (int x =  0; x < msb.width; x++) {
            if (x == 0) {
                printf("%2i | ", y+1);
            }
            if (msb.flagged[y][x]) {
               printf("\033[31mF\033[0m ");
            } else if (msb.revealed[y][x]) {
                if (msb.board[y][x] == -1) printf("M ");
                else printf("%s%i\033[0m ", numcolors[msb.board[y][x]], msb.board[y][x]);
            } else {
                printf("* ");
            }
        }
        printf("\n");
    }
}

#define SINCE ((float)(get_epoch_millis() - startTime) / 1000.0)

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <rows> <columns> <mines>\n", argv[0]);
        printf("Example: %s 16 30 99\n", argv[0]);
        return 1;
    }

    int height = atoi(argv[1]);
    int width = atoi(argv[2]);
    int mines = atoi(argv[3]);

    if (height <= 0 || width <= 0 || mines <= 0) {
        printf("Error: rows, columns, and mines must be positive integers\n");
        return 1;
    }

    if (mines >= height * width) {
        printf("Error: number of mines must be less than total cells (%d)\n", height * width);
        return 1;
    }

    FILE *histfp = fopen("/tmp/mshistory.txt", "w");
    if (!histfp) {
        printf("Failed to open history file");
    }

    srand(time(0));
    MSBoard msb = createBoard(width, height, mines);
    
    int numCells = msb.width * msb.height;
    int revealedGoal = numCells - msb.mines;
    int gameLost = 0;
    int firstMove = 1;

    long long startTime = get_epoch_millis();

    while (msb.numRevealed < revealedGoal) {
        printf("\033[2J\033[H");
        printBoard(msb);
        int x, row;
        while (1) {
            printf("Enter a slot: [row][column][action] (. to reveal, ! to flag):\n");
            char col, action;
            int scanResult = scanf("%d%c%c", &row, &col, &action);
            if (scanResult != 3) {
                printf("Input format: [row number][column letter][. or !]\n"); 
                while(getchar() != '\n');
                continue;
            }
            while(getchar() != '\n');  // clear buffer
            
            if (action != '.' && action != '!') {
                printf("Action must be . (reveal) or ! (flag)\n");
                continue;
            }
            
            if (row < 1 || row > msb.height) {
                printf("Invalid row number.\n");
                continue;
            }
            if (col >= 'a' && col <= 'z') {
                x = col - 'a';
            } else if (col >= 'A' && col <= 'Z') {
                x = col - 'A' + 26;
            } else {
                printf("Invalid column letter.\n");
                continue;
            }
            if (x < 0 || x >= msb.width) {
                printf("Column out of range.\n");
                continue;
            }
            if (msb.revealed[row-1][x]) {
                printf("Cell already revealed.\n");
                continue;
            }
            
            if (action == '!') {
                msb.flagged[row-1][x] = !msb.flagged[row-1][x];
                if (msb.flagged[row-1][x]) {
                    fprintf(histfp, "%.3f Flag: %i%c\n", SINCE, row, col);
                } else {
                    fprintf(histfp, "%.3f Unflag: %i%c\n", SINCE, row, col);
                }
                break;
            }
            
            if (msb.flagged[row-1][x]) {
                printf("Cell is flagged. Unflag first.\n");
                continue;
            }

            fprintf(histfp, "%.3f Move: %i%c\n", SINCE, row, col);
            
            int result = revealMine(&msb, x, row-1);
            if (result) {
                if (firstMove) {
                    // Clear the clicked cell
                    msb.board[row-1][x] = 0;
                    
                    // Clear and relocate all surrounding mines
                    for (int i = -1; i <= 1; i++) {
                        for (int j = -1; j <= 1; j++) {
                            if (i == 0 && j == 0) continue;
                            int ny = row-1 + i;
                            int nx = x + j;
                            if (ny < 0 || ny >= msb.height) continue;
                            if (nx < 0 || nx >= msb.width) continue;
                            
                            // If this cell was a mine, relocate it
                            if (msb.board[ny][nx] == MINE) {
                                msb.board[ny][nx] = 0;
                                
                                // Find new location outside the 3x3 area
                                int Nx, Ny;
                                do {
                                    Nx = rand() % msb.width;
                                    Ny = rand() % msb.height;
                                } while (msb.board[Ny][Nx] == MINE || 
                                         (Nx >= x-1 && Nx <= x+1 && Ny >= row-2 && Ny <= row));
                                msb.board[Ny][Nx] = MINE;
                            }
                        }
                    }

                    refreshMineCounts(&msb);
                    revealMine(&msb, x, row-1);
                } else {
                    revealBoard(&msb);
                    printf("\033[2J\033[H");
                    printBoard(msb);
                    fprintf(histfp, "%.3f Mine triggered at %i%c\n", SINCE, row, col);
                    printf("Mine triggered, game over!\n");
                    gameLost = 1;
                }
            }
            break;
        }
        firstMove = 0;
        if (gameLost) break;
    }

    if (!gameLost) {
        revealBoard(&msb);
        printBoard(msb);
        fprintf(histfp, "%.3f Board cleared\n", SINCE);
        printf("Board cleared, you won!\n");
    }
    printf("Game history output to /tmp/mshistory.txt\n");
    fclose(histfp);
}
