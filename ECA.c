/*
    ECA.c: Outputs to terminal a visualization of the time-space diagram of 
    any elementary cellular automata (rule 0-255).
*/

#include <stdio.h>
#include <stdlib.h>

#define WIDTH 80

int main(int argc, char *argv[]) {
    int rule = 30;
    if (argc > 1) {
        rule = atoi(argv[1]) % 256;
    }

    char space = ' ';
    char fill  = '#';

    char bits[WIDTH+1];
    for (int i = 0; i < WIDTH; i++) 
        bits[i] = space;
    bits[WIDTH/2] = fill;
    //bits[WIDTH-1] = fill;
    bits[WIDTH] = '\0';
    
    int cy = 0;
    char nextbits[WIDTH];
    while (cy < 128) {
        printf("%s\n", bits);
        for (int i = 0; i < WIDTH; i++) {
            int r = 0;
            for (int j = -1; j <= 1; j++) {
                if (i+j >= 0 && i+j < WIDTH && bits[i+j] == fill) {
                    r += 1 << (1-j);
                }
            }
            nextbits[i] = (rule >> r & 1) ? fill : space;
        }
        for (int i = 0; i < WIDTH; i++) {
            bits[i] = nextbits[i];
        }
        cy++;
    }
}