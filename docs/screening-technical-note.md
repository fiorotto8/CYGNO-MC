# Minimal screened nuclear-recoil branch

21 October 2026. Branch: `g4v11.3_cygno04_screening`.
Base: `g4v11.3_cygno04`, commit `3f53d24ce433e304dd86f31ef9c5f16cefd50ee1`.

## Purpose and qualified scope

This branch installs the selected ZBL screened transport and its frozen TRIM
mean-response calibration in the application. It changes three existing files
(44 added lines), adds one response header, and vendors the seven necessary
TestEm7 implementation/header files plus their license and attribution. It does
not import the development physics lists, scan switches or diagnostic actions.
The branch name follows the existing naming convention: the build and numerical
qualification reported here use **Geant4 11.4.0 and ROOT 6.36.08 under WSL**.
Compilation and transport under Geant4 11.3 have not been demonstrated.

The qualified sample is single H1, He4, C12, O16 or F19 primaries, total kinetic
energy 0.25-1000 keV, isotropic at (0,0,245) mm, in the existing 900 mbar,
293.15 K He/CF4 detector gas. Geometry, composition, electronic stopping,
production cuts, source macros, energy-dependent step caps and 25 eV charged
tracking threshold are unchanged. Calibration coefficients are specific to
this configuration. Range and direction improve substantially on average;
high-energy heavy-ion ranges remain too short. This is agreement with a TRIM
simulation reference, not a validation against measured detector charge.

## What changes in the nuclear-recoil physics

The original branch constructs `G4EmStandardPhysics_option4`. It combines
electronic ionisation, condensed scattering and continuous nuclear stopping.
The new branch first constructs the same physics, then replaces the nuclear
transport processes on GenericIon, alpha, He3, proton, deuteron and triton.
Dynamic heavy ions inherit the GenericIon process configuration.

| Component | Original branch | Screening branch |
|---|---|---|
| Electronic ionisation / option4 settings | Existing option4 | Unchanged |
| Continuous `nuclearStopping` | Present | Removed on the ion species above |
| Low-energy multiple/Coulomb scattering | Existing option4 processes | Removed and replaced by screened elastic collisions |
| Nuclear elastic interaction | Original treatment | TestEm7 `G4ScreenedNuclearRecoil`, ZBL potential, explicit secondary recoils |
| Transfer/physics cutoff | Not the same parameterisation | 0.1 eV, supplied in constructor |
| Recoil/terminal cutoff | Existing transport settings | TestEm7 1 eV/u, in addition to unchanged 25 eV tracking threshold |
| Higher-energy scattering | Existing option4 | Urban MSC activation at 100 MeV total; screened maximum 100 MeV/u |
| Local custom QF | Existing TRIM response derivative | Same local prescription, followed by bounded event correction |

Explicit target recoils carry energy and direction into a cascade instead of
representing their transfer only through continuous nuclear stopping. Removing
the old nuclear stopping and scattering avoids overlapping low-energy nuclear
treatments. Electron, photon, decay and hadronic constructors are unchanged.
This is a hybrid with option4 ionisation; it is not wholesale replacement by
the TestEm7 example physics list.

The 100 MeV thresholds reproduce the tested prototype exactly. Geant4's Urban
activation and TestEm7 maximum use different energy conventions, so these
settings must not be interpreted as a validated high-energy handoff. All
reported tests are at or below 1 MeV total. Behaviour at higher energies and
background sources requires separate qualification.

Two small upstream fixes are included in the vendored screened source:

1. Scattering quadrature can request a potential outside the native finite
   interpolation domain. Above that domain the code evaluates the same ZBL
   analytic function, with coefficients 0.1818, 0.5099, 0.2802 and 0.0281 and
   exponents 3.2, 0.9432, 0.4028 and 0.2016 in reduced radius. In-domain
   interpolation is unchanged. This is a numerical continuation, not fitted
   screening strength. It supports this branch's ZBL configuration only.
2. `PostStepDoIt` now uses the same low-energy terminal condition as
   `GetMeanFreePath`, avoiding a collision-handler access after the latter
   selected a terminal step. Supplying the actual physics cutoff in the
   constructor also avoids the upstream setter leaving its initial low-energy
   limit stale.

## Quenching: what is fitted and what is preserved

The original hit prescription is retained: for response R(E)=E QF_TRIM(E),
apply dR/dE to nuclear deposits above 25 eV; use the existing low-energy
prescription below that threshold. The original hard-coded TRIM models are
unchanged. This local weighting does not automatically reproduce the TRIM
whole-event electronic fraction once explicit recoil cascades are generated.

For a qualified single-ion event let E0 be the actual generated primary kinetic
energy in keV, and S the sum of custom response weights in keV. Define:

```
f = S/E0
f_corrected = 1 - (1-f)^alpha_species(E0)
gain = f_corrected/f
```

The implementation uses `-expm1(alpha*log1p(-f))` for numerical stability;
zero and unity fractions use unit gain. Alpha is interpolated linearly in
log(alpha) versus log(E0), using the final frozen 28-node table for each ion.
The 140 fitted coefficients are embedded in `CYGNOScreeningResponse.hh`;
there are no runtime data-file paths or environment switches.

The same gain multiplies `energyDep_QF`, `energyDep_NRQF`, and both corresponding
custom per-hit vectors. It leaves raw deposited energies, positions, momenta,
track IDs, native Geant4 ionising/NIEL-related channels and primary truth
unchanged. Scalar/vector response sums remain consistent. Normalized charge
geometry within each event is preserved, including its centroid, normalized
covariance and head-tail fractions. The nonlinear event mapping changes yield
fluctuations; it does not establish a microscopic ionisation or Fano model.

Application is restricted to one vertex with one supported primary and E0 in
0.25-1000 keV. Other events retain the original local response. For eligible
events a nonfinite response or fraction outside [0,1] raises a fatal exception.
The correction is calibrated for contained recoil runs: it does not establish
the response of escaping tracks, other gas mixtures, multi-primary events or
neutron-induced recoil cascades. Those still undergo the modified transport,
but do not acquire a separately validated event response calibration.

The final dense fit used 300 fresh events/node/ion (42,000 training events).
It was tested on 27 fresh midpoint energies with 150 events/ion/energy
(20,250 events) and a further 1,000 log-uniform events/ion (5,000 events).
The earlier rejected five-anchor correction is not included. Midpoint median
absolute relative QF errors are 0.056%, 0.537%, 0.467%, 0.608%, 0.502% for
H, He, C, O, F; the largest individual midpoint error is 3.872%. Maximum
absolute spectrum-bin errors are respectively 1.96%, 2.71%, 4.87%, 1.54%,
2.02%. These are finite-sample checks, not uncertainty guarantees.

## Comparison with the original branch and TRIM

The independent matched comparison uses 200 events per energy, ion and model
at 0.25, 1, 10, 100 and 1000 keV: 5,000 scored events/model. Additional
25 and 100 eV controls are diagnostic and excluded from the scores. Initial
energies and directions are identical between models. The current-branch
control was run before integration; its source hashes are retained in the
development manifests. The new branch is checked by replay against the
selected prototype, retaining the actual branch physics and event response.

The table reports equal-weight mean absolute relative discrepancies from TRIM
for range and QF; the cosine discrepancy is absolute, not a percentage.

| Ion | Endpoint error original / screening (%) | Projected error original / screening (%) | Mean cosine error original / screening | QF error original / screening (%) |
|---|---:|---:|---:|---:|
| H1 | 41.65 / 4.52 | 44.91 / 4.73 | 0.0805 / 0.0177 | 2.07 / 0.18 |
| He4 | 27.27 / 14.51 | 28.86 / 12.31 | 0.0275 / 0.0163 | 1.98 / 0.60 |
| C12 | 48.87 / 10.51 | 57.37 / 12.07 | 0.2084 / 0.0201 | 1.81 / 0.43 |
| O16 | 50.23 / 14.11 | 62.93 / 14.19 | 0.3063 / 0.0156 | 1.89 / 0.65 |
| F19 | 48.90 / 9.81 | 63.91 / 10.15 | 0.3270 / 0.0201 | 2.13 / 0.89 |

Across ions the mean endpoint discrepancy falls from 43.38% to 10.69%, and
the cosine discrepancy from 0.1900 to 0.0180. These descriptive summaries
do not establish improvement at every energy or in every observable.
At 1 MeV the new endpoint/TRIM ratios are H 1.015, He 1.133, C 0.681,
O 0.680 and F 0.717: heavy-ion endpoints remain approximately 28-32% short.

Endpoint range is |x_stop-x_start|; projected range is the displacement dotted
with the initial unit direction. Bending cosine uses that same displacement,
not final momentum or detector reconstruction. Backward means negative
projection. The machine-readable [comparison table](screening-comparison.csv)
contains both models at each energy, MC SEM, TRIM reference information,
backward fractions with Wilson 95% intervals, longitudinal standard deviation,
transverse RMS, raw-energy budgets, steps and cascade track counts. In that
table `current` denotes the original branch and `candidate` this branch;
`QF_fixed_calibration` is the output response compared here, while
`QF_original` retains the pre-event-correction response. TRIM range means can
also be recovered as `endpoint_mm/endpoint_ratio` and analogously for projection.

Original raw-energy excess above E0*(1+0.0001) appears in 26/5,000 scored
events (0 H, 6 He, 8 C, 7 O, 5 F); the screened sample has zero such events.
QF fitting does not rescale or repair raw energy. The original budget issue
was traced in development to continuous nuclear stopping interactions.
Mean total step counts per event increase from 172/99/100/101/102 to
458/1139/3322/5062/6294 for H/He/C/O/F. These indicate transport cost,
not a wall-time benchmark.

The frozen range reference contains 195 points from 897 batches / 89,700
TRIM ions. Stopping-point spreads are reconstructed from cohort means, SEM
and N. Range error bars include finite Geant4 and TRIM statistics where
reported, excluding interpolation/model systematics. QF fit errors are
conditional on the frozen TRIM response and calibration scheme. TRIM electronic
energy includes excitation and is not directly measured collected charge.
Full local charge profiles, reconstructed angular resolution, head-tail
efficiency, yield fluctuations and curved track length are not independently
validated by agreement of these event means. TRIM sampled polyline length and
Geant4 true step-path length must not be equated.

## ROOT output and the 10 GB target

The original ROOT schema and full hit vectors are retained. Explicit recoils
greatly increase hit count: the screened full-hit 1,000-event log-uniform pilot
projected approximately 37, 114, 297, 503 and 572 GB per million events for
H/He/C/O/F. **This minimal branch does not meet 10 GB per million events.**

The earlier compact-output experiment projected 1.79, 2.46, 5.62, 8.16 and
7.91 GB/million for those ions. Those results required adaptive aggregation,
integer coordinates/charge fractions and removal of raw/native per-hit
vectors while retaining event totals and truth. They are 1,000-event pilot
extrapolations, not measured million-event files. That schema and its reader
integration are separate work and are deliberately absent from this minimal
physics patch. Do not use the packed projections as this branch's file size.

## Exact code inventory and build

* `CMakeLists.txt`: three lines compile the two vendored TestEm7 source files
  and expose their private include directory.
* `src/CYGNOPhysicsList.cc`: 34 added lines include/register the hybrid screened
  ion processes after the standard constructors, removing overlapping processes.
* `src/CYGNOEventAction.cc`: seven added lines include the calibration helper
  and apply its uniform gain before writing existing response scalars/vectors.
* `include/CYGNOScreeningResponse.hh`: single event helper and frozen alpha table.
* `external/TestEm7/`: seven required files, license and provenance README;
  only the screened source has the two numerical changes listed above.
* `docs/`: this note and the 50-row original/screening comparison dataset.

Build with the existing Geant4/ROOT environment; no additional dependency is
introduced beyond the included TestEm7 sources:

```sh
cmake -S . -B build_screening -DWITH_GEANT4_UIVIS=OFF
cmake --build build_screening --parallel 3
```

Use the existing recoil macros and geometry setup. Switching branches must use
a separate or rebuilt executable; an old executable does not acquire screening
from the macro alone. There is no physics-selection runtime flag in this branch.

## Integration verification

Both the normal production target and an observed build compiled successfully.
A 7,000-event replay (1,400 per ion, including the low-energy diagnostics)
reproduced every primary tracking CSV field exactly against the selected
prototype. Raw deposited energies and native Geant4 response were bitwise
identical. The embedded response matched the frozen table calculation within
2e-14 numerical tolerance. The observed build retained this branch's actual
physics registration, vendored process and calibrated event action; diagnostics
replaced source sampling/output booking and added read-only tracking/energy
callbacks. Replays verify integration and are not additional independent
physics statistics. The 50 scored comparison rows are included alongside this
note so its numerical comparison does not depend on generated build directories.
An additional 20-event log-uniform run per ion used the actual production
executable and full ROOT booking: all 100 events passed scalar/vector energy
sum checks and the bounded response check. These small runs verify output
integration; they do not measure a million-event file size.

Upstream source: [Geant4 v11.4.0 TestEm7](https://github.com/Geant4/geant4/tree/v11.4.0/examples/extended/electromagnetic/TestEm7).
Original notices and license are included in `external/TestEm7`.
