# CYGNO-MC

Geant4 detector simulation for CYGNO.

Low-energy ion nuclear stopping/scattering uses ZBL screened collisions with local recoil deposition; the TRIM response is integrated per hit and NR step caps are increased, preserving option4 electronic stopping and ROOT output.

## Build and run

Requires ROOT 6 and Geant4; CADMesh 2 is bundled.

| Tested environment | Geant4 | ROOT |
| --- | --- | --- |
| Local | 11.4.0 | 6.36.08 |
| CNAF | 11.2.0 | 6.30.02 |

Load the ROOT/Geant4 environments; set `Geant4_DIR`/`ROOT_DIR` to their CMake package directories if needed. From a common parent directory:

```sh
git clone git@github.com:CYGNUS-RD/CYGNO-MC.git
git clone git@github.com:CYGNUS-RD/geometry.git
cmake -S CYGNO-MC -B CYGNO-MC-build
cmake --build CYGNO-MC-build -j
cd CYGNO-MC-build
./CYGNO /path/to/run.mac
```

Run `./CYGNO` for the interactive interface. Commands: [CYGNOCommandsREADME.md](CYGNOCommandsREADME.md).
The current CMake configuration forces Debug builds.

## Geometry

Default: `../geometry/cygno04_v4_ASCII/`. Missing CAD uses the nominal analytic detector; incomplete CAD reports an error.
Select legacy v3 with `CYGNO_GEOMETRY_PATH=../geometry/cygno_04_v3_ASCII ./CYGNO run.mac`.
Isotope input examples are in `backgrounds/`.

## CNAF jobs

Follow [mycondor: Submit job to CNAF](https://github.com/CYGNUS-RD/mycondor#submit-job-to-cnaf).
`master` with the v4 geometry has been tested on the CNAF queue, including ROOT output retrieval, without source-code changes.
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

## PBS jobs

`scripts/submit_jobs_lngs.py` (Python 3) submits to the `cygno` PBS queue at LNGS; site paths are in `scripts/config_lngs.py` and the worker environment is set at the top of the script.
Each job gets a copy of the macro template with its own event count, output name and random seeds (`/random/setSeeds`).
Basic PBS commands: <https://www.bo.infn.it/alice/introgrd/pbsabout/node17.html>

Split 100M events into 100 jobs:

```sh
python3 scripts/submit_jobs_lngs.py -m CYGNOtest_surface_gamma --tag ext_gamma -n 100000000 -e 1000000 --builddir ../CYGNO-MC-build/ --outdir ./
```

* `-m` macro template in `macro/`, without the `.mac` extension
* `-n` total number of events
* `-e` events per job
* `--tag` simulation tag; outputs go in a directory with this name
* `--builddir` directory containing the `CYGNO` executable
* `--outdir` output path

Output is saved in `<outdir>/pbs_outputs/<tag>`; per-job logs and macro copies go in `pbs_logs/` and `pbs_workdir/`.
Use `--retry` with the same tag to resubmit jobs whose output is missing.

Submit one job set per isotope from a list, e.g. the camera lens radioactivity; each isotope's NEvents is split into jobs of `-e` events:

```sh
python3 scripts/submit_jobs_lngs.py -m CYGNO04_camera_lens --tag CYGNO04_CameraLens -f CameraLensActivity.txt -e 100000 --builddir ../CYGNO-MC-build/
```

`-f` names a file in `backgrounds/` with columns Name, Z, A, NEvents (optionally IonicCharge and ExcitationEnergy[keV] before NEvents, for metastable states); `-n` is then ignored.
Its name must start with a material key of `MATERIALS` in `config_lngs.py` or contain `Activity`; the prefix names the outputs.
The template sets the source volume with `/gps/pos/confine`; `/gps/ion`, `/run/beamOn`, `/CYGNO/outfile` and `/random/setSeeds` are rewritten per job.
