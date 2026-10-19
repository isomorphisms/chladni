#include "rectangle_experiment.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned checks;
static void check(int condition, const char *message)
{
    ++checks;
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

static void near(double actual, double expected, const char *message)
{
    check(isfinite(actual) && fabs(actual - expected) <= 2e-12, message);
}

static ChladniRectangleRequest fixture(void)
{
    const ChladniRectangleRequest request = {
        CHL_RECTANGLE_CONTRACT_VERSION, 1.0, 1.0, 0.25, 0.5, 0.0,
        1, 1, RECTANGLE_SINGLE_MODE, CHL_SIMPLY_SUPPORTED, 3.0,
        0.5, 0.5, 2.5
    };
    return request;
}

static void rejected(ChladniRectangleRequest request,
                      ChladniRectangleStatus expected, const char *message)
{
    billiard_segment segments[16];
    ChladniRectangleResult result;
    ChladniRectangleRequest good = fixture();
    check(chladni_rectangle_run(&good, segments, 16, &result) == CHL_RECTANGLE_OK,
          "prime a valid output before negative control");
    check(chladni_rectangle_run(&request, segments, 16, &result) == expected, message);
    check(!result.valid && !result.segment_count && !result.wave_count,
          "failure invalidates old output");
    check(!chladni_rectangle_result_matches(&result, &good), "failed output is stale");
}

static void reconstruct(const ChladniRectangleResult *result, double x, double y,
                         double expected)
{
    double real_part = 0.0, imaginary_part = 0.0;
    for (size_t index = 0; index < result->wave_count; ++index) {
        const ChladniPlaneWave wave = result->waves[index];
        const double phase = wave.horizontal_wave_number * x +
                             wave.vertical_wave_number * y;
        real_part += wave.real_coefficient * cos(phase);
        imaginary_part += wave.real_coefficient * sin(phase);
    }
    near(real_part, expected, "signed waves reconstruct independent spatial value");
    near(imaginary_part, 0.0, "imaginary components cancel");
}

int main(void)
{
    const double pi = acos(-1.0);
    billiard_segment segments[32];
    ChladniRectangleResult result, old;
    ChladniRectangleRequest request = fixture();
    check(chladni_rectangle_run(&request, segments, 32, &result) == CHL_RECTANGLE_OK,
          "unit square shared request");
    check(result.valid && result.trace_complete && result.wave_count == 4,
          "complete ray and four wave components");
    near(segments[0].x1, 1.0, "unit square wall");
    near(segments[0].length, 0.75, "unit square first flight");
    near(result.wave_number_squared, 2.0 * pi * pi, "unit square k squared");
    near(result.angular_frequency, 6.0 * pi * pi, "unit square plate frequency");
    near(result.sampled_amplitude, 1.0, "unit square sample");
    check(chladni_rectangle_result_matches(&result, &request), "result matches request");
    old = result;
    request.width = 2.0;
    check(!chladni_rectangle_result_matches(&old, &request), "resize rejects cached square");
    check(chladni_rectangle_run(&request, segments, 32, &result) == CHL_RECTANGLE_OK,
          "resized rectangle recomputes both owners");
    near(segments[0].x0, 0.25, "resize restarts from requested source");
    near(segments[0].x1, 2.0, "resized wall");
    near(segments[0].length, 1.75, "resized first flight");
    near(result.wave_number_squared, 1.25 * pi * pi, "resized k squared");
    near(result.angular_frequency, 3.75 * pi * pi, "resized plate frequency");
    near(result.sampled_amplitude, sqrt(0.5), "resized nodal sample");
    printf("rectangle\twidth\theight\tk_squared\tomega\n");
    printf("shared\t1\t1\t%.15g\t%.15g\n", old.wave_number_squared,
           old.angular_frequency);
    printf("shared\t2\t1\t%.15g\t%.15g\n", result.wave_number_squared,
           result.angular_frequency);
    reconstruct(&result, 0.5, 0.5, sqrt(0.5));
    reconstruct(&result, 0.0, 0.37, 0.0);
    reconstruct(&result, 2.0, 0.37, 0.0);
    reconstruct(&result, 0.71, 0.0, 0.0);
    reconstruct(&result, 0.71, 1.0, 0.0);
    reconstruct(&result, 1.0, 0.5, 1.0);

    /* Exercise all four signed components through the existing ray owner.
     * Launch in each component direction; its first reflection must flip
     * exactly the normal component, preserving tangential phase gradient.
     */
    for (size_t index = 0; index < result.wave_count; ++index) {
        const ChladniPlaneWave wave = result.waves[index];
        const double magnitude = sqrt(result.wave_number_squared);
        billiard_table table = {2.0, 1.0, 0.61, 0.37,
            atan2(-wave.vertical_wave_number, wave.horizontal_wave_number) * 180.0 / pi};
        for (int wall = 1; wall <= 2; ++wall) {
            table.start_x = wall == 1
                ? (wave.horizontal_wave_number > 0.0 ? 1.95 : 0.05) : 1.0;
            table.start_y = wall == 2
                ? (wave.vertical_wave_number > 0.0 ? 0.95 : 0.05) : 0.5;
            billiard_ray ray;
            billiard_segment first;
            check(billiard_begin(&table, &ray) == 0, "component ray begins");
            near(ray.dx, wave.horizontal_wave_number / magnitude, "wave normal x");
            near(ray.dy, wave.vertical_wave_number / magnitude, "wave normal y");
            const double before_x = ray.dx, before_y = ray.dy;
            check(billiard_next(&table, &ray, &first) == 0, "component reflects");
            check((int)first.hit == wall, "test exercises the requested wall");
            near(ray.dx, wall == 1 ? -before_x : before_x, "normal x reflection");
            near(ray.dy, wall == 2 ? -before_y : before_y, "normal y reflection");
        }
    }
    request.launch_degrees = 45.0;
    check(chladni_rectangle_run(&request, segments, 32, &result) == CHL_RECTANGLE_OK,
          "independent adjustable launch remains valid");
    near(fabs(result.waves[0].vertical_wave_number /
              result.waves[0].horizontal_wave_number), 2.0,
         "2x1 fundamental component slope differs from 45-degree launch");

    request = fixture();
    request.horizontal_index = 1;
    request.vertical_index = 2;
    request.combination = RECTANGLE_ANTISYMMETRIC_PAIR;
    request.sample_x = request.sample_y = 0.37;
    check(chladni_rectangle_run(&request, segments, 32, &result) == CHL_RECTANGLE_OK,
          "square degenerate combination");
    near(result.wave_number_squared, 5.0 * pi * pi, "square degeneracy eigenvalue");
    near(result.sampled_amplitude, 0.0, "antisymmetric diagonal");
    check(result.wave_count == 8, "paired mode exposes both sets of waves");
    reconstruct(&result, 0.25, 0.5, -sqrt(0.5));
    request.combination = RECTANGLE_SYMMETRIC_PAIR;
    check(chladni_rectangle_run(&request, segments, 32, &result) == CHL_RECTANGLE_OK,
          "symmetric degenerate combination");
    reconstruct(&result, 0.25, 0.5, sqrt(0.5));

    request = fixture();
    request.boundary = CHL_DIRICHLET;
    request.bending_scale = 0.0;
    request.distance_budget = 0.0;
    check(chladni_rectangle_run(&request, segments, 32, &result) == CHL_RECTANGLE_OK,
          "explicit Dirichlet spatial model");
    check(isnan(result.angular_frequency) && result.trace_complete && !result.segment_count,
          "Dirichlet has no plate frequency; zero budget is complete");
    request = fixture();
    request.distance_budget = 100.0;
    check(chladni_rectangle_run(&request, segments, 1, &result) == CHL_RECTANGLE_OK,
          "bounded trace result");
    check(!result.trace_complete && result.segment_count == 1,
          "capacity exhaustion cannot be called a completed trace");

    const double invalid_dimensions[] = {0.0, -1.0, NAN, INFINITY};
    for (size_t index = 0; index < 4; ++index) {
        request = fixture(); request.width = invalid_dimensions[index];
        rejected(request, CHL_RECTANGLE_INVALID, "reject invalid width");
        request = fixture(); request.height = invalid_dimensions[index];
        rejected(request, CHL_RECTANGLE_INVALID, "reject invalid height");
    }
    request = fixture(); request.source_x = 1.1;
    rejected(request, CHL_RECTANGLE_INVALID, "outside launch");
    request = fixture(); request.source_y = 0.0;
    rejected(request, CHL_RECTANGLE_INVALID, "boundary launch");
    request = fixture(); request.launch_degrees = NAN;
    rejected(request, CHL_RECTANGLE_INVALID, "nonfinite direction");
    request = fixture(); request.horizontal_index = 0;
    rejected(request, CHL_RECTANGLE_INVALID, "zero horizontal index");
    request = fixture(); request.vertical_index = 0;
    rejected(request, CHL_RECTANGLE_INVALID, "zero vertical index");
    request = fixture(); request.version = 2;
    rejected(request, CHL_RECTANGLE_INVALID, "unknown contract version");
    request = fixture(); request.boundary = (ChladniBoundary)99;
    rejected(request, CHL_RECTANGLE_INVALID, "unknown boundary label");
    request = fixture(); request.boundary = CHL_FREE_EDGE;
    rejected(request, CHL_RECTANGLE_UNSUPPORTED, "free edge never becomes simply supported");
    request = fixture(); request.bending_scale = 0.0;
    rejected(request, CHL_RECTANGLE_INVALID, "zero plate scale");
    request = fixture(); request.bending_scale = INFINITY;
    rejected(request, CHL_RECTANGLE_INVALID, "nonfinite plate scale");
    request = fixture(); request.combination = (RectangleModeCombination)99;
    rejected(request, CHL_RECTANGLE_INVALID, "unknown mode combination");
    request = fixture(); request.width = 2.0; request.vertical_index = 2;
    request.combination = RECTANGLE_SYMMETRIC_PAIR;
    rejected(request, CHL_RECTANGLE_INVALID, "nondegenerate mixture");
    request = fixture(); request.width = nextafter(1.0, 2.0);
    request.vertical_index = 2; request.combination = RECTANGLE_SYMMETRIC_PAIR;
    rejected(request, CHL_RECTANGLE_INVALID, "nearly square is not exactly degenerate");
    request = fixture(); request.sample_x = 1.1;
    rejected(request, CHL_RECTANGLE_INVALID, "sample outside geometry");
    request = fixture(); request.distance_budget = -1.0;
    rejected(request, CHL_RECTANGLE_INVALID, "negative ray budget");
    request = fixture(); request.bending_scale = DBL_MAX;
    rejected(request, CHL_RECTANGLE_NUMERICAL, "nonrepresentable plate frequency");
    request = fixture(); request.width = 1e-200; request.source_x = 0.5e-200;
    request.sample_x = 0.5e-200;
    rejected(request, CHL_RECTANGLE_NUMERICAL, "nonrepresentable wave number");
    const RectangleMode huge = {1e308, 1.0, 3, 1};
    near(rectangle_mode_value(huge, 5e307, 0.5), -1.0,
         "finite phase despite huge coordinates");
    request = fixture();
    check(chladni_rectangle_run(NULL, segments, 32, &result) == CHL_RECTANGLE_INVALID,
          "null request rejected");
    check(chladni_rectangle_run(&request, NULL, 32, &result) == CHL_RECTANGLE_INVALID,
          "null segment storage rejected");
    check(chladni_rectangle_run(&request, segments, 0, &result) == CHL_RECTANGLE_INVALID,
          "zero storage rejected");
    check(chladni_rectangle_run(&request, segments, 32, NULL) == CHL_RECTANGLE_INVALID,
          "null result rejected");
    printf("rectangle contract v1: %u checks PASS\n", checks);
    return 0;
}
