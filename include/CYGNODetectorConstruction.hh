#pragma once

// STL //
#include <string>

class CYGNODetectorConstructionMessenger;
// GEANT4 //
class G4VSolid;
class G4LogicalVolume;
class G4VPhysicalVolume;
class G4UserLimits;

// CADMESH //
#include "CADMesh.hh"

// USER //
class CYGNOSensitiveDetector;

#include "G4RunManager.hh"
#include "G4ThreeVector.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4RotationMatrix.hh"
#include "G4SystemOfUnits.hh"
#include "globals.hh"
#include <vector>


class CYGNODetectorConstruction : public G4VUserDetectorConstruction
{
  public:

    CYGNODetectorConstruction();
    ~CYGNODetectorConstruction();

    G4VPhysicalVolume* Construct() override;
    void SaveMassAndDensity();
    void UpdateGeometry();
    void UpdateGeometryPath(G4String newpath);
    void ConstructSDandField() override;

    void SetExternalRockThickness(G4double rockthick) {rockThicknessOuter = rockthick;}
    void SetProductionRockThickness(G4double rockthick) {productionLayerThickness = rockthick;}
    void SetInternalRockThickness(G4double rockthick) {rockThicknessInner = rockthick;}

    void SetCYGNOLab(G4String lab) {CYGNOLab = lab;}
    G4String GetCYGNOLab() {return CYGNOLab;}

    void SetCYGNOShielding(G4String shield) {CYGNOShielding = shield;}
    G4String GetCYGNOShielding() {return CYGNOShielding;}

    void SetGeomPath(const G4String& path) {
      CYGNOGeomPath = path;
    
      // Tell Geant4 that geometry must be rebuilt
      G4RunManager::GetRunManager()->InitializeGeometry();
      G4RunManager::GetRunManager()->GeometryHasBeenModified();
    }
    G4String GetGeomPath() {return CYGNOGeomPath;}

    void SetShieldThick0(G4double thick) {thick0 = thick;}
    void SetShieldThick1(G4double thick) {thick1 = thick;}
    void SetShieldThick2(G4double thick) {thick2 = thick;}
    void SetShieldThick3(G4double thick) {thick3 = thick;}    
    void SetShield0Material(G4String shm) {Mat0 = shm;}
    void SetShield1Material(G4String shm) {Mat1 = shm;}
    void SetShield2Material(G4String shm) {Mat2 = shm;}
    void SetShield3Material(G4String shm) {Mat3 = shm;}
    void SetInsideVolumeRadius(G4double r) {InsideVolume_OR = r;}
    void SetInsideVolumeHeight(G4double h) {InsideVolume_Z = h;}

    G4double GetShieldThick0() {return thick0;}
    G4double GetShieldThick1() {return thick1;}
    G4double GetShieldThick2() {return thick2;}
    G4double GetShieldThick3() {return thick3;}    
  private:
    
    CYGNODetectorConstructionMessenger* fMessenger;

    //G4VSolid * world_solid;
    //G4LogicalVolume* world_logical;
    //G4VPhysicalVolume* world_physical;
    G4UserLimits* fStepLimit;

    G4double InsideVolume_OR;
    G4double InsideVolume_Z;
    G4double rockThicknessOuter;
    G4double rockThicknessInner;
    G4double productionLayerThickness;
   
    G4String CYGNOGeomPath; 
    G4String CYGNOLab;
    G4String CYGNOShielding;
    G4double thick0;
    G4double thick1;
    G4double thick2;
    G4double thick3;
    G4String Mat0;
    G4String Mat1;
    G4String Mat2;
    G4String Mat3;


    //CADMesh
    G4bool fUseCADGeometry = false;
    G4bool fUseV4Geometry = false;
    std::shared_ptr<CADMesh::TessellatedMesh> mesh_Cathode;
    std::shared_ptr<CADMesh::TessellatedMesh> mesh_N66;
    std::shared_ptr<CADMesh::TessellatedMesh> mesh_FCSupport;
    std::shared_ptr<CADMesh::TessellatedMesh> mesh_GemFrame;
    std::shared_ptr<CADMesh::TessellatedMesh> mesh_FieldCage;
    std::shared_ptr<CADMesh::TessellatedMesh> mesh_GEM;
    std::shared_ptr<CADMesh::TessellatedMesh> mesh_InnerShieldCu;
    std::shared_ptr<CADMesh::TessellatedMesh> mesh_OuterShieldCu;
    std::shared_ptr<CADMesh::TessellatedMesh> mesh_PEBase; 
    std::shared_ptr<CADMesh::TessellatedMesh> mesh_PMMABox;
    std::shared_ptr<CADMesh::TessellatedMesh> mesh_GasBoundary;
    
    //Building blocks: logic volumes, sizes and positions
    G4ThreeVector  tr_Tot;
    G4LogicalVolume* Rock_log;
    G4ThreeVector size_Rock;
    G4ThreeVector tr_Rock;
    G4RotationMatrix rot_Rock;
    G4RotationMatrix absrot_Rock;
    G4LogicalVolume* Laboratory_log;
    G4ThreeVector size_Laboratory;
    G4ThreeVector tr_Laboratory;
    G4RotationMatrix rot_Laboratory;
    G4LogicalVolume* Shielding_log;
    G4ThreeVector size_Shielding;
    G4ThreeVector tr_Shielding;
    G4RotationMatrix rot_Shielding;
    G4RotationMatrix absrot_Shielding;
    G4LogicalVolume* InsideVolume_log;
    G4ThreeVector size_InsideVolume;
    G4ThreeVector tr_InsideVolume;
    G4RotationMatrix rot_InsideVolume;
    G4ThreeVector size_cad;
    G4ThreeVector tr_cad;
    G4RotationMatrix rot_cad;
    G4RotationMatrix absrot_cad;
    G4ThreeVector tr_CYGNO_gas_1;
    G4ThreeVector tr_CYGNO_gas_2;
    G4RotationMatrix rot_CYGNO_gas;
    G4RotationMatrix absrot_CYGNO_gas;
    G4ThreeVector tr_cad_internal;
    
    G4RotationMatrix rot_cad_shield;
    G4ThreeVector tr_cad_shield;
    
    G4ThreeVector tr_camera_L0;
    G4ThreeVector tr_camera_L1;
    G4ThreeVector tr_camera_L2;
    G4ThreeVector tr_camera_R0;
    G4ThreeVector tr_camera_R1;
    G4ThreeVector tr_camera_R2;
    G4ThreeVector tr_camera_lens_L0;
    G4ThreeVector tr_camera_lens_L1;
    G4ThreeVector tr_camera_lens_L2;
    G4ThreeVector tr_camera_lens_R0;
    G4ThreeVector tr_camera_lens_R1;
    G4ThreeVector tr_camera_lens_R2;
    
    //Solids and meshes
    G4VSolid * cad_Cathode_solid;
    G4VSolid * cad_N66_solid;
    G4VSolid * cad_FCSupport_solid;
    G4VSolid * cad_GemFrame_solid;
    G4VSolid * cad_FieldCage_solid;
    G4VSolid * cad_GEM_solid;
    G4VSolid * cad_InnerShieldCu_solid;
    G4VSolid * cad_OuterShieldCu_solid;
    G4VSolid * cad_PEBase_solid;
    G4VSolid * cad_PMMABox_solid;
   
    
    // Logical volumes
    G4LogicalVolume* WorldVolume_log;
    G4LogicalVolume* Shield0_log; 
    G4LogicalVolume* Shield1_log; 
    G4LogicalVolume* Shield2_log; 
    G4LogicalVolume* Shield3_log; 
    G4LogicalVolume* AirBox_log;

    G4LogicalVolume * cad_Cathode_logical;
    G4LogicalVolume * cad_N66_logical;
    G4LogicalVolume * cad_FCSupport_logical;
    G4LogicalVolume * cad_GemFrame_logical;
    G4LogicalVolume * TPC_log;
    G4LogicalVolume * CYGNO_log;
    G4LogicalVolume * cad_FieldCage_logical;
    G4LogicalVolume * cad_GEM_logical;
    G4LogicalVolume * cad_InnerShieldCu_logical;
    G4LogicalVolume * cad_OuterShieldCu_logical;
    G4LogicalVolume * cad_PEBase_logical;
    G4LogicalVolume * cad_PMMABox_logical;
    G4LogicalVolume * camera_log; 
    G4LogicalVolume * camera_lens_log; 
    G4LogicalVolume * camera_shield_log;
    G4LogicalVolume * Cathode_log;
    
    // Physical volumes
    G4VPhysicalVolume* WorldVolume_phys;
    G4VPhysicalVolume* productionRockThinTube_phys;
    G4VPhysicalVolume* externalRock_phys;
    G4VPhysicalVolume* InnerAirSphere_phys;
    G4VPhysicalVolume* Shield0_phys;
    G4VPhysicalVolume* Shield1_phys;
    G4VPhysicalVolume* Shield2_phys;
    G4VPhysicalVolume* Shield3_phys;
    G4VPhysicalVolume* AirBox_phys;

    G4VPhysicalVolume * cad_Cathode_physical;
    G4VPhysicalVolume * cad_N66_physical;
    G4VPhysicalVolume * cad_FCSupport_physical;
    G4VPhysicalVolume * cad_GemFrame_physical;
    G4VPhysicalVolume * cad_FieldCage_physical;
    G4VPhysicalVolume * TPC_phys;
    G4VPhysicalVolume * CYGNO_phys;
    G4VPhysicalVolume * cad_GEM_physical;
    G4VPhysicalVolume * cad_InnerShieldCu_physical;
    G4VPhysicalVolume * cad_OuterShieldCu_physical;
    G4VPhysicalVolume * cad_PEBase_physical;
    G4VPhysicalVolume * cad_PMMABox_physical;
    G4VPhysicalVolume* camera_phys; 
    G4VPhysicalVolume* camera_lens_phys; 
    G4VPhysicalVolume* camera_shield_phys;
    G4VPhysicalVolume* Cathode_phys; 
    


};
