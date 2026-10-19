#include "rectangle_modes.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Produce a portable PGM (P5) picture, white near nodal lines.
 * This is a diagnostic of a theoretical eigenfunction, not a sand simulation.
 */
static int read_positive_double(const char *source, double *output)
{
    char *end = NULL;
    errno = 0;
    *output = strtod(source, &end);
    return errno == 0 && end != source && *end == '\0' &&
           isfinite(*output) && *output > 0.0;
}

static int read_positive_unsigned(const char *source, unsigned *output)
{
    char *end = NULL;
    errno = 0;
    const unsigned long converted = strtoul(source, &end, 10);
    if (errno || end == source || *end != '\0' || source[0] == '-' ||
        converted == 0 || converted > 2048UL) {
        return 0;
    }
    *output = (unsigned)converted;
    return 1;
}

static void usage(const char *command)
{
    fprintf(stderr,
            "usage: %s width height m n columns rows [single|symmetric|antisymmetric] > nodal.pgm\n"
            "dimensions are physical lengths; m,n are positive mode indices.\n"
            "Only degenerate swapped modes may be combined (for example on a square).\n",
            command);
}

int main(int argc, char **argv)
{
    if (argc != 7 && argc != 8) {
        usage(argv[0]);
        return 2;
    }
    RectangleMode mode;
    unsigned columns;
    unsigned rows;
    if (!read_positive_double(argv[1], &mode.width) ||
        !read_positive_double(argv[2], &mode.height) ||
        !read_positive_unsigned(argv[3], &mode.horizontal_index) ||
        !read_positive_unsigned(argv[4], &mode.vertical_index) ||
        !read_positive_unsigned(argv[5], &columns) ||
        !read_positive_unsigned(argv[6], &rows) ||
        !isfinite(rectangle_wave_number_squared(mode))) {
        usage(argv[0]);
        return 2;
    }
    RectangleModeCombination combination = RECTANGLE_SINGLE_MODE;
    if (argc == 8) {
        if (strcmp(argv[7], "single") == 0) {
            combination = RECTANGLE_SINGLE_MODE;
        } else if (strcmp(argv[7], "symmetric") == 0) {
            combination = RECTANGLE_SYMMETRIC_PAIR;
        } else if (strcmp(argv[7], "antisymmetric") == 0) {
            combination = RECTANGLE_ANTISYMMETRIC_PAIR;
        } else {
            usage(argv[0]);
            return 2;
        }
    }
    if (combination != RECTANGLE_SINGLE_MODE &&
        (mode.horizontal_index == mode.vertical_index ||
         !rectangle_swapped_mode_is_degenerate(mode))) {
        fprintf(stderr, "requested pairing is not a pair of distinct degenerate modes\n");
        return 2;
    }
    /* Stdout contains only the binary image; diagnostics belong on stderr. */
    fprintf(stderr, "k^2 = %.12g; plate omega / sqrt(D/(rho*h)) = %.12g\n",
            rectangle_wave_number_squared(mode),
            rectangle_wave_number_squared(mode));
    if (printf("P5\n%u %u\n255\n", columns, rows) < 0) {
        return 1;
    }
    for (unsigned row = 0; row < rows; row++) {
        for (unsigned column = 0; column < columns; column++) {
            const double x = mode.width * ((double)column + 0.5) / columns;
            const double y = mode.height * ((double)row + 0.5) / rows;
            const double amplitude =
                rectangle_combined_mode_value(mode, combination, x, y);
            /* Gaussian proximity to the zero set, not a probability model. */
            const double scaled = fabs(amplitude) / 0.06;
            const unsigned char pixel =
                (unsigned char)lround(255.0 * exp(-0.5 * scaled * scaled));
            if (fputc(pixel, stdout) == EOF) {
                return 1;
            }
        }
    }
    return fflush(stdout) == 0 ? 0 : 1;
}
