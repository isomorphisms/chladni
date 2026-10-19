#include "rectangle_experiment.h"

#include <math.h>
#include <stdint.h>

static void append_waves(ChladniRectangleResult *result, RectangleMode mode,
                         double scale)
{
    const double horizontal = acos(-1.0) * mode.horizontal_index / mode.width;
    const double vertical = acos(-1.0) * mode.vertical_index / mode.height;
    for (int horizontal_sign = -1; horizontal_sign <= 1; horizontal_sign += 2) {
        for (int vertical_sign = -1; vertical_sign <= 1; vertical_sign += 2) {
            result->waves[result->wave_count++] = (ChladniPlaneWave){
                horizontal_sign * horizontal, vertical_sign * vertical,
                -0.25 * horizontal_sign * vertical_sign * scale
            };
        }
    }
}

ChladniRectangleStatus chladni_rectangle_run(
    const ChladniRectangleRequest *request,
    billiard_segment *segments, size_t capacity,
    ChladniRectangleResult *result)
{
    if (!result) return CHL_RECTANGLE_INVALID;
    *result = (ChladniRectangleResult){0};
    if (!request || !segments || capacity == 0 ||
        request->version != CHL_RECTANGLE_CONTRACT_VERSION)
        return CHL_RECTANGLE_INVALID;
    if (request->boundary == CHL_FREE_EDGE) return CHL_RECTANGLE_UNSUPPORTED;
    if (request->boundary != CHL_DIRICHLET &&
        request->boundary != CHL_SIMPLY_SUPPORTED)
        return CHL_RECTANGLE_INVALID;
    if (!isfinite(request->bending_scale) ||
        (request->boundary == CHL_DIRICHLET && request->bending_scale != 0.0) ||
        (request->boundary == CHL_SIMPLY_SUPPORTED && request->bending_scale <= 0.0))
        return CHL_RECTANGLE_INVALID;
    if (!isfinite(request->distance_budget) || request->distance_budget < 0.0)
        return CHL_RECTANGLE_INVALID;

    const RectangleMode mode = {request->width, request->height,
        request->horizontal_index, request->vertical_index};
    const billiard_table table = {request->width, request->height,
        request->source_x, request->source_y, request->launch_degrees};
    billiard_ray initial_ray;
    if (!rectangle_mode_is_valid(mode) || billiard_begin(&table, &initial_ray))
        return CHL_RECTANGLE_INVALID;
    if (request->combination != RECTANGLE_SINGLE_MODE &&
        request->combination != RECTANGLE_SYMMETRIC_PAIR &&
        request->combination != RECTANGLE_ANTISYMMETRIC_PAIR)
        return CHL_RECTANGLE_INVALID;
    if (request->combination != RECTANGLE_SINGLE_MODE &&
        (mode.horizontal_index == mode.vertical_index ||
         !rectangle_swapped_mode_is_degenerate(mode)))
        return CHL_RECTANGLE_INVALID;
    if (!isfinite(request->sample_x) || !isfinite(request->sample_y) ||
        request->sample_x < 0.0 || request->sample_x > request->width ||
        request->sample_y < 0.0 || request->sample_y > request->height)
        return CHL_RECTANGLE_INVALID;

    ChladniRectangleResult proposed = {0};
    proposed.request = *request;
    proposed.wave_number_squared = rectangle_wave_number_squared(mode);
    proposed.sampled_amplitude = rectangle_combined_mode_value(
        mode, request->combination, request->sample_x, request->sample_y);
    proposed.angular_frequency = request->boundary == CHL_SIMPLY_SUPPORTED
        ? rectangle_plate_angular_frequency(mode, request->bending_scale) : NAN;
    if (!isfinite(proposed.wave_number_squared) ||
        !isfinite(proposed.sampled_amplitude) ||
        (request->boundary == CHL_SIMPLY_SUPPORTED &&
         !isfinite(proposed.angular_frequency)))
        return CHL_RECTANGLE_NUMERICAL;

    append_waves(&proposed, mode,
        request->combination == RECTANGLE_SINGLE_MODE ? 1.0 : 1.0 / sqrt(2.0));
    if (request->combination != RECTANGLE_SINGLE_MODE) {
        const RectangleMode swapped = {mode.width, mode.height,
            mode.vertical_index, mode.horizontal_index};
        append_waves(&proposed, swapped,
            (request->combination == RECTANGLE_SYMMETRIC_PAIR ? 1.0 : -1.0) /
            sqrt(2.0));
    }
    proposed.segment_count = billiard_trace(&table, request->distance_budget,
                                            segments, capacity);
    if (proposed.segment_count == SIZE_MAX) return CHL_RECTANGLE_NUMERICAL;
    proposed.trace_complete = request->distance_budget == 0.0;
    if (proposed.segment_count) {
        const billiard_segment *last = &segments[proposed.segment_count - 1];
        proposed.trace_complete = last->hit == BILLIARD_NO_HIT ||
            last->distance_start + last->length >= request->distance_budget;
    }
    proposed.valid = 1;
    *result = proposed;
    return CHL_RECTANGLE_OK;
}

int chladni_rectangle_result_matches(const ChladniRectangleResult *result,
                                     const ChladniRectangleRequest *request)
{
    if (!result || !result->valid || !request) return 0;
    const ChladniRectangleRequest *prior = &result->request;
    return prior->version == request->version &&
        prior->width == request->width && prior->height == request->height &&
        prior->source_x == request->source_x && prior->source_y == request->source_y &&
        prior->launch_degrees == request->launch_degrees &&
        prior->horizontal_index == request->horizontal_index &&
        prior->vertical_index == request->vertical_index &&
        prior->combination == request->combination && prior->boundary == request->boundary &&
        prior->bending_scale == request->bending_scale &&
        prior->sample_x == request->sample_x && prior->sample_y == request->sample_y &&
        prior->distance_budget == request->distance_budget;
}
