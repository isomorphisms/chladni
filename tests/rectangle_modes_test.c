#include "../src/rectangle_modes.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static void check(int condition, const char *name)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", name);
        exit(1);
    }
}

static void check_near(double actual, double expected, double tolerance,
                       const char *name)
{
    check(isfinite(actual) && fabs(actual - expected) <= tolerance, name);
}

int main(void)
{
    const double pi = acos(-1.0);
    RectangleMode fundamental = { 2.0, 1.0, 1, 1 };
    check(rectangle_mode_is_valid(fundamental), "valid rectangle");
    check_near(rectangle_mode_value(fundamental, 1.0, 0.5), 1.0,
               1e-13, "fundamental maximum");
    check_near(rectangle_mode_value(fundamental, 0.0, 0.3), 0.0,
               1e-13, "fixed left edge is a node");
    check_near(rectangle_mode_value(fundamental, 1.3, 1.0), 0.0,
               1e-13, "fixed bottom edge is a node");
    check_near(rectangle_wave_number_squared(fundamental),
               pi * pi * (0.25 + 1.0), 1e-12, "rectangular eigenvalue");
    check_near(rectangle_plate_angular_frequency(fundamental, 3.0),
               3.0 * rectangle_wave_number_squared(fundamental),
               1e-12, "plate angular frequency scaling");
    check(isnan(rectangle_plate_angular_frequency(fundamental, 0.0)),
          "invalid bending scale");
    check(isnan(rectangle_mode_value(fundamental, -0.1, 0.5)),
          "reject out-of-domain point");
    RectangleMode invalid = { 0.0, 1.0, 1, 1 };
    check(isnan(rectangle_wave_number_squared(invalid)),
          "reject invalid rectangle");
    RectangleMode square = { 1.0, 1.0, 1, 2 };
    check(rectangle_swapped_mode_is_degenerate(square),
          "square swapped pair is degenerate");
    check(rectangle_swapped_mode_is_degenerate(fundamental),
          "identical indices are trivially degenerate");
    check_near(rectangle_combined_mode_value(square,
                   RECTANGLE_ANTISYMMETRIC_PAIR, 0.37, 0.37),
               0.0, 1e-13, "antisymmetric nodal diagonal");
    check_near(rectangle_combined_mode_value(square,
                   RECTANGLE_SYMMETRIC_PAIR, 0.5, 0.25),
               1.0 / sqrt(2.0), 1e-13, "symmetric pair normalized sum");
    RectangleMode nonsquare_pair = { 2.0, 1.0, 1, 2 };
    check(!rectangle_swapped_mode_is_degenerate(nonsquare_pair),
          "nonsquare indices are not degenerate");
    check(isnan(rectangle_combined_mode_value(nonsquare_pair,
                    RECTANGLE_ANTISYMMETRIC_PAIR, 0.5, 0.5)),
          "refuse non-eigenmode mixture");
    check(isnan(rectangle_combined_mode_value(square, (RectangleModeCombination)99,
                    0.5, 0.5)), "refuse unknown combination");
    puts("rectangle_modes: all checks passed");
    return 0;
}
