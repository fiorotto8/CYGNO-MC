#######################################################################
## THIS PART OF CODE MAY NEED SOME CHANGES TO MATCH THE USER'S NEEDS ##
#######################################################################

import getpass

#Job properties
MacrosList = ['CYGNOtest.mac'] #This is the list of configuration macros to be submitted. They must have extension .mac and must be located inside the CODEDIR+'macro/' directory
NEvts = ['20000'] #This list has to be the same lenght of MacrosList and specifies the number of events to be simulated for each macro
NEvtsPerJob = '10000' #The maximum number of events per job. If NEvtsPerJob<NEvts, the simulation will be splitted in  NEvts/NEvtsPerJob jobs
MaxNJobs=500 #This depends on the machine cluster used. Some of these system have a maximum limit on the number of jobs that can be submitted
TAG='test1' #this is the reference name of the simulation that you have submitted. The output of the simulation will be contained in a folder with this name

username = getpass.getuser()

#Directory paths (the queue and the worker Geant4/ROOT environment are set in submit_jobs_lngs.py)
CODEDIR='/users/'+username[0]+'/'+username+'/CYGNO/CYGNO-MC/' # this folder contains the code of the simulation and the background information (subdir backgrounds/)
OUTDIR='/nfs/cygno/CYGNO-MC-data/' # this is the folder where results of the simulations are stored: the folder will contain PBSOUTDIR and pbs_logs subdirectories containing the outputs of simulation and the logs, respectively.
PBSOUTDIR='pbs_outputs/' #subfolder of OUTDIR where the ROOT output files of each TAG are saved
TMPDIR='/nfs/scratch/'+username #this is a temporary directory where the simulation is compiled per each job. It is then cleaned up at the end of each job
BUILDDIR='/users/'+username[0]+'/'+username+'/CYGNO/CYGNO-MC-build/'


MATERIALS={'Pb':'G4_Pb', 'PE':'G4_POLYETHYLENE', 'Air':'G4_AIR', 'Rock':'LNGSRock', 'Cu':'G4_Cu', 'Steel':'G4_STAINLESS-STEEL', 'Teflon':'G4_TEFLON','Water':'Water','Acrylic':'Acrylic','CYGNO_gas':'CYGNO_gas'} #List of known materials. Used to give name to the outuput file and (obsolete) to identify shielding layers
