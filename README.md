# CYGNO-MC

Geant4 detector simulation for CYGNO.

Compared with `g4v11.3_cygno04`, this branch replaces low-energy ion nuclear stopping/scattering with ZBL screened collisions and local recoil deposition, integrates the existing TRIM response per hit, and increases NR step caps, preserving option4 electronic stopping and ROOT output.

## Build and run

Requires ROOT 6 and Geant4; CADMesh 2 is bundled. Locally tested with ROOT 6.36.08 and Geant4 11.4.0.
Load the ROOT/Geant4 environments; set `Geant4_DIR`/`ROOT_DIR` to their CMake package directories if needed. From a common parent directory:

```sh
git clone --branch g4v11.3_cygno04_screening git@github.com:fiorotto8/CYGNO-MC.git
git clone git@github.com:fiorotto8/geometry.git
cmake -S CYGNO-MC -B CYGNO-MC-build
cmake --build CYGNO-MC-build -j
cd CYGNO-MC-build
./CYGNO /path/to/run.mac
```

Run `./CYGNO` for the interactive interface. Commands: [CYGNOCommandsREADME](CYGNOCommandsREADME).
The current CMake configuration forces Debug builds.

## Geometry

Default: `../geometry/cygno04_v4_ASCII/`. Missing CAD uses the nominal analytic detector; incomplete CAD reports an error.
Select legacy v3 with `CYGNO_GEOMETRY_PATH=../geometry/cygno_04_v3_ASCII ./CYGNO run.mac`.
Isotope input examples are in `backgrounds/`.

## CNAF jobs

Follow [mycondor: Submit job to CNAF](https://github.com/CYGNUS-RD/mycondor#submit-job-to-cnaf).
Copy the templates from `/cvmfs/sft-cygno.infn.it/config/templates/` into `Launch/Sub_folder/`.
Edit `subber.sh` for the executable, macro arguments, CPUs and output files, then:

```sh
cygno_htc -t
cd Launch
source Sub_folder/subber.sh
cygno_htc -q
cygno_htc -f JOBID CEID
```

Stage v4 geometry separately: it exceeds the guide's approximate 10 MB upload limit.
Set a worker-visible absolute `CYGNO_GEOMETRY_PATH` and match `G4FORCENUMBEROFTHREADS` to requested CPUs.
