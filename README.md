# CYGNO-MC

Geant4 detector simulation for CYGNO.

Physics: EM option4 electronic stopping; low-energy ions add ZBL-screened nuclear scattering (`external/TestEm7`) with local recoil deposition, and nuclear-recoil hits get an SRIM/TRIM quenching factor.
Notes: [screening](docs/cygno04_screening_technical_note.pdf), [TRIM comparison](docs/cygno_mc_trim_comparison.pdf).

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
v4 includes the 16 Hamamatsu R7378A PMTs and E2924-500 socket assemblies (`cad_PMT_physical`, `cad_PMTSocket_physical`): borosilicate glass and acrylic (Perspex) compositions at effective densities giving the measured masses, 16.9 g per PMT and 72.5 g per socket including its cables.
Select legacy v3 with `CYGNO_GEOMETRY_PATH=../geometry/cygno_04_v3_ASCII ./CYGNO run.mac`.
Isotope input examples are in `backgrounds/`.

## CNAF jobs

Follow [mycondor: Submit job to CNAF](https://github.com/CYGNUS-RD/mycondor#submit-job-to-cnaf).
This workflow was tested on the CNAF queue with the v4 geometry, including ROOT output retrieval, before the PMT, socket and PMMA-window update.
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

The compressed v4 geometry (about 7 MB; 4.3 MB in the tested runs) transfers with the job inputs.
ROOT files and `cygno_out.log` go to the configured storage bucket/path; download them there separately from `cygno_htc -f`.
Check for `Geometry mode: CAD v4` in the log and the requested event count in ROOT tree `tree1`.

### Many jobs (background campaigns)

First prepare one macro per job with the PBS script in debug mode, which writes the macros (isotope, events, output name, random seeds) without submitting:

```sh
python3 scripts/submit_jobs_lngs.py -d -m CYGNO04_camera_lens -f CameraLensActivity.txt -e 100000 \
  --tag CYGNO04_CameraLens --codedir . --builddir ../CYGNO-MC-build --outdir /path/to/work -t /path/to/work/tmp/
```

Then submit one CNAF job per macro; each job folder gets the macro, `CYGNO`, the geometry archive and a `run_mc.sh`, and a filled copy of `subber.sh` is sourced from `Launch/`:

```sh
scripts/submit_cnaf.sh -m /path/to/work/pbs_workdir/CYGNO04_CameraLens -s Launch/Sub_folder/subber.sh \
  -x ../CYGNO-MC-build/CYGNO -g geo_v4.tar.gz -T From_queue/<user>/CYGNO04_CameraLens -w cnaf_jobs -k
```

`-k` only prepares the folders; check one `Launch/Sub_folder/subber_<job>.sh`, then rerun without `-k`.
Each job uploads `<job>.root` and `<job>.log` to the bucket path given by `-T`; `-c` sets CPUs and Geant4 threads (default 2).
Submitted jobs are listed in `cnaf_jobs/submitted.txt` and skipped on reruns; after `cygno_htc -f JOBID CEID`, `Launch/return_<job>.txt` holds the job's exit code (0 = success).

## PBS jobs

`scripts/submit_jobs_lngs.py` (Python 3) submits to the `cygno` PBS queue at LNGS; site paths are in `scripts/config_lngs.py` and the worker environment is set at the top of the script.
Each job gets a copy of the macro template with its own event count, output name and random seeds (`/random/setSeeds`).
Basic PBS commands: <https://www.bo.infn.it/alice/introgrd/pbsabout/node17.html>

Split 100M events into 100 jobs:

```sh
python3 scripts/submit_jobs_lngs.py -m CYGNO04_camera_lens --tag camera_lens_K40 -n 100000000 -e 1000000 --builddir ../CYGNO-MC-build/ --outdir ./
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
The template sets the source volume with `/gps/pos/confine` (e.g. `camera_lens_L`, `cad_PMT_physical`); `/gps/ion`, `/run/beamOn`, `/CYGNO/outfile` and `/random/setSeeds` are rewritten per job. The `-k/--confine` option is parsed but not applied.
