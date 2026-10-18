#ifndef CHLADNI_RECTANGLE_MODES_H
#define CHLADNI_RECTANGLE_MODES_H

/* Exact separated modes for a rectangular Dirichlet Laplacian / simply
 * supported thin plate. Not an approximation to a free-edge Chladni plate.
 * Coordinates match isomorphisms/billiard: (0,0) is the upper-left corner.
 */

typedef struct {
    double width;
    double height;
    unsigned horizontal_index;
    unsigned vertical_index;
} RectangleMode;

typedef enum {
    RECTANGLE_SINGLE_MODE,
    RECTANGLE_SYMMETRIC_PAIR,
    RECTANGLE_ANTISYMMETRIC_PAIR
} RectangleModeCombination;

/* Return 1 for positive finite dimensions and positive indices, else 0. */
int rectangle_mode_is_valid(RectangleMode mode);

/* k^2 in inverse-length squared units. Invalid inputs return NAN. */
double rectangle_wave_number_squared(RectangleMode mode);

/* Normalized eigenfunction. Out-of-domain and invalid inputs return NAN. */
double rectangle_mode_value(RectangleMode mode, double x, double y);

/* sqrt(D/(rho*h)) * k^2, with bending_scale = sqrt(D/(rho*h)) > 0.
 * Returns radians per time unit. Invalid inputs return NAN.
 */
double rectangle_plate_angular_frequency(RectangleMode mode,
                                        double bending_scale);

/* A paired mode exchanges the two positive indices; for a square this is
 * exactly degenerate, while a non-square generally is not.
 * Return 1 if the two eigenvalues coincide at floating-point precision.
 */
int rectangle_swapped_mode_is_degenerate(RectangleMode mode);

/* A normalized (phi_mn +/- phi_nm)/sqrt(2) when indices differ.
 * Refuse mixtures unless swapped modes are degenerate. Invalid -> NAN.
 * Single mode ignores the combination's pairing condition.
 */
double rectangle_combined_mode_value(RectangleMode mode,
                                     RectangleModeCombination combination,
                                     double x, double y);

#endif
