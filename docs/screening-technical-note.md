# Screened NR transport within the existing ROOT budget

Branch: `g4v11.3_cygno04_screening`, 21 October 2026.
Original control: `g4v11.3_cygno04`, commit `3f53d24ce433e304dd86f31ef9c5f16cefd50ee1`.
All builds/runs reported here used **Geant4 11.4.0 and ROOT 6.36.08 under WSL**.
The historical branch name does not establish a tested Geant4 11.3 build.

[Technical PDF](cygno04_screening_technical_note.pdf),
[detailed comparison PDF](cygno_mc_trim_comparison.pdf) and
[comparison CSV](screening-comparison.csv) describe the current implementation.
They supersede the earlier fully tracked screening results for this branch.

## Required output contract

**The original ROOT format is preserved, with one hit per native depositing
Geant4 step.** Branch names, types, units and vector alignment match the original.
There is no aggregation, packing, quantization, hit thinning or new reader.
The ROOT writer, sensitive detector, event action and primary generator are
identical to the original branch. `numhits` still counts native depositing
steps. Raw energies are never rescaled to enforce quenching.

The target is 10 decimal GB per ion per million events of the existing
0.25-1000 keV log-uniform NR source. The earlier fully tracked screening
prototype required approximately 37-572 GB/million in this format. That is
not the model now installed. The rejected compact-output implementation is
absent from this branch.

## Physics model and compromise

Option4 electronic ionisation and the existing electromagnetic, decay and
hadronic constructors remain. On GenericIon, alpha, He3, proton, deuteron and
triton, continuous `nuclearStopping` and original low-energy multiple/Coulomb
scattering are replaced by TestEm7 ZBL screened elastic scattering.

For affordability, **target recoil energy is deposited locally using TestEm7's
native local-deposition treatment; explicit target-recoil cascade tracks are
disabled**. Projectile energy loss and deflection are still simulated by
screened collisions. This is a primary-ion transport approximation, not the
earlier fully tracked cascade. Secondary electrons and secondaries from other
registered processes are not globally disabled.

TestEm7's default `G4LindhardRobinsonPartition` splits the local recoil deposit
between electronic loss and NIEL. The stored native Geant4 energy channels
therefore remain separate from CYGNO's custom TRIM response below.

The cross-section/mean-free-path tables use an energy-dependent transfer
resolution parameter, with E the total projectile kinetic energy:

```
Tcut(E) = min(100 eV, max(0.1 eV, 0.001 * E))
```

| Projectile energy | Transfer parameter |
|---|---:|
| 0.25 keV | 0.25 eV |
| 1 keV | 1 eV |
| 10 keV | 10 eV |
| 100 keV | 100 eV |
| 1000 keV | 100 eV |

This controls approximate cross-section table construction and collision
sampling, not an exact sharp lower bound on every sampled transfer. Fine
sampling at low energy preserves low-energy direction/range while coarser
high-energy sampling reduces actual native steps. ZBL potential coefficients
and screening lengths are not fitted.

The constructor low-energy limit remains 0.1 eV, its terminal cutoff 1 eV/u,
and the existing charged tracking threshold 25 eV. The maximum-step law remains
`cap * min(1,sqrt(E/40 keV))`, without a spatial floor. The NR macros use:

| Ion | Original cap (um) | Screening cap (um) |
|---|---:|---:|
| H1 | 52 | 208 |
| He4 | 26 | 208 |
| C12 | 13 | 52 |
| O16 | 6.5 | 26 |
| F19 | 3.25 | 13 |

Every saved record is still a native Geant4 step. The prototype's Urban MSC
activation at 100 MeV total and screened maximum at 100 MeV/u are retained;
their differing energy conventions are not a validated high-energy handoff.
Qualification is limited to the stated NR range at or below 1 MeV total.

## TRIM response: integrate the existing derivative

The original hard-coded TRIM mean-response curves remain. For
`R(E) = E * QF_TRIM(E)`, replace the original finite-step approximation
`deposit * dR/dE(Epre)` by:

```
custom_hit_response = R(Epre) - R(max(0, Epre - local_raw_deposit))
```

This integrates the same derivative over the finite deposit. Very small
deposits use the midpoint derivative to avoid subtraction loss. The existing
sub-25 eV extrapolation remains; a terminal deposit gives `R(Epre)`, matching
the mean-QF prescription. ER hits retain raw energy as before.

The earlier screened prototype's fitted 28-node event-gain tables and uniform
event correction are removed. No new response fit is needed. Custom scalars
and hit vectors are summed through the original event action. Raw and native
Geant4 ionising/NIEL-derived channels keep their original meanings.

For a contained primary-only recoil with entirely local deposition, increments
telescope to `R(E0)`. Close agreement with the frozen TRIM QF curve is therefore
**by construction, not an independent microscopic prediction**. Small departures
include effects of separately tracked electronic secondaries. Numerical
step-size response fluctuations are removed; an intrinsic NR yield-fluctuation
or Fano model is not supplied by this prescription.

## Fresh matched comparison

The final test uses **10,000 fresh events per ion per model**, 100,000 events
total. Source arrays (energies, positions, directions and IDs) match bitwise.
The control is built from an immutable archive of the base commit. Both use
the original full ROOT schema and a read-only primary tracking observer.

Single H1, He4, C12, O16 or F19 primaries have log-uniform total energy
0.25-1000 keV and isotropic direction at (0,0,245) mm in the existing 900 mbar,
293.15 K He/CF4 gas. All primary tracks stop inside sensitive gas. Geometry,
composition, source distributions and production cuts are unchanged; transport
resolution changes are listed above.

The scores equally weight ten logarithmic energy bins. Every event is divided
by the frozen TRIM expectation at its own energy. These descriptive mean
absolute discrepancies are not significance-weighted goodness-of-fit tests.
They are not directly interchangeable with the earlier five-energy scores.

| Ion | Endpoint error original / screening (%) | Projected error original / screening (%) | Mean cosine error original / screening |
|---|---:|---:|---:|
| H1 | 34.52 / 5.77 | 37.96 / 3.30 | 0.0711 / 0.0183 |
| He4 | 21.10 / 21.23 | 23.73 / 20.10 | 0.0349 / 0.0094 |
| C12 | 48.52 / 12.52 | 54.69 / 13.04 | 0.1684 / 0.0149 |
| O16 | 51.00 / 11.51 | 58.81 / 12.56 | 0.2165 / 0.0156 |
| F19 | 51.80 / 9.88 | 60.24 / 10.99 | 0.2407 / 0.0169 |

Endpoint agreement improves for H/C/O/F; He endpoint agreement is essentially
unchanged. Bending improves in these descriptive scores. Residual errors
remain, and average improvement does not imply agreement at every energy.
Endpoint bending concerns displacement relative to the initial direction,
not final momentum or a reconstructed detector angle.

In the highest energy bin (geometric means about 0.66 MeV), endpoint/TRIM
ratios are H 1.009, He 1.164, C 0.695, O 0.696 and F 0.749. C/O endpoints
therefore remain approximately 30% short and F about 25% short in that bin.

The original sample has 235/50,000 raw sensitive-gas deposits above
`E0 * 1.0001`, reaching 8.12% excess in the largest F event. The new sample has
zero such events, with no raw-energy rescaling. Continuous nuclear stopping
was implicated in the original budget excess during earlier diagnostics.

## File-size qualification

Actual 10,000-event files, scaled to one million events:

| Ion | Original GB/million | Screening GB/million |
|---|---:|---:|
| H1 | 9.638 | 8.786 |
| He4 | 9.217 | 9.494 |
| C12 | 9.335 | 7.545 |
| O16 | 8.955 | 7.335 |
| F19 | 8.923 | 7.354 |

All new pilot projections are below 10 GB; He has about 5% margin. These are
measurements of full-schema files followed by sample-size extrapolation,
**not measured million-event files or an arbitrary-spectrum size guarantee**.
A high-energy-concentrated sample or changed caps requires another size check.
No output compaction or special decoder contributes to these numbers.
Mean native hit counts are approximately 78, 82, 65, 63 and 63 for H/He/C/O/F.
Step counts are not a wall-time benchmark.

## Checks, uncertainty and limits

All ROOT branch names and types match the original. All six event energy
channels match their hit-vector sums. IDs, vector lengths, raw budgets and
contained stopping are checked for all 50,000 new events. Source arrays match
bitwise between models. Both ordinary and observed production targets build.

The frozen TRIM range reference has 195 energy/species points from 897 batches
and 89,700 ions. Range ratio errors include finite Geant4 statistics and
conditional TRIM mean uncertainty, excluding interpolation/model systematics.
TRIM mixture spread curves include the variation of energy inside each bin.
Backward fractions use Wilson 95% intervals. Geant4 true curved path is not
equated with the sampled TRIM polyline.

Local target-energy deposition omits explicit secondary-recoil branches and
their spatial correlations. Good primary ranges/bending and imposed TRIM mean
QF do not independently validate charge-cloud morphology, head-tail efficiency,
reconstructed angular resolution or intrinsic yield fluctuations. The report
compares charge-axis/head-tail quantities between models; the frozen reference
does not supply per-event 3D charge clouds for their TRIM validation. A detector
response assessment remains necessary.

## Code inventory and build

* `src/CYGNOPhysicsList.cc`: screened ion transport replaces overlapping nuclear
  stopping/scattering; target recoil energy is locally deposited.
* `external/TestEm7/G4ScreenedNuclearRecoil.cc`: adaptive transfer parameter,
  plus the existing ZBL quadrature continuation and consistent terminal-energy
  fixes. Other required TestEm7 sources/headers and license are retained.
* `src/CYGNOHit.cc`: finite-step integral of the original TRIM response.
* Five `macro/nuclear_recoils_4pi/*.mac` files: revised maximum-step caps.
* `CMakeLists.txt`: compile two vendored TestEm7 implementation units.
* The former event-calibration header is removed. ROOT writing, hit creation,
  event output and primary generation have no changes from the original branch.

```sh
cmake -S . -B build_screening -DWITH_GEANT4_UIVIS=OFF
cmake --build build_screening --parallel 3
```

Use the existing ROOT reader and the branch's NR macros. Switching branches
requires a rebuilt or separate executable. Production has no scan environment
variables or compact-output mode.

Upstream: [Geant4 v11.4.0 TestEm7](https://github.com/Geant4/geant4/tree/v11.4.0/examples/extended/electromagnetic/TestEm7).
Original notices and license are retained in `external/TestEm7`.
