#include "CYGNOPrimaryGeneratorAction.hh"
#include "CYGNODetectorConstruction.hh"
#include "Randomize.hh"
#include "globals.hh"
#include <math.h>
#include "G4GeneralParticleSource.hh"
#include "G4ThreeVector.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4TransportationManager.hh"
#include "G4RunManager.hh"
#include "G4Gamma.hh"
#include "G4Event.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"
#include "G4Exception.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4AnalysisManager.hh"

#define PI 3.14159265
using namespace std;

CYGNOPrimaryGeneratorAction::CYGNOPrimaryGeneratorAction(CYGNODetectorConstruction* myDC):myDetector(myDC)
{
  n_particle = 1;
  particleGun = new G4GeneralParticleSource();

  // default particle
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  G4ParticleDefinition* particle = particleTable->FindParticle("e-");
  particleGun->SetParticleDefinition(particle);
  particle_energy = 1.0*MeV;
  }

CYGNOPrimaryGeneratorAction::~CYGNOPrimaryGeneratorAction()
{
  delete particleGun;
}

void CYGNOPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{ 
  // Generate primary vertex
  particleGun->GeneratePrimaryVertex(anEvent);

  if (fFillNtuple) {
    // Keep one source row per event: first primary of the first vertex.
    // GPS getters can refer to a source selected by another worker.
    const auto* vertex = anEvent->GetPrimaryVertex(0);
    const auto* primary = vertex ? vertex->GetPrimary(0) : nullptr;
    if (!primary) {
      G4Exception("CYGNOPrimaryGeneratorAction::GeneratePrimaries", "CYGNO_Primary",
                  FatalException, "Cannot record source information: event has no primary particle.");
      return;
    }
    energy_pri = primary->GetKineticEnergy();
    const G4ThreeVector particlePosition = vertex->GetPosition();
    const G4ThreeVector direction = primary->GetMomentumDirection();

    //Fill ntuple #1
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    man->FillNtupleDColumn(1,0,energy_pri);
    man->FillNtupleDColumn(1,1,particlePosition[0]);
    man->FillNtupleDColumn(1,2,particlePosition[1]);
    man->FillNtupleDColumn(1,3,particlePosition[2]);
    man->FillNtupleDColumn(1,4,direction.x());
    man->FillNtupleDColumn(1,5,direction.y());
    man->FillNtupleDColumn(1,6,direction.z());
    man->FillNtupleIColumn(1,7,anEvent->GetEventID());
    man->AddNtupleRow(1);
  }
}

