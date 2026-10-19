# Shared rectangle contract, version 1

`src/rectangle_experiment.h` is the versioned producer/consumer interface.
It calls the existing Chladni mode kernel and the external Billiard collision
kernel. A single request supplies both widths/heights, the launch point and
direction, positive mode indices, combination, boundary label, sample point,
plate scale and ray-distance budget. Every length uses the same unit.
Coordinates have their origin at the top-left; x increases rightward and y
downward. Launch angles retain Billiard's counterclockwise y-up convention.

Each call starts a fresh ray. Its output contains a copy of every semantic
input, and `chladni_rectangle_result_matches` checks that copy without relying
on struct padding. Consumers must require both `valid` and a matching request
before showing output. Failure invalidates the prior output. Segment capacity
truncation is reported separately from completing the requested distance.
The request must be immutable during a call, and request/result storage must
not overlap. Segments are caller-owned and are unusable after a failed call.

`CHL_DIRICHLET` reports a spatial mode and eigenvalue, with scale zero and no
plate frequency. `CHL_SIMPLY_SUPPORTED` requires the positive finite scale
√(D/(ρh)) and reports angular frequency equal to that scale times k².
`CHL_FREE_EDGE` returns unsupported. Unknown labels, invalid geometry, outside
or edge launch, invalid indices, nondegenerate mixtures and nonrepresentable
required quantities fail. Free-edge output is never substituted.

For each pair of signs s,t ∈ {−1,+1}, one separated mode has wavevector
(s mπ/width, t nπ/height) and complex coefficient −st/4. Adding those four
complex waves reconstructs the existing sine product; the imaginary parts
cancel. A normalized degenerate pair exposes eight components, preserving
the second mode's sign. Each component's phase front is perpendicular to its
wavevector. Reflection at an x-wall flips the horizontal component, and at
a y-wall flips the vertical component. The collision owner supplies those
reflections. A freely adjusted launch can have another direction: on 2×1,
the fundamental mode's component slopes have magnitude 2, while a 45° launch
has magnitude 1. Neither trajectories nor phase fronts are nodal lines.

## Source ancestry and qualification

This is a stacked follower of the unchanged initial Chladni mode implementation
at `e40d86b882abf3c3c226d92885dd573674b20413`. Its ray dependency is
`isomorphisms/billiard@b2aaed84cb24bf3d3510c6ed5e4b3baa09f3b02a`.
The latter keeps the same rectangle engine as the original assignment pin
and adds unrelated pixel-profile evidence. Neither engine is copied.

The existing ai-ci host action is pinned at
`4ea071a96239f3a29ca6d98454feb59947d87cfe`; it builds and qualifies
ICK `c61e448251744a2f40ad743ebef1a027bdcd2f9d` with reference
`6294f1d9e7536e5ffcde09d1528c918d63abfef5`. The workflow checks out the
PR head explicitly, not the synthetic merge commit. `make test receipt` uses
the stage's actual `ICK_CC`, `ICK_SOURCE_DIR` and `BILLIARD_SOURCE_DIR`.
Per-translation-unit compiler invocations appear in the run log; artifacts
retain exact source SHAs, compiler/cc1 digests, executable digests and test
outputs. Missing/stock compiler controls must fail with the specified reason.
Source pins and an executable named gcc alone do not establish qualification;
the pinned action's actual build and arithmetic qualification are required.

The original mode tests and PGM CLI are reused. CLI smoke checks moved from
the stock-compiler shell runner into `tests/cli_test.c`, compiled with the
same ICK. The old runner was removed. The separate contract test computes
analytical expectations without calling the mode kernel for expected values,
reconstructs complex waves with cos/sin, and tests Billiard reflection output.
It is an executable numerical oracle, not mathematical proof or a claim that
a separate model worker was dispatched.

Required controls include 1×1 and 2×1 geometry, fixed-position resize,
frequencies, square degeneracy, four/eight-wave reconstruction, edge zeros,
reflection signs, invalid requests, unsupported free edges, truncation, stale
results, overflow and rejected compiler substitution. The phase overflow fix
divides x by width before multiplying by π and the mode index. Wave-number
squares and frequencies outside finite positive double range return NAN.
Swapped distinct modes are combined only when the stored width equals height
exactly; a near-square tolerance must not admit a nondegenerate mixture.

Scope is ICK host source and behavior. No APK, emulator, device, free-edge
solver, sand experiment, full moduli-space classification or physical claim
follows from these checks. The user's direct one-off instruction overrides
this assignment's original Flexible Pipes admission requirement.
