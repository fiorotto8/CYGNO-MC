# TestEm7 dependency

These seven implementation/header files are vendored from Geant4 **v11.4.0**,
`examples/extended/electromagnetic/TestEm7`, with their original notices and
the Geant4 license retained. They are required because the screened process
is an example implementation, not an installed Geant4 library component.

Upstream: https://github.com/Geant4/geant4/tree/v11.4.0/examples/extended/electromagnetic/TestEm7

Only `G4ScreenedNuclearRecoil.cc` differs from upstream:

1. In `DoScreeningComputation`, evaluate the same analytic ZBL potential beyond
   the finite interpolation table during scattering quadrature. In-domain
   interpolation and cross-section construction are unchanged. This local
   continuation is for the **ZBL configuration used by CYGNO**, not a general
   continuation for the other upstream screening keys.
2. In `PostStepDoIt`, include `incidentEnergy < lowEnergyLimit` in the terminal
   condition, matching `GetMeanFreePath`, before accessing collision handlers.

The application supplies the actual 0.1 eV physics cutoff in the constructor;
using only the upstream setter would leave its initial low-energy limit stale.
No other TestEm7 physics list, detector, primary generator or actions are copied.

See [the technical note](../../docs/screening-technical-note.md) for the
application changes, qualified energy interval and comparison with TRIM.
