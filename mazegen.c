#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define ANIMATE 0  // 1 to make it animate...
#define INT     10 // ...with an interval of INT milliseconds per frame

struct Tile {
    short right; /* 1 = wall (blocked), 0 = empty (can pass to tile on other side) */
    short down;
    /* the left or upper wall can be checked by checking the left cell's right wall (or col == 0) and checking the upper cell's
       lower wall (or row == 0) respectively */
};

struct Tile **maze;
short **visited;
int numvisited;
int numtiles;

void drawmaze(int rows, int cols) {
    const char* crosses[16] = {" ", "╺", "╸", "━", "╻", "┏", "┓", "┳", "╹", "┗", "┛", "┻", "┃", "┣", "┫", "╋"};
    for (int r = -1; r < rows; r++) {
        for (int c = -1; c < cols; c++) {
            int arms = 0; // 0000 : up, down, left, right
            if ((r != -1 && c != -1 && maze[r][c].right == 1) || (c == -1 && r != -1)) arms += 0b1000;
            if ((c != -1 && r != -1 && maze[r][c].down == 1) || (r == -1 && c != -1)) arms += 0b0010;
            if ((c != cols-1 && r != -1 && maze[r][c+1].down == 1) || (r == -1 && c != cols-1)) arms += 0b0001;
            if ((r != rows-1 && c != -1 && maze[r+1][c].right == 1) || (c == -1 && r != rows-1)) arms += 0b0100;
            if ((c != -1 && r != -1 && maze[r][c].down == 1) || (r == -1 && c != -1)) {
                printf("━");
            } else if (c != -1) {
                printf(" ");
            }
            printf("%s", crosses[arms]);
        }
        printf("\n");
    }
}

void sleep_ms(int ms) {
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	nanosleep(&ts, NULL);
}

void bt(int row, int col, int rows, int cols, int depth) {
    int v_c;
    visited[row][col] = 1;
    numvisited++;
    do {
        // pick random direction to tunnel toward
        int dir[4];
        v_c = 0; // valid_choices
        if (row != 0 && visited[row-1][col] == 0) dir[v_c++] = 0; // up 0 is avaliable
        if (col != 0 && visited[row][col-1] == 0) dir[v_c++] = 2; // left 2 is availabe
        if (row != rows-1 && visited[row+1][col] == 0) dir[v_c++] = 1; // down 1 is available
        if (col != cols-1 && visited[row][col+1] == 0) dir[v_c++] = 3; // right 3 is available
        if (v_c == 0) 
            return; // nowhere to tunnel from here. move back

        if (ANIMATE == 1) {
            system("clear");
            drawmaze(rows, cols);
            printf("%2d,%-2d | depth:%-3d | visited:%3d/%-3d\n\n", row, col, depth, numvisited, numtiles);
            sleep_ms(INT);
        }
        
        int pick = rand() % v_c;
        
        switch (dir[pick]) {
            case 0: // up
                maze[row-1][col].down = 0;
                bt(row-1, col, rows, cols, depth+1);
                break;
            case 1: // down
                maze[row][col].down = 0;
                bt(row+1, col, rows, cols, depth+1);
                break;
            case 2: // left
                maze[row][col-1].right = 0;
                bt(row, col-1, rows, cols, depth+1);
                break;
            case 3: // right
                maze[row][col].right = 0;
                bt(row, col+1, rows, cols, depth+1);
                break;
        }
    } while (v_c > 1 && numvisited <= numtiles);
}

int main(int argc, char ** argv) {

    int rows, cols;
    time_t t; srand((unsigned)time(&t));

    // parse args
    if (argc < 2) {
        rows = 10;
        cols = 10;
    } else if (argc < 3) {
        int atoy = atoi(argv[1]); atoy = atoy > 1 ? atoy : 10;
        rows = atoy;
        cols = atoy;
    } else {
        int atoyr = atoi(argv[1]); atoyr = atoyr > 1 ? atoyr : 10;
        int atoyc = atoi(argv[2]); atoyc = atoyc > 1 ? atoyc : 10;
        rows = atoyr;
        cols = atoyc;
    }

    maze = malloc(sizeof(struct Tile *) * rows);
    visited = malloc(sizeof(short *) * rows);
    for (int i = 0; i < rows; i++) {
        maze[i] = malloc(sizeof(struct Tile)*cols);
        visited[i] = malloc(sizeof(short)*cols);
    }

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            maze[r][c].down = 1;
            maze[r][c].right = 1;
            visited[r][c] = 0;
        }
    }

    numtiles = rows*cols;
    numvisited = 0; /* So bt() knows when it can just exit the do while loop if all tiles visited without needing to 
                       iterate through it again every step back. Makes the animation end immediately when this happens
                       rather than continuing visually frozen for as many frames as it needs to backtrack to the start. */
    bt(0, 0, rows, cols, 0);

    if (ANIMATE == 1) system("clear");
    drawmaze(rows, cols);
    printf("\n");

}
