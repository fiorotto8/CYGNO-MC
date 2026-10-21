#include "CYGNOEventAction.hh"
#include "CYGNOHit.hh"
#include "CYGNORunAction.hh"
#include "CYGNODetectorConstruction.hh"
#include "CYGNOUserEventInformation.hh"

#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4TrajectoryContainer.hh"
#include "G4Trajectory.hh"
#include "G4ios.hh"
#include "G4SDManager.hh"
#include "G4AnalysisManager.hh"
#include "CYGNOScreeningResponse.hh"

#include <fstream>
#include <unistd.h>
size_t GetMemoryUsageMB_2()
{
    long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024;
    std::ifstream statm("/proc/self/statm");
    long size, resident;
    statm >> size >> resident;  // read both values
    return resident * page_size_kb / 1024;  // MB
}

CYGNOEventAction::CYGNOEventAction(CYGNODetectorConstruction* myDC)
  : G4UserEventAction(), fDetector(myDC)
{
  // hits collections
  CYGNOID = -1;
}

CYGNOEventAction::~CYGNOEventAction()
{}

void CYGNOEventAction::BeginOfEventAction(const G4Event* evt)
{
  //New event, add the user information object
  G4EventManager::
    GetEventManager()->SetUserInformation(new CYGNOUserEventInformation);

  if ( CYGNOID == -1) {
    G4SDManager * SDman = G4SDManager::GetSDMpointer();
    CYGNOID = SDman->GetCollectionID("CYGNOCollection");
  } 

  G4int evtID = evt->GetEventID();

  if (evtID % 1000 == 0) {
	       G4cout << "Thread "
               << G4Threading::G4GetThreadId()
               << " processing event "
               << evtID
               << G4endl;
    }
}

void CYGNOEventAction::EndOfEventAction(const G4Event* evt)
{
  G4int event_id = evt->GetEventID();

  // check that hits collection has been defined
  if(CYGNOID<0) return;

  G4AnalysisManager* man = G4AnalysisManager::Instance();
  if (!man) {
      G4cerr << "Error: Analysis manager not available!" << G4endl;
      return;
  }

  // address hits collections
  CYGNOHitsCollection* CYGNOHC = NULL;
  G4HCofThisEvent* HCE = evt->GetHCofThisEvent();
  if(HCE) {
    CYGNOHC = (CYGNOHitsCollection*)(HCE->GetHC(CYGNOID));
  }

  // event summary
  G4double  energyDep=0.;
  G4double  energyDep_QF=0.;
  G4double  energyDep_QF_geant=0.;
  G4double  energyDep_NR=0.;
  G4double  energyDep_NRQF=0.;
  G4double  energyDep_NRQF_geant=0.;

  v_pdgID_hits.clear();
  v_tracklen_hits.clear();
  v_px_particle.clear();
  v_py_particle.clear();
  v_pz_particle.clear();
  v_energyDep_hits.clear();
  v_energyDep_hits_QF.clear();
  v_energyDep_hits_QF_geant.clear();
  v_energyDep_hits_NR.clear();
  v_energyDep_hits_NRQF.clear();
  v_energyDep_hits_NRQF_geant.clear();
  v_x_hits.clear();
  v_y_hits.clear();
  v_z_hits.clear();
  v_parentID_hits.clear();
  v_trackID_hits.clear();
  v_kinEne_hits.clear();
  v_time_hits.clear();


  if(CYGNOHC) {
    CYGNO_hits = CYGNOHC->entries();
        
    if (CYGNO_hits > 0) {
      
      man->FillNtupleIColumn(2,0,event_id);
      man->FillNtupleIColumn(2,1,CYGNO_hits);
      man->FillNtupleDColumn(2,2,(*CYGNOHC)[0]->GetKineticEne());
      man->FillNtupleIColumn(2,3,(*CYGNOHC)[0]->GetParticleID());
      //G4cout << "firstParticleE =" << (*CYGNOHC)[0]->GetKineticEne() << "keV" << G4endl;
      
      for (G4int i=0; i<CYGNO_hits; i++) {
        
        v_pdgID_hits.push_back((*CYGNOHC)[i]->GetParticleID());
        v_tracklen_hits.push_back((*CYGNOHC)[i]->GetLength());
        v_px_particle.push_back((*CYGNOHC)[i]->GetMom().x());
        v_py_particle.push_back((*CYGNOHC)[i]->GetMom().y());
        v_pz_particle.push_back((*CYGNOHC)[i]->GetMom().z());
        v_x_hits.push_back((*CYGNOHC)[i]->GetPos().x());
        v_y_hits.push_back((*CYGNOHC)[i]->GetPos().y());
        v_z_hits.push_back((*CYGNOHC)[i]->GetPos().z());
        v_parentID_hits.push_back((*CYGNOHC)[i]->GetParentID());
        v_trackID_hits.push_back((*CYGNOHC)[i]->GetTrackID());
        v_kinEne_hits.push_back((*CYGNOHC)[i]->GetKineticEne());
        v_time_hits.push_back((*CYGNOHC)[i]->GetGlobalTime());
        
	G4double rawEdep = (*CYGNOHC)[i]->GetEdep();
        v_energyDep_hits.push_back(rawEdep);
	
	G4int pdg = (int)(*CYGNOHC)[i]->GetParticleID(); 
	if(pdg == 2212 || pdg > 1000000000) {
	    // Proton or ion => fill NR (raw) and NRQF (quenched)
	    // store raw deposit in energyDep_hits_NR
	    v_energyDep_hits_NR.push_back(rawEdep);
	    v_energyDep_hits_NRQF_geant.push_back((*CYGNOHC)[i]->GetIonizingEnergy());   //fill with ionising energy calculated by geant4
	    v_energyDep_hits_QF_geant.push_back((*CYGNOHC)[i]->GetIonizingEnergy());   //fill with ionising energy calculated by geant4
	    //G4cout << "kin ene: " << (*CYGNOHC)[i]->GetKineticEne() << G4endl;
	    // apply QF for the same hit
	    // Kinetic energy is in keV; the SRIM/TRIM model starts at 25 eV.
	    if ((*CYGNOHC)[i]->GetKineticEne() <= 0.025){
	    	(*CYGNOHC)[i]->ApplyQuenchingAvg(); 
	    }
	    else{
	    	//G4cout << "######## Applying dQFdE ############" << G4endl; 
	    	(*CYGNOHC)[i]->ApplyQuenching(); 
	    }
	    //G4cout << "raw ene " << rawEdep << "\t corrected with QF\t" << (*CYGNOHC)[i]->GetEdep() <<"\tQF\t"<< (*CYGNOHC)[i]->GetEdep()/rawEdep  << G4endl;
	    // store the now‐modified deposit
	    v_energyDep_hits_NRQF.push_back((*CYGNOHC)[i]->GetEdep());
	    v_energyDep_hits_QF.push_back((*CYGNOHC)[i]->GetEdep());   //fill with corrected energy

	    // optional: restore the original edep if you do NOT want
	    // to leave it permanently changed inside the hit object
	    // (*CYGNOHC)[i]->SetEdep(rawEdep);
	} else {
	    // Not an ion => fill energyDep_hits, zero in the other two
	    v_energyDep_hits_NR.push_back(0.0);
	    v_energyDep_hits_NRQF.push_back(0.0);
	    v_energyDep_hits_QF.push_back(rawEdep);   //fill with raw energy
	    v_energyDep_hits_NRQF_geant.push_back(0.0);
	    v_energyDep_hits_QF_geant.push_back(rawEdep);
	}

      // sum total energy deposited in hits (no QF)
	    energyDep += rawEdep;
	    // Protons and ions are nuclear recoils (ion PDG scheme: 100ZZZAAAI).
	    if (pdg == 2212 || pdg > 1000000000)
	    {
	       // Apply QF derivative step by step for nuclear hits
	       energyDep_NR += rawEdep;

	       energyDep_NRQF += (*CYGNOHC)[i]->GetEdep();        
	       energyDep_NRQF_geant += (*CYGNOHC)[i]->GetIonizingEnergy();        
	       energyDep_QF += (*CYGNOHC)[i]->GetEdep();
	       energyDep_QF_geant +=  (*CYGNOHC)[i]->GetIonizingEnergy();
	    }
	    else{
	       energyDep_QF += rawEdep;
	       energyDep_QF_geant +=  rawEdep;
	    }
      }
      // Uniform event response calibration preserves normalized charge geometry.
      const G4double gain = CYGNOScreeningResponseGain(evt, energyDep_QF);
      energyDep_QF *= gain;
      energyDep_NRQF *= gain;
      for (auto& e : v_energyDep_hits_QF) e *= gain;
      for (auto& e : v_energyDep_hits_NRQF) e *= gain;
      man->FillNtupleDColumn(2,4,energyDep);
      man->FillNtupleDColumn(2,5,energyDep_QF);
      man->FillNtupleDColumn(2,6,energyDep_QF_geant);
      man->FillNtupleDColumn(2,7,energyDep_NR);
      man->FillNtupleDColumn(2,8,energyDep_NRQF);
      man->FillNtupleDColumn(2,9,energyDep_NRQF_geant);
      
      man->AddNtupleRow(2);
    }
  }

// FIXME - memory control
size_t mem = GetMemoryUsageMB_2();

    if (mem > 3800) {  // 4 GB limit
        G4cout << "Memory limit WARNING: " << mem << " MB!! (Limit 4GB)" << G4endl;
        //G4RunManager::GetRunManager()->AbortRun(true);
    }
}
