#include "CYGNORunAction.hh"
#include "CYGNOEventAction.hh"
#include "CYGNORunActionMessenger.hh"
#include "G4AnalysisManager.hh"
#include "CYGNODetectorConstruction.hh"
#include "G4Run.hh"


CYGNORunAction::CYGNORunAction(CYGNOEventAction* myEA, CYGNODetectorConstruction* myDC)
  : G4UserRunAction(), fDetector(myDC), fEventAction(myEA)
{
  fMessenger = new CYGNORunActionMessenger(this);
}

CYGNORunAction::~CYGNORunAction()
{
  delete fMessenger;
}

void CYGNORunAction::BeginOfRunAction(const G4Run* aRun)
{
		//Master mode or sequential
  if (IsMaster())    
    G4cout << "### Run " << aRun->GetRunID() << " starts (master)." << G4endl;
  else
    G4cout << "### Run " << aRun->GetRunID() << " starts (worker)." << G4endl;
  
  // Book histograms and ntuples
  Book();

}

void CYGNORunAction::EndOfRunAction(const G4Run*)
{ 
  G4AnalysisManager* man = G4AnalysisManager::Instance();
  man->Write();
  man->CloseFile();
}

void CYGNORunAction::Book() 
{
  // Get/create analysis manager
  G4AnalysisManager* man = G4AnalysisManager::Instance();
  man->SetDefaultFileType("root");
  
  man->SetNtupleMerging(true);

  man->SetFirstNtupleId(1);


  // ---- primary ntuple ------
  // id==1
  man->CreateNtuple("tree1", "Particle Source Info");
  man->CreateNtupleDColumn("Energy");
  man->CreateNtupleDColumn("xpos_vertex");
  man->CreateNtupleDColumn("ypos_vertex");
  man->CreateNtupleDColumn("zpos_vertex");
  // Sampled source direction (dimensionless); event ID links to nTuple.
  man->CreateNtupleDColumn("dir_x");
  man->CreateNtupleDColumn("dir_y");
  man->CreateNtupleDColumn("dir_z");
  man->CreateNtupleIColumn("eventnumber");
  man->FinishNtuple();

  // ---- secondary ntuple ------   
  //id==2
  man->CreateNtuple("nTuple", "Hits Info");
  man->CreateNtupleIColumn("eventnumber");
  man->CreateNtupleIColumn("numhits");
  man->CreateNtupleDColumn("ekin_particle");
  man->CreateNtupleIColumn("particle_type");
  man->CreateNtupleDColumn("energyDep");
  man->CreateNtupleDColumn("energyDep_QF");
  man->CreateNtupleDColumn("energyDep_QF_geant");
  man->CreateNtupleDColumn("energyDep_NR");
  man->CreateNtupleDColumn("energyDep_NRQF");
  man->CreateNtupleDColumn("energyDep_NRQF_geant");
  man->CreateNtupleIColumn("pdgID_hits", fEventAction ? fEventAction -> Get_pdgID_hits() : fEmptyIntVector);
  man->CreateNtupleDColumn("tracklen_hits", fEventAction ? fEventAction -> Get_tracklen_hits() : fEmptyDoubleVector);
  man->CreateNtupleDColumn("px_particle", fEventAction ? fEventAction -> Get_px_particle() : fEmptyDoubleVector);
  man->CreateNtupleDColumn("py_particle", fEventAction ? fEventAction -> Get_py_particle() : fEmptyDoubleVector);
  man->CreateNtupleDColumn("pz_particle", fEventAction ? fEventAction -> Get_pz_particle() : fEmptyDoubleVector);
  man->CreateNtupleDColumn("energyDep_hits", fEventAction ? fEventAction -> Get_energyDep_hits() : fEmptyDoubleVector);
  man->CreateNtupleDColumn("energyDep_hits_QF", fEventAction ? fEventAction -> Get_energyDep_QF_hits() : fEmptyDoubleVector);
  man->CreateNtupleDColumn("energyDep_hits_QF_geant", fEventAction ? fEventAction -> Get_energyDep_QF_geant_hits() : fEmptyDoubleVector);
  man->CreateNtupleDColumn("energyDep_hits_NR", fEventAction ? fEventAction -> Get_energyDep_NR_hits() : fEmptyDoubleVector);
  man->CreateNtupleDColumn("energyDep_hits_NRQF", fEventAction ? fEventAction -> Get_energyDep_NR_QF_hits() : fEmptyDoubleVector);
  man->CreateNtupleDColumn("energyDep_hits_NRQF_geant", fEventAction ? fEventAction -> Get_energyDep_NR_QF_geant_hits() : fEmptyDoubleVector);
  man->CreateNtupleDColumn("x_hits", fEventAction ? fEventAction -> Get_x_hits() : fEmptyDoubleVector);
  man->CreateNtupleDColumn("y_hits", fEventAction ? fEventAction -> Get_y_hits() : fEmptyDoubleVector);
  man->CreateNtupleDColumn("z_hits", fEventAction ? fEventAction -> Get_z_hits() : fEmptyDoubleVector);
  man->CreateNtupleIColumn("parentID_hits", fEventAction ? fEventAction -> Get_parentID_hits() : fEmptyIntVector);
  man->CreateNtupleIColumn("trackID_hits", fEventAction ? fEventAction -> Get_trackID_hits() : fEmptyIntVector);
  man->CreateNtupleDColumn("kinEne_hits", fEventAction ? fEventAction -> Get_kinEne_hits() : fEmptyDoubleVector);
  man->CreateNtupleDColumn("time_hits", fEventAction ? fEventAction -> Get_time_hits() : fEmptyDoubleVector);
man->FinishNtuple();

  // Open an output file
  man->OpenFile(FileName);

  return;
}
