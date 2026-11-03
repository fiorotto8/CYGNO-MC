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

# Split jobs in the batch system

A set of scripts to split the simulation in multiple jobs is provided in the folder `scripts`.
Some command examples are provided for roma3 cluster, where PBS batch system is used.
Examples of basic commands of PBS can be found in https://www.bo.infn.it/alice/introgrd/pbsabout/node17.html (or simply google).

## Submit and check status of single job

Submit job:
```
qsub scripts/examplejob.sh
```
Check status:
```
qstat -u $USER
```


## Split jobs

Example to split 100M events into 100 jobs:
```
python scripts/submit_jobs_lngs.py -m CYGNOtest_surface_gamma --tag ext_gamma -n 100000000 -e 1000000 --builddir ../CYGNO-MC-build/ --outdir ./
```

Options meaning:

* `-m` name of the macro template. The script looks for the macro in the directory `macro/` (use macro name without `.mac` extension)
* `-n` total number of events to generate
* `-e` events per job
* `--tag` useful tag to identify the simulation. Output will be saved in a directory with this name
* `--builddir` path to dir containing CYGNO executable
* `--outdir` out path

The output is saved in `<outdir>/pbs_outputs/<tag-string>`
Also other directories will be created in `pbs_logs` and  `pbs_workdir` folders, containing respectively the logs and the copy of the macro for each job.

## Submit jobs with multiple macro configurations (for radioactive decays)

Example to send on batch multiple radioactive isotopes simulations:
```
python scripts/submit_jobs_rm3.py -m CYGNOtest --tag PbShieldRadioactivity -f U238Activity_Shield2Pb_10Pb2Cu.txt -e 1000000  --builddir /storage/local/home/cygnorm3/dimperio/CYGNO/CYGNO-MC-build/
```
Options meaning:
* `-f` configuration file for isotopes to be simulated. The script looks for the file in the `background/` directory. The file contains 4 columns: Name, Z, A, NEvents
* the other options are the same. Note that `-n` option is not necessary when `-f` option is used (and will be ignored), since the total number of events is in the configuration file
