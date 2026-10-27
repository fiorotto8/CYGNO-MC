#include "CYGNOStepMax.hh"
#include "CYGNOStepMaxMessenger.hh"
#include "G4SystemOfUnits.hh"
#include <algorithm>
#include <cmath>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CYGNOStepMax::CYGNOStepMax(const G4String& processName)
 : G4VDiscreteProcess(processName),fMaxChargedStep(DBL_MAX),fMess(0)
{
  fMess = new CYGNOStepMaxMessenger(this);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CYGNOStepMax::~CYGNOStepMax() { delete fMess; }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4bool CYGNOStepMax::IsApplicable(const G4ParticleDefinition& particle)
{
  return (particle.GetPDGCharge() != 0.);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CYGNOStepMax::SetMaxStep(G4double step) {fMaxChargedStep = step;}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4double CYGNOStepMax::PostStepGetPhysicalInteractionLength( 
                                                 const G4Track& aTrack,
                                                       G4double,
                                                       G4ForceCondition* condition )
{
  // condition is set to "Not Forced"
  *condition = NotForced;
  G4double ProposedStep = fMaxChargedStep;
  const G4int pdg = aTrack.GetDefinition()->GetPDGEncoding();
  if (fMaxChargedStep < DBL_MAX && (pdg == 2212 || pdg > 1000000000))
    ProposedStep *= std::min(1., std::sqrt(aTrack.GetKineticEnergy()/(40.*CLHEP::keV)));
  if (pdg == 11 && aTrack.GetParentID() == 0 && aTrack.GetTrackLength() < 1.*mm)
    ProposedStep = std::min(ProposedStep, 5.*um);

  return ProposedStep;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VParticleChange* CYGNOStepMax::PostStepDoIt(const G4Track& aTrack, const G4Step&)
{
   // do nothing
   aParticleChange.Initialize(aTrack);
   return &aParticleChange;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


