#include "rectangle_modes.h"

#include <float.h>
#include <math.h>

static double pi_value(void)
{
    return acos(-1.0);
}

int rectangle_mode_is_valid(RectangleMode mode)
{
    return isfinite(mode.width) && isfinite(mode.height) &&
           mode.width > 0.0 && mode.height > 0.0 &&
           mode.horizontal_index > 0 && mode.vertical_index > 0;
}

double rectangle_wave_number_squared(RectangleMode mode)
{
    if (!rectangle_mode_is_valid(mode)) {
        return NAN;
    }
    const double horizontal_wave_number =
        pi_value() * (double)mode.horizontal_index / mode.width;
    const double vertical_wave_number =
        pi_value() * (double)mode.vertical_index / mode.height;
    return horizontal_wave_number * horizontal_wave_number +
           vertical_wave_number * vertical_wave_number;
}

double rectangle_mode_value(RectangleMode mode, double x, double y)
{
    if (!rectangle_mode_is_valid(mode) || !isfinite(x) || !isfinite(y) ||
        x < 0.0 || x > mode.width || y < 0.0 || y > mode.height) {
        return NAN;
    }
    return sin(pi_value() * (double)mode.horizontal_index * x / mode.width) *
           sin(pi_value() * (double)mode.vertical_index * y / mode.height);
}

double rectangle_plate_angular_frequency(RectangleMode mode,
                                        double bending_scale)
{
    if (!isfinite(bending_scale) || bending_scale <= 0.0) {
        return NAN;
    }
    return bending_scale * rectangle_wave_number_squared(mode);
}

int rectangle_swapped_mode_is_degenerate(RectangleMode mode)
{
    if (!rectangle_mode_is_valid(mode)) {
        return 0;
    }
    RectangleMode exchanged = mode;
    exchanged.horizontal_index = mode.vertical_index;
    exchanged.vertical_index = mode.horizontal_index;
    const double first = rectangle_wave_number_squared(mode);
    const double second = rectangle_wave_number_squared(exchanged);
    return isfinite(first) && isfinite(second) &&
           fabs(first - second) <= 64.0 * DBL_EPSILON * fmax(first, second);
}

double rectangle_combined_mode_value(RectangleMode mode,
                                     RectangleModeCombination combination,
                                     double x, double y)
{
    const double first = rectangle_mode_value(mode, x, y);
    if (combination == RECTANGLE_SINGLE_MODE || !isfinite(first)) {
        return first;
    }
    if (combination != RECTANGLE_SYMMETRIC_PAIR &&
        combination != RECTANGLE_ANTISYMMETRIC_PAIR) {
        return NAN;
    }
    if (mode.horizontal_index == mode.vertical_index ||
        !rectangle_swapped_mode_is_degenerate(mode)) {
        return NAN;
    }
    RectangleMode exchanged = mode;
    exchanged.horizontal_index = mode.vertical_index;
    exchanged.vertical_index = mode.horizontal_index;
    const double second = rectangle_mode_value(exchanged, x, y);
    const double sign = (combination == RECTANGLE_SYMMETRIC_PAIR) ? 1.0 : -1.0;
    return (first + sign * second) / sqrt(2.0);
}
