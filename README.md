# Introduction

This repository hosts the Monte Carlo simulation for CYGNO experiment.

Compared with `g4v11.3_cygno04`, this branch replaces low-energy ion nuclear stopping/scattering with ZBL screened collisions and local recoil deposition, integrates the existing TRIM response per hit, and increases NR step caps, preserving option4 electronic stopping and ROOT output.

The prerequisite to run this simulations is to have ROOT (v6.X.X), GEANT4 (v11.2.X) and CADMesh (v2) software installed.
For more informations about the software see:
* ROOT:  https://root.cern.ch/ 
* GEANT4: http://geant4.cern.ch/
* CADMesh: https://github.com/christopherpoole/CADMesh


# Setup ROOT, GEANT4 and CADMesh

Setup all the environment variables of ROOT and GEANT4.

## Instructions 

In  general you can do
```
###for ROOT
source <path-to-root>/bin/thisroot.sh
## for pyROOT
export LD_LIBRARY_PATH=$PYTHONDIR/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$ROOTSYS/lib:$PYTHONPATH
## for geant4
source <path-to-geant>/bin/geant4.sh 
alias g4cmake="cmake -DGeant4_DIR=<path-to-geant4>lib64/Geant4-11.X.X/"
```
# Download CYGNO-MC repository

Download geometry files
```
git clone git@github.com:CYGNUS-RD/geometry.git
```

Download Geant4 code 
```
git clone git@github.com:CYGNUS-RD/CYGNO-MC.git
```

Now you have downloaded the code in `CYGNO-MC/` directory

# Setup CYGNO-MC code

Create a build directory 
```
mkdir CYGNO-MC-build
```
and compile CYGNO-MC code
```
cd CYGNO-MC-build
g4cmake ../CYGNOMC
make -j`nproc`
```

Now you have the CYGNO executable in the build directory.
You can run it in graphic mode:
```
./CYGNO
```
or specify a macro to run instructions
```
./CYGNO macro.mac
```
Some example macros are available in the `macro` directory.


A guide of CYGNO commands is in  CYGNOCommandsREADME file.


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
