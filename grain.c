/*
    'grain', generates shape with perlin noise output to terminal using quadrant-block
    Unicode characters for finer quality.

    Uses https://github.com/nothings/stb/blob/master/stb_perlin.h for Perlin noise.

    Flags:
        -x (number): X offset
        -y (number): Y offset
        -z (number): Zoom
        -s (number): Random seed
        -c (number): Output width (columns)
        -r (number): Output height (rows)
        -i : Invert colors
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define STB_PERLIN_IMPLEMENTATION
#include "includes/stdperlin.h"

int width = 32;
int height = 16;
float xOffset = 0;
float yOffset = 0;
float zoom = 0.08;
float invert = 1;
int seed = 0;

int parseargs(int argc, char *argv[]) {
    // Please Excuse this sloppy argument parsing code
    for (int i = 0; i < argc; i++) {
        char *arg = argv[i];
        if (arg[0] == '-') {
            // flag
            char flag = arg[1];
            switch(flag) {
                case 's':
                    // seed flag
                    if (++i == argc) {
                        printf("Flag -s missing argument\n");
                        return 1;
                    } else if (argv[i][0] == '-') {
                        printf("Flag -s missing argument\n");
                        return 1;
                    } else {
                        seed = atoi(argv[i]);
                    }
                    break;
                case 'i':
                    // seed flag
                    invert = -1;
                    break;
                case 'h':
                    printf("'grain', generates shape with perlin noise output to terminal using \nquadrant-block Unicode characters for finer quality.\nFlags:\n\t-x (number): X offset\n\t-y (number): Y offset\n\t-z (number): Zoom\n\t-s (number): Random seed\n\t-c (number): Output width (columns)\n\t-r (number): Output height (rows)\n\t-i : Invert colors\n");
                    return 1;
                case 'x':
                    // x offset flag
                    if (++i == argc) {
                        printf("Flag -x missing argument\n");
                        return 1;
                    } else {
                        xOffset = atof(argv[i]);
                    }
                    break;
                case 'y':
                    // y offset flag
                    if (++i == argc) {
                        printf("Flag -y missing argument\n");
                        return 1;
                    } else {
                        yOffset = atof(argv[i]);
                    }
                    break;
                case 'z': 
                    // zoom flag
                    if (++i == argc) {
                        printf("Flag -z missing argument\n");
                        return 1;
                    } else if (argv[i][0] == '-') {
                        printf("Flag -z missing argument\n");
                        return 1;
                    } else {
                        zoom = atof(argv[i]);
                        if (zoom <= 0.0f) {
                            printf("Invalid argument to -z\n");
                            return 1;
                        }
                    }
                    break;
                case 'c':
                    // width flag
                    if (++i == argc) {
                        printf("Flag -c missing argument\n");
                        return 1;
                    } else if (argv[i][0] == '-') {
                        printf("Flag -c missing argument\n");
                        return 1;
                    } else {
                        width = atoi(argv[i]);
                        if (width < 1) {
                            printf("Invalid argument to -c\n");
                            return 1;
                        }
                    }
                    break;
                case 'r':
                    // height flag
                    if (++i == argc) {
                        printf("Flag -r missing argument\n");
                        return 1;
                    } else if (argv[i][0] == '-') {
                        printf("Flag -r missing argument\n");
                        return 1;
                    } else {
                        height = atoi(argv[i]);
                        if (height < 1) {
                            printf("Invalid argument to -r\n");
                            return 1;
                        }
                    }
                    break;
                default:
                    printf("Unknown flag -%c\n", flag);
                    return 1;
            }
        }
    }
    return 0;
}

/* Each of these symbols has a specific set of quadrants filled in; the first 4 bits of their
   respective indices corrosponds to whether specific one of them is filled or not */
char *symbols[16] = {" ","▖","▘","▌","▗","▄","▚","▙","▝","▞","▀","▛","▐","▟","▜","█"};

int main(int argc, char *argv[]) {
    
    if (parseargs(argc, argv)) {
        return 1;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = 0;
            if (invert*stb_perlin_noise3_seed(((float)(x - width/2)*2)*zoom + xOffset, ((float)(y - height/2)*2+1)*zoom + yOffset, 0, 0, 0, 0, seed) > 0)
                idx += 0b0001; // Bottom left
            if (invert*stb_perlin_noise3_seed(((float)(x - width/2)*2)*zoom + xOffset, ((float)(y - height/2)*2)*zoom + yOffset, 0, 0, 0, 0, seed) > 0)
                idx += 0b0010; // Top left
            if (invert*stb_perlin_noise3_seed(((float)(x - width/2)*2+1)*zoom + xOffset, ((float)(y - height/2)*2+1)*zoom + yOffset, 0, 0, 0, 0, seed) > 0)
                idx += 0b0100; // Bottom right
            if (invert*stb_perlin_noise3_seed(((float)(x - width/2)*2+1)*zoom + xOffset, ((float)(y - height/2)*2)*zoom + yOffset, 0, 0, 0, 0, seed) > 0)
                idx += 0b1000; // Top right
            printf("%s", symbols[idx]);       
        }
        printf("\n");
    }

}