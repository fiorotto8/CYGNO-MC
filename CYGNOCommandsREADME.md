# CYGNO commands

The geometry is built at startup, before any macro runs. The laboratory, shielding and gas
commands below only store values; `/CYGNO/updateGeo` applies them, but it crashes
multithreaded runs (tested with Geant4 11.4). In practice the startup defaults apply:
`NoCave`, `FullShield` with four 1 cm Air layers, and He/CF4 60/40. To change them, edit
the constructor defaults in `src/CYGNODetectorConstruction.cc` and `gasHeFrac` in
`src/CYGNODetectorMaterial.cc`.

## Laboratory

```text
/CYGNO/lab/select LNGS|NoCave                 Default: NoCave
/CYGNO/lab/externalrockthick <value> <unit>
/CYGNO/lab/internalrockthick <value> <unit>
/CYGNO/lab/productionlayerthick <value> <unit>
```

## Shielding

```text
/CYGNO/shield/select FullShield|NoShield
/CYGNO/shield/thick0 <value> <unit>           Layers numbered outside to inside
/CYGNO/shield/thick1 <value> <unit>
/CYGNO/shield/thick2 <value> <unit>
/CYGNO/shield/thick3 <value> <unit>
/CYGNO/shield/mat0 <material>                 Pb, PE, Cu, Air, Water or Steel
/CYGNO/shield/mat1 <material>
/CYGNO/shield/mat2 <material>
/CYGNO/shield/mat3 <material>
/CYGNO/shield/inner_radius <value> <unit>     Currently no effect (value is stored but not used)
/CYGNO/shield/inner_height <value> <unit>     Currently no effect (value is stored but not used)
```

These commands configure analytic outer shielding, not the CAD copper shells.

## Gas

```text
/CYGNO/gas/frac_He <fraction>
/CYGNO/gas/frac_CF4 <fraction>
```

Supply molar fractions summing to 1; gas is defined at 900 mbar and 293.15 K.

## Output and tracking

```text
/CYGNO/outfile <name>                         ROOT output filename
/CYGNO/primaryGenerator/fillNtuple true|false Default: true; primary rows only
/CYGNO/stepmax <value> <unit>                 Step cap for charged particles (> 0)
```

For protons and ions the `stepmax` cap shrinks below 40 keV by sqrt(E/40 keV).
Primary electrons are always capped at 5 um during their first mm.

Primary information is stored in `tree1`; events with sensitive hits are stored
in `nTuple`. Hit-vector columns are always created. Progress prints every 1000 events.
For multiple primaries, `tree1` describes the first primary of the first vertex.
Use one `/run/beamOn` per job: output trees are booked again at each run.

## Geometry

Select CAD at startup with `CYGNO_GEOMETRY_PATH` (see [README.md](README.md)).
`/CYGNO/pathtocad` also needs `/CYGNO/updateGeo`, so use `CYGNO_GEOMETRY_PATH` instead.
The macros `CYGNOtest*.mac`, `CYGNO_60_40_ER_10_keV.mac` and
`CYGNO_surface_gamma_50Water5Cu_inputdist50Water.mac` call `/CYGNO/updateGeo` and crash.
