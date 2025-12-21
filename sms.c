/*

    "Stateless" Minesweeper

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MINE -1

typedef struct {
    int width, height;
    char **board;
    char **revealed;
    char **flagged;
    int mines;
    int numRevealed;
} MSBoard;

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

void revealAll(MSBoard *msb) {
    for (int i = 0; i < msb->height; i++) {
        for (int j = 0; j < msb->width; j++) {
            msb->revealed[i][j] = 1;
        }
    }
}

void clearFirstMoveArea(MSBoard *msb, int x, int y) {
    // Check all cells in 3x3 grid centered on (x,y)
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int checkY = y + i;
            int checkX = x + j;
            
            // Skip if out of bounds
            if (checkY < 0 || checkY >= msb->height) continue;
            if (checkX < 0 || checkX >= msb->width) continue;
            
            // If there's a mine here, relocate it
            if (msb->board[checkY][checkX] == MINE) {
                msb->board[checkY][checkX] = 0; // Remove mine
                
                // Find a new random location outside the 3x3 grid
                int newX, newY;
                int attempts = 0;
                do {
                    newX = rand() % msb->width;
                    newY = rand() % msb->height;
                    attempts++;
                    
                    // Check if this position is outside the 3x3 grid and not already a mine
                    int outsideGrid = (newY < y - 1 || newY > y + 1 || 
                                      newX < x - 1 || newX > x + 1);
                    int notMine = (msb->board[newY][newX] != MINE);
                    
                    if (outsideGrid && notMine) break;
                    
                    // Safety check to prevent infinite loop
                    if (attempts > 1000) return;
                } while (1);
                
                msb->board[newY][newX] = MINE;
            }
        }
    }
    
    // Recalculate all mine counts after relocating
    refreshMineCounts(msb);
}

int revealMine(MSBoard *msb, int x, int y) {
    // If this is the first move, ensure the 3x3 area is clear
    if (msb->numRevealed == 0) {
        clearFirstMoveArea(msb, x, y);
    }
    
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
               //printf("\033[31mF\033[0m ");
               printf("F ");
            } else if (msb.revealed[y][x]) {
                if (msb.board[y][x] == -1) printf("M ");
                else printf("%i ", msb.board[y][x]);
            } else {
                printf("* ");
            }
        }
        printf("\n");
    }
}

void usage() {
    printf("Usage: sms new [rows] [cols] [mine count]\n"
           "       sms move [row] [col]\n"
           "       sms flag [row] [col]\n"
           "       (row is a number, col is a letter a-z)\n");
}

void writeMSBToFile(MSBoard msb, FILE *fp) {
    // Write integers
    fwrite(&msb.width, sizeof(int), 1, fp);
    fwrite(&msb.height, sizeof(int), 1, fp);
    fwrite(&msb.mines, sizeof(int), 1, fp);
    fwrite(&msb.numRevealed, sizeof(int), 1, fp);

    // Write board array (mine positions and counts)
    for (int i = 0; i < msb.height; i++) {
        fwrite(msb.board[i], sizeof(char), msb.width, fp);
    }

    // Write revealed array
    for (int i = 0; i < msb.height; i++) {
        fwrite(msb.revealed[i], sizeof(char), msb.width, fp);
    }

    // Write flagged array
    for (int i = 0; i < msb.height; i++) {
        fwrite(msb.flagged[i], sizeof(char), msb.width, fp);
    }
}

MSBoard readMSBFromFile(FILE *fp) {
    MSBoard msb;

    // Read integers
    fread(&msb.width, sizeof(int), 1, fp);
    fread(&msb.height, sizeof(int), 1, fp);
    fread(&msb.mines, sizeof(int), 1, fp);
    fread(&msb.numRevealed, sizeof(int), 1, fp);

    // Allocate memory for arrays
    msb.board = malloc(sizeof(char*) * msb.height);
    msb.revealed = malloc(sizeof(char*) * msb.height);
    msb.flagged = malloc(sizeof(char*) * msb.height);

    for (int i = 0; i < msb.height; i++) {
        msb.board[i] = malloc(sizeof(char) * msb.width);
        msb.revealed[i] = malloc(sizeof(char) * msb.width);
        msb.flagged[i] = malloc(sizeof(char) * msb.width);
    }

    // Read board array (mine positions and counts)
    for (int i = 0; i < msb.height; i++) {
        fread(msb.board[i], sizeof(char), msb.width, fp);
    }

    // Read revealed array
    for (int i = 0; i < msb.height; i++) {
        fread(msb.revealed[i], sizeof(char), msb.width, fp);
    }

    // Read flagged array
    for (int i = 0; i < msb.height; i++) {
        fread(msb.flagged[i], sizeof(char), msb.width, fp);
    }

    return msb;
}

int main(int argc, char **argv) {

    if (argc < 2) {
        usage();
        return 1;
    }

    FILE *gsfp;
    MSBoard msb;

    if (!strcmp(argv[1], "new")) {
        if (argc < 5) {
            usage();
            return 1;
        }

        int rows = atoi(argv[2]);
        int cols = atoi(argv[3]);
        int minecount = atoi(argv[4]);

        if (rows < 2 || cols < 2 || minecount < 1) {
            printf("Invalid game setup.\n");
            return 1;
        }

        printf("Starting new game. Use 'sms move [row] [col]' to play (row is a number, col is a letter).\n");

        srand(time(0));
        msb = createBoard(cols, rows, minecount);

        gsfp = fopen("gamestate", "wb");
        writeMSBToFile(msb, gsfp);
        fclose(gsfp);

    } else if (!strcmp(argv[1], "move")) {
        if (argc < 4) {
            usage();
            return 1;
        }

        gsfp = fopen("gamestate", "rb");
        msb = readMSBFromFile(gsfp);
        fclose(gsfp);

        int numCells = msb.width * msb.height;
        int revealedGoal = numCells - msb.mines;
        int gameLost = 0;

        // Parse row (first argument, a number)
        int row = atoi(argv[2]);
        if (row < 1 || row > msb.height) {
            printf("Invalid row number.\n");
            return 1;
        }

        // Parse column (second argument, a letter)
        char col = argv[3][0];
        int x;
        if (col >= 'a' && col <= 'z') {
            x = col - 'a';
        } else if (col >= 'A' && col <= 'Z') {
            x = col - 'A' + 26;
        } else {
            printf("Invalid column letter.\n");
            return 1;
        }
        if (x < 0 || x >= msb.width) {
            printf("Column out of range.\n");
            return 1;
        }

        int result = revealMine(&msb, x, row-1);
        if (result) {
            revealAll(&msb);
            gameLost = 1;
        } else if (revealedGoal == msb.numRevealed) {
            // Win condition - all safe cells revealed
            revealAll(&msb);
            printBoard(msb);
            printf("Board cleared, you won!\n");
            return 0;
        }

        if (gameLost) {
            printf("Mine triggered, game over!\n");
        }

        gsfp = fopen("gamestate", "wb");
        writeMSBToFile(msb, gsfp);
        fclose(gsfp);

    } else if (!strcmp(argv[1], "flag")) {
        if (argc < 4) {
            usage();
            return 1;
        }

        gsfp = fopen("gamestate", "rb");
        msb = readMSBFromFile(gsfp);
        fclose(gsfp);

        // Parse row (first argument, a number)
        int row = atoi(argv[2]);
        if (row < 1 || row > msb.height) {
            printf("Invalid row number.\n");
            return 1;
        }

        // Parse column (second argument, a letter)
        char col = argv[3][0];
        int x;
        if (col >= 'a' && col <= 'z') {
            x = col - 'a';
        } else if (col >= 'A' && col <= 'Z') {
            x = col - 'A' + 26;
        } else {
            printf("Invalid column letter.\n");
            return 1;
        }
        if (x < 0 || x >= msb.width) {
            printf("Column out of range.\n");
            return 1;
        }

        // Toggle flag at the position
        msb.flagged[row-1][x] = !msb.flagged[row-1][x];

        gsfp = fopen("gamestate", "wb");
        writeMSBToFile(msb, gsfp);
        fclose(gsfp);

    } else {
        printf("Unknown command %s\n", argv[1]);
        usage();
        return 1;
    }

    printBoard(msb);

    return 0;
    
}
