#ifndef CHLADNI_RECTANGLE_EXPERIMENT_H
#define CHLADNI_RECTANGLE_EXPERIMENT_H

#include "rectangle_modes.h"
#include "billiard.h"

#define CHL_RECTANGLE_CONTRACT_VERSION 1u

typedef enum {
    CHL_DIRICHLET = 1,
    CHL_SIMPLY_SUPPORTED = 2,
    CHL_FREE_EDGE = 3
} ChladniBoundary;

typedef enum {
    CHL_RECTANGLE_OK = 0,
    CHL_RECTANGLE_INVALID = 1,
    CHL_RECTANGLE_UNSUPPORTED = 2,
    CHL_RECTANGLE_NUMERICAL = 3
} ChladniRectangleStatus;

/* One immutable input owns both models' dimensions. Coordinates have a
 * top-left origin, x rightward and y downward, in one shared length unit.
 * launch_degrees preserves Billiard's y-up counterclockwise angle convention.
 * It is independently adjustable; it need not be a mode wavevector direction.
 * Dirichlet uses bending_scale = 0; the plate uses sqrt(D/(rho*h)) > 0.
 */
typedef struct {
    unsigned version;
    double width, height;
    double source_x, source_y, launch_degrees;
    unsigned horizontal_index, vertical_index;
    RectangleModeCombination combination;
    ChladniBoundary boundary;
    double bending_scale;
    double sample_x, sample_y;
    double distance_budget;
} ChladniRectangleRequest;

typedef struct {
    double horizontal_wave_number, vertical_wave_number;
    double real_coefficient;
} ChladniPlaneWave;

typedef struct {
    int valid;
    ChladniRectangleRequest request;
    double wave_number_squared, angular_frequency, sampled_amplitude;
    /* One separated mode has four components; a degenerate pair has eight.
     * Sum coefficient * exp(i * wavevector.dot(position)) to reconstruct it.
     */
    ChladniPlaneWave waves[8];
    size_t wave_count, segment_count;
    int trace_complete;
} ChladniRectangleResult;

/* Borrow the request for this call; do not mutate it concurrently. Starts a
 * fresh ray on EVERY call. Segments belong to the caller. Capacity truncation
 * is explicit. Failure clears result.valid/counts, invalidating old output.
 * A Dirichlet result has NAN angular_frequency; a free-edge request fails.
 * No mutable continuation ray or cached geometry enters this interface.
 */
ChladniRectangleStatus chladni_rectangle_run(
    const ChladniRectangleRequest *request,
    billiard_segment *segments, size_t capacity,
    ChladniRectangleResult *result);

/* Compare every semantic input field, never struct padding. Consumers use
 * this to reject a stale result after dimensions or another input change.
 */
int chladni_rectangle_result_matches(
    const ChladniRectangleResult *result,
    const ChladniRectangleRequest *request);

#endif
