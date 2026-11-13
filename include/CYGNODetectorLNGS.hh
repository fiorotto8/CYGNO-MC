#ifndef CYGNODetectorLNGS_h
#define CYGNODetectorLNGS_h 1

#include "globals.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "CYGNODetectorMaterial.hh"


class CYGNODetectorLNGS
{
public:

  CYGNODetectorLNGS();
  ~CYGNODetectorLNGS();
  static CYGNODetectorLNGS* GetInstance();
  void Construct();
  void ConstructRock();
  //void ConstructShielding();
  void ConstructVessel();
  void UpdateGeometry();
  void SaveMassAndDensity();
  void Refresh();

  G4LogicalVolume* GetRock() {return Rock_log;}
  G4ThreeVector GetRockSizeXYZ() {return size_Rock;}
  G4RotationMatrix GetRockAbsRotation() {return absrot_Rock;}//Rotation wrt the world reference frame
  G4LogicalVolume* GetLaboratory() {return Laboratory_log;}
  G4ThreeVector GetLaboratorySizeXYZ() {return size_Laboratory;}
  G4ThreeVector GetLaboratoryTranslation() {return tr_Laboratory;}
  G4RotationMatrix GetLaboratoryRotation() {return rot_Laboratory;}
  //G4LogicalVolume* GetShielding() {return Shielding_log;}
  //G4ThreeVector GetShieldingSizeXYZ() {return size_Shielding;}
  //G4RotationMatrix GetShieldingAbsRotation() {return absrot_Shielding;}//Rotation wrt the world reference frame
  G4LogicalVolume* GetInsideVolume() {return InsideVolume_log;}
  G4ThreeVector GetInsideVolumeSizeXYZ() {return size_InsideVolume;}
  G4ThreeVector GetInsideVolumeTranslation() {return tr_InsideVolume;}
  G4RotationMatrix GetInsideVolumeRotation() {return rot_InsideVolume;}

  G4double GetProductionRockThickness() {return productionLayerThickness;}
  G4double GetInternalRockThickness() {return rockThicknessInner;}
  void SetExternalRockThickness(G4double);
  void SetProductionRockThickness(G4double);
  void SetInternalRockThickness(G4double);
  void SetDetectorMaterial(CYGNODetectorMaterial* );


  static CYGNODetectorLNGS* fCYGNODetectorLNGS;

  G4double rockdist_z;
  G4double rockdepth_z;

private:


  G4LogicalVolume* Rock_log;
  G4ThreeVector size_Rock;
  G4RotationMatrix absrot_Rock;
  G4LogicalVolume* Laboratory_log;
  G4ThreeVector size_Laboratory;
  G4ThreeVector tr_Laboratory;
  G4RotationMatrix rot_Laboratory;
  //G4LogicalVolume* Shielding_log;
  //G4ThreeVector size_Shielding;
  //G4RotationMatrix absrot_Shielding;
  G4LogicalVolume* InsideVolume_log;
  G4ThreeVector size_InsideVolume;
  G4ThreeVector tr_InsideVolume;
  G4RotationMatrix rot_InsideVolume;

  G4double rockThicknessOuter;
  G4double rockThicknessInner;
  G4double productionLayerThickness;

  G4LogicalVolume* productionRock_log;
  G4LogicalVolume* externalRock_log; 
  G4LogicalVolume* internalRock_log; 
  G4LogicalVolume* expHallWall_log;
  G4LogicalVolume* expHall_log;
  G4VPhysicalVolume* productionRock_phys;
  G4VPhysicalVolume* internalRock_phys; 
  G4VPhysicalVolume* expeHallWall_phys ;
  G4VPhysicalVolume* expHall_phys  ; 
  
  CYGNODetectorMaterial* CYGNOMaterials;

};

#endif   /* CYGNODetectorLNGS.hh */




