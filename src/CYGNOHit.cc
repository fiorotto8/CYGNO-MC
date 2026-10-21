#include "CYGNOHit.hh"
#include "G4UnitsTable.hh"
#include "G4VVisManager.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#include "CYGNOSRIMQuenching.hh"
#include "G4Exception.hh"
#include <algorithm>

G4ThreadLocal G4Allocator<CYGNOHit>* CYGNOHitAllocator=0;

CYGNOHit::CYGNOHit() {}

CYGNOHit::~CYGNOHit() {}

CYGNOHit::CYGNOHit(const CYGNOHit& right)
  : G4VHit()
{
  parentID   = right.parentID;
  particleID   = right.particleID;
  trackID   = right.trackID;
  globalTime = right.globalTime;
  kinEne = right.kinEne;
  processIni = right.processIni;
  processFin = right.processFin;
  edep      = right.edep;
  pos       = right.pos;
  trackLen       = right.trackLen;
}

const CYGNOHit& CYGNOHit::operator=(const CYGNOHit& right)
{
  parentID   = right.parentID;
  particleID  = right.particleID;
  trackID   = right.trackID;
  globalTime = right.globalTime;
  kinEne = right.kinEne;
  processIni = right.processIni;
  processFin = right.processFin;
  edep      = right.edep;
  pos       = right.pos;
  trackLen       = right.trackLen;

  return *this;
}

G4int CYGNOHit::operator==(const CYGNOHit& right) const
{
  return (this==&right) ? 1 : 0;
}

void CYGNOHit::Draw()
{
  G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
  if(pVVisManager)
  {
    G4Circle circle(pos);
    circle.SetScreenSize(2.);
    circle.SetFillStyle(G4Circle::filled);
    G4Colour colour(1.,0.,0.);
    G4VisAttributes attribs(colour);
    circle.SetVisAttributes(attribs);
    pVVisManager->Draw(circle);
  }
}

void CYGNOHit::Print()
{
//  G4cout << "  parent id : " << parentID
//	 << "  particleID : " << particleID
//	 << "  track id : " << trackID
// 	 << "  global time : " << G4BestUnit(globalTime,"Time")
//	 << "  kinetic energy : " << G4BestUnit(kinEne,"Energy")
//	 << "  process ini : " << processIni 
//	 << "  process fin : " << processFin 
//     << "  detector number : " << detN
//	 << "  energy deposit : " << G4BestUnit(edep,"Energy")
//	 << "  position : " << G4BestUnit(pos,"Length") << G4endl;
}


namespace {
CYGNOSRIM::Response HitResponse(const CYGNOSRIM::Model& model, double energy_keV)
{
  if (!std::isfinite(energy_keV) || energy_keV < 0. || energy_keV > model.hi) {
    G4ExceptionDescription message;
    message << "SRIM QF for PDG " << model.pdg << " received " << energy_keV
            << " keV. Supported upper energy is " << model.hi << " keV per ion.";
    G4Exception("CYGNOHit::HitResponse", "SRIMQF001", FatalException, message);
    // Also fail if a custom Geant4 exception handler returns.
    throw std::out_of_range("Invalid kinetic energy for CYGNO SRIM QF");
  }
  if (energy_keV >= model.lo) return CYGNOSRIM::Evaluate(model, energy_keV);

  // Explicit extrapolation, not SRIM data: match QF and its first derivative
  // at the lower fitted boundary (25 eV) using QF(E)=QF(Eb)*(E/Eb)^beta, with beta=dln(QF)/dln(E).
  // The imported models have beta>0, so R(0)=QF(0)=0. See the dedicated note.
  const auto boundary = CYGNOSRIM::Evaluate(model, model.lo);
  const double beta = boundary.dRdE / boundary.qf - 1.;
  const double qf = boundary.qf * std::pow(energy_keV / model.lo, beta);
  return {qf, (1. + beta) * qf};
}
} // namespace

void CYGNOHit::ApplyQuenching()
{
  const auto* model = CYGNOSRIM::FindModel(particleID);
    if (model) {
      // Integrate the same TRIM derivative over the finite local deposit.
      // This removes the step-size-dependent left-endpoint quadrature bias.
      const double raw = edep;
      if (raw < 1.e-6*kinEne)
        edep *= HitResponse(*model, kinEne - 0.5*raw).dRdE;
      else {
        const double after = std::max(0., kinEne - raw);
        edep = kinEne*HitResponse(*model, kinEne).qf
             - after*HitResponse(*model, after).qf;
      }
    }
}

void CYGNOHit::ApplyQuenchingAvg()
{
    ApplyQuenching();
}
