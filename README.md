# Chladni — standing waves meet billiard geometry

The first shared example with [isomorphisms/billiard](https://github.com/isomorphisms/billiard)
is the **same rectangle**, viewed in two different ways:

- A particle travels in straight lines and reflects specularly from the walls.
  Unfolding a rectangular billiard produces straight-line motion on a flat torus;
  the rectangular torus has modulus `tau = i * height / width`.
- A vibrating field has **nodal lines**, the points where an eigenfunction is zero.
  The same rectangle and its dimensions determine which standing waves exist.

## Wavefronts and billiard rays: the direct bridge

A propagating wavefront is a level curve of its phase `S(x,y,t) = constant`.
In an isotropic homogeneous medium, the ray direction is parallel to
`grad(S)`, hence **perpendicular to the wavefront**. At a specular wall with
unit normal `N`, the wavevector (and ray direction) reflects as

```
k_out = k_in - 2 * dot(k_in, N) * N
```

This follows from conserving the tangential phase gradient at a stationary
flat wall. Billiard trajectories are thus the geometrical-optics rays of
propagating waves, not an unrelated analogy. Unfolding the rectangle straightens
the reflected rays and the corresponding local phase fronts.

Even the rectangle's standing mode below admits an **exact four-plane-wave
decomposition**. With `kx = m*pi/a`, `ky = n*pi/b`, the spatial mode is

```
sin(kx*x) sin(ky*y) = [cos(kx*x - ky*y) - cos(kx*x + ky*y)] / 2
```

Multiplying by `cos(omega*t)` resolves it into four traveling plane waves,
whose wavevectors are the sign combinations `(+/-kx, +/-ky)`. Their ray
directions are precisely the ones obtained by specular reflection in the
rectangle. The nodal figure comes from **interference among these waves**:
a stationary nodal line is not itself one traveling wavefront.

The ray picture is geometrical optics (or a phase decomposition for the exact
rectangle), *not* a complete solver for finite-wavelength boundary effects.
A free-edge elastic plate has fourth-order bending dynamics and can involve
additional boundary phenomena. The dispersion changes frequency and speed:
for an isotropic thin bending plate, `omega = sqrt(D/(rho*h))*|k|^2`, so
group velocity remains parallel to `k` but depends on its magnitude.

Translation-surface moduli and Mirzakhani's work enter through the rational
polygon billiard side; they do not directly calculate the sand patterns.

## Exact first model: simply supported rectangular plate

For width `a`, height `b`, and positive integers `m,n`, put

```
phi_mn(x,y) = sin(m*pi*x/a) sin(n*pi*y/b)
k_mn^2      = pi^2[(m/a)^2 + (n/b)^2]
-Delta phi_mn = k_mn^2 phi_mn
```

For a tensioned membrane fixed at the edge, the frequency scales as `k_mn`.
For a simply supported *thin elastic plate* with bending stiffness `D` and
mass per area `rho*h`, the interior equation is

```
D * Delta^2 w + rho*h * d^2w/dt^2 = 0
omega_mn = sqrt(D/(rho*h)) * k_mn^2
D = E*h^3 / [12*(1-nu^2)]      (homogeneous isotropic plate)
```

The same `phi_mn` satisfies the simply supported plate conditions: zero
transverse displacement and zero bending moment along the four edges. It is not
a general model of a **free-edge Chladni plate**; free edges instead require
vanishing bending moment and effective transverse shear, with different modes.
A real sand experiment also needs a forcing, damping, support and particle-motion
model. None is claimed here.

On a square, `phi_12` and `phi_21` have the same eigenvalue. The normalized sum
and difference remain eigenfunctions; the difference vanishes along the diagonal.
On a nonsquare rectangle this generally ceases to be true, so the code refuses
an unequal-eigenvalue pair rather than calling it a normal mode.

## Portable numerical kernel (C99)

Host build and run without graphical dependencies:

```sh
cc -std=c99 -O2 src/rectangle_modes.c src/main.c -lm -o chladni
./chladni 1 1 1 2 480 480 antisymmetric > diagonal.pgm
./chladni 2 1 3 2 640 320 > rectangle.pgm
sh tests/run.sh
```

The first six arguments are `width height m n image_columns image_rows`.
Optional seventh argument: `single`, `symmetric`, or `antisymmetric`.
Output is a binary PGM image on stdout (white near the field's zero set), with
`k^2` printed to stderr. This visualizes mode nodal proximity, **not sand
motion**. The pure mode functions live in `src/rectangle_modes.[ch]` and can be
called by a future raylib or Android renderer without adopting the CLI.

All spatial coordinates match `billiard`: top-left origin, x right, y down.
Resizing the billiard rectangle must change the eigenvalues and recompute the
wave field. Keep geometry truth independent of rough/hand-drawn rendering.

## Extension path

1. Link the draggable-wall rectangle of `billiard` to this mode kernel. Show a
   **propagating wavefront orthogonal to its reflected billiard rays**, the four
   traveling-wave directions behind each standing mode, faint unfolded copies,
   the interference nodal plot, and the eigenvalue. Recalculate both when width
   changes. Retain a native C math core and a replaceable raylib/Android
   presentation layer.
2. Compare rectangular closed-orbit lengths with the mode spectrum. State the
   boundary model and distinguish exact formulas from semiclassical heuristics.
3. Add polygon domains and a numerical Laplace-eigenmode method for membranes or
   quantum billiards; independently add the **fourth-order** plate solver with
   explicit support and free-edge conditions. Do not substitute an ordinary
   Laplacian solver and label its output a free-edge plate.
4. For **rational polygons**, unfold trajectories to translation surfaces and
   explore their strata and `GL(2,R)` orbit closures. This is the direct route
   toward Eskin–Mirzakhani–Mohammadi, not a claim that their theorem calculates
   Chladni figures. Full moduli-space computations are not implemented here.

## Sources

- [Jim Woodhouse, *Plate vibration*](https://euphonics.org/3-2-3-plate-vibration/): thin-plate model and exact simply supported rectangle modes.
- [Jain & Samajdar, *Nodal portraits of quantum billiards*, Rev. Mod. Phys. 89 (2017)](https://doi.org/10.1103/RevModPhys.89.045005): nodal sets and classical dynamics.
- [Alex Wright, *From rational billiards to dynamics on moduli spaces*](https://public.websites.umich.edu/~alexmw/BilliardsToModuli.pdf): polygon unfolding and translation surfaces.
- [Eskin, Mirzakhani & Mohammadi, *Annals of Mathematics* 182 (2015)](https://annals.math.princeton.edu/2015/182-2/p07): orbit closures and equidistribution on moduli spaces.
