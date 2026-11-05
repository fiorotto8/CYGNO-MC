# CYGNO-MC

Geant4 detector simulation for CYGNO.

Compared with `g4v11.3_cygno04`, this branch replaces low-energy ion nuclear stopping/scattering with ZBL screened collisions and local recoil deposition, integrates the existing TRIM response per hit, and increases NR step caps, preserving option4 electronic stopping and ROOT output.

## Build and run

Requires ROOT 6 and Geant4; CADMesh 2 is bundled.

| Tested environment | Geant4 | ROOT |
| --- | --- | --- |
| Local | 11.4.0 | 6.36.08 |
| CNAF (`g4v11.3_cygno04`, v4 geometry) | 11.2.0 | 6.30.02 |

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
The standard branch `g4v11.3_cygno04` (`5e777d0`) with v4 geometry passed 10- and 100-event CNAF runs, including ROOT output retrieval, without source-code changes.
This screening branch has not yet been validated on CNAF.
Before building, source `/cvmfs/sft.cern.ch/lcg/views/LCG_105/x86_64-ubuntu2204-gcc11-opt/setup.sh` to use the tested worker libraries. The tested worker image is `cygno-wn_v2.4.sif`.

Copy the templates from `/cvmfs/sft-cygno.infn.it/config/templates/` into `Launch/Sub_folder/`.
Prepare a job input directory containing `CYGNO`, `run.mac` and the compressed geometry:

```sh
tar -czf geo_v4.tar.gz -C /path/to/geometry cygno04_v4_ASCII
```

Create `run_mc.sh` there; the templates supply the worker software environment:

```sh
#!/bin/bash
set -e
tar -xzf geo_v4.tar.gz
export CYGNO_GEOMETRY_PATH="$PWD/cygno04_v4_ASCII"
export G4FORCENUMBEROFTHREADS=2
exec ./CYGNO run.mac > cygno_out.log 2>&1
```

Run `chmod +x run_mc.sh`. In `subber.sh`, set `EXECUTEPATH` to the input directory, `EXECUTE` to `run_mc.sh`, `CPUs=2`, and include the executable, macro and archive in `OTHERFILE`.
Set `FILENAMES` to the ROOT filename chosen in the macro and `cygno_out.log`, with a unique `BATCHNAME` and output `TAG` in an allowed bucket.
Set the thread count inside the worker script to match `CPUs`; submission-shell variables are not automatically passed to the worker.
Then submit and retrieve the job logs from `Launch/`:

```sh
cygno_htc -t
cd Launch
source Sub_folder/subber.sh
cygno_htc -q
cygno_htc -f JOBID CEID
```

The compressed v4 geometry used in these tests was about 4.3 MB and transferred successfully with the job inputs.
ROOT files and `cygno_out.log` go to the configured storage bucket/path; download them there separately from `cygno_htc -f`.
Check for `Geometry mode: CAD v4` in the log and the requested event count in ROOT tree `tree1`.
