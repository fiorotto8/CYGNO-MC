#include <iostream>
#include <string.h>
#include <fstream> 

// GEANT4 //
#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4Transform3D.hh"
//#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Polyhedra.hh"
#include "G4Sphere.hh"
#include "G4EllipticalTube.hh"
#include "G4Ellipsoid.hh"
#include "G4IntersectionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4DisplacedSolid.hh"
#include "G4Exception.hh"
#include <filesystem>
#include <cstdlib>

#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4VisAttributes.hh"
#include "G4SDManager.hh"

#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"
#include "G4GeometryManager.hh"
#include "G4RunManager.hh"

#include "G4UserLimits.hh"

#include "G4RotationMatrix.hh"

// USER //
#include "CYGNODetectorConstruction.hh"
#include "CYGNODetectorConstructionMessenger.hh"
#include "CYGNODetectorLNGS.hh"
#include "CYGNODetectorMaterial.hh"
#include "CYGNODetectorProperty.hh"
#include "CYGNOSensitiveDetector.hh"
#include "CYGNOVolumes.hh"
#include "CADMesh.hh"
//#include "CYGNOBiasMultiParticleChangeCrossSection.hh"

#include <fstream>
#include <unistd.h>
size_t GetMemoryUsageMB()
{
    long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024;
    std::ifstream statm("/proc/self/statm");
    long size, resident;
    statm >> size >> resident;  // read both values
    return resident * page_size_kb / 1024;  // MB
}

CYGNODetectorConstruction::CYGNODetectorConstruction() :
   CYGNOGeomPath(std::getenv("CYGNO_GEOMETRY_PATH")
       ? std::getenv("CYGNO_GEOMETRY_PATH") : "../geometry/cygno04_v4_ASCII/"),
   rockThicknessOuter(-999*m),
   rockThicknessInner(-999*m),
   //rockThicknessInner(4.*m),
   //productionLayerThickness(2.5*m),
   productionLayerThickness(-999*m),
   CYGNOLab("NoCave"),
   //CYGNOLab("LNGS"),
   //CYGNOLab("MuonLNGS"),
   CYGNOShielding("FullShield"),
  //  CYGNOShielding("NoShield"),
   thick0(0.01*m), thick1(0.01*m), thick2(0.01*m), thick3(0.01*m), 
   Mat0("Air"), Mat1("Air"), Mat2("Air"), Mat3("Air")
{
	
     fMessenger = new CYGNODetectorConstructionMessenger(this);
}

CYGNODetectorConstruction::~CYGNODetectorConstruction()
{
	delete fMessenger;

}

G4VPhysicalVolume* CYGNODetectorConstruction::Construct()
{
   
    //UpdateGeometryPath(CYGNOGeomPath); 
    //G4cout << "Updated geometry path..." << G4endl;
	
    G4NistManager * nist_manager = G4NistManager::Instance();
   
    //-----------------------------
    // construction of materials
    //-----------------------------
    G4cout << "Constructing materials..." << G4endl;

    //G4Material * air = nist_manager->FindOrBuildMaterial("G4_AIR");
    //G4Material * water = nist_manager->FindOrBuildMaterial("G4_WATER");
    CYGNODetectorMaterial* CYGNOMaterials = CYGNODetectorMaterial::GetInstance();
    G4cout << "... done" << G4endl;

    ////-----------------------------
    //// construction of general properties
    ////-----------------------------
    //G4cout << "Constructing general properties...";
    //CYGNODetectorProperty* CYGNOProperties = CYGNODetectorProperty::GetInstance();
    //G4cout << "... done" << G4endl;




    //**********************************************************************
    //   DEFINITION OF THE GEOMETRY
    //**********************************************************************
      
    //INITIALIZING TRANSLATION VECTORS TO 0:
    Rock_log = 0;
    size_Rock = G4ThreeVector();
    tr_Rock = G4ThreeVector();
    rot_Rock = G4RotationMatrix();
    absrot_Rock = G4RotationMatrix();
    Laboratory_log = 0;
    size_Laboratory = G4ThreeVector();
    tr_Laboratory = G4ThreeVector();
    rot_Laboratory = G4RotationMatrix();
    Shielding_log = 0;
    size_Shielding = G4ThreeVector();
    tr_Shielding = G4ThreeVector();
    rot_Shielding = G4RotationMatrix();
    absrot_Shielding = G4RotationMatrix();
    InsideVolume_log = 0;
    size_InsideVolume = G4ThreeVector();
    tr_InsideVolume = G4ThreeVector();
    rot_InsideVolume = G4RotationMatrix();

    //Name of the volumes
    G4String name_solid="";
    G4String name_log="";
    G4String name_phys="";

    G4ThreeVector tr;
    G4RotationMatrix rot;

    //**********************************************************************
    // WORLD ***************************************
    //**********************************************************************
    G4double world_x = 100.0*m;
    G4double world_y = 100.0*m;
    G4double world_z = 400.0*m;
      
    name_phys="WorldVolume";
    name_log=name_phys+"_log";
    name_solid=name_phys+"_solid";
    G4Box* WorldVolume_box = new G4Box(name_solid,0.5*world_x,0.5*world_y,0.5*world_z);
    WorldVolume_log = new G4LogicalVolume(WorldVolume_box,CYGNOMaterials->Material("Vacuum"),name_log,0,0,0);
    //THE WORLD CANNOT BE TRANSLATED, therefore the first volume inside it (Rock_log) is the volume that must be translated in order to adjust the origin. Rock_log cannot be as large as the world to avoid that this volume is moved out of the world
    WorldVolume_phys = new G4PVPlacement(0,G4ThreeVector(),WorldVolume_log,name_phys,0,false,0,true);//The world volume cannot be translated
          
    //These variables are used to set the thin tube for depth studies in the externalRock_log
    G4double rockdist_z;
    G4double rockdepth_z;
        
    //**********************************************************************
    // LABORATORY ***************************************
    //**********************************************************************
    G4cout << "Constructing laboratory..." << G4endl;
    G4bool isThinTubeCompatible=false;
    // ---------------------------------- LNGS
    if (CYGNOLab == "LNGS"){
    
        /////////////////////////////////////////////////////////
	// In this configuration the translations are wrong, WIP
	// /////////////////////////////////////////////////////
	G4cout << "========== WARNING =============" << G4endl;
	G4cout << "Translation vectors in this configuration ar wrong, WIP, use 'NoCave'" << G4endl;
	G4cout << "================================" << G4endl;
	CYGNODetectorLNGS* LNGS = CYGNODetectorLNGS::GetInstance();
        if (rockThicknessOuter != -999*m)
              LNGS->SetExternalRockThickness(rockThicknessOuter);
        if (productionLayerThickness != -999*m)
              LNGS->SetProductionRockThickness(productionLayerThickness);
        if (rockThicknessInner != -999*m)
              LNGS->SetInternalRockThickness(rockThicknessInner);	  
        
	LNGS->SetDetectorMaterial(CYGNOMaterials);
	LNGS->ConstructRock();
        Rock_log=LNGS->GetRock();
        size_Rock=LNGS->GetRockSizeXYZ();
        absrot_Rock=LNGS->GetRockAbsRotation();
        Laboratory_log=LNGS->GetLaboratory();
        size_Laboratory=LNGS->GetLaboratorySizeXYZ();
        tr_Laboratory=LNGS->GetLaboratoryTranslation();
        rot_Laboratory=LNGS->GetLaboratoryRotation();
        
        ////for thin tube
        //if(LNGS->GetProductionRockThickness()==0.*cm && LNGS->GetInternalRockThickness()==0.*cm)
        //  isThinTubeCompatible=true;
        //rockdist_z=LNGS->rockdist_z;
        //rockdepth_z=LNGS->rockdepth_z;
    }
    // ---------------------------------- NoCave
    else if (CYGNOLab == "NoCave") 
	{
	  //**********************************************************************
	  // Double Air sphere surrounding the whole detector and shielding
	  //**********************************************************************        
	  G4double airInnerRadius = 25.0*m;
	  G4double airThickness = 20.0*m;
	  if (productionLayerThickness != -999*m)
		airThickness = productionLayerThickness;
	  if (rockThicknessInner != -999*m)
		airInnerRadius=rockThicknessInner;

	  //Air permeates the are around the detector
	  name_phys="OuterAirSphere";
	  name_log=name_phys+"_log";
	  name_solid=name_phys+"_solid";
	  G4Sphere* OuterAirSphere = new G4Sphere(name_solid, 0., airInnerRadius+airThickness, 0*degree, 360*degree, 0*degree,180*degree);
	  G4LogicalVolume* OuterAirSphere_log = new G4LogicalVolume(OuterAirSphere,CYGNOMaterials->Material("Air"),name_log);
	  Rock_log=OuterAirSphere_log;
	  size_Rock=G4ThreeVector(airInnerRadius+airThickness,airInnerRadius+airThickness,airInnerRadius+airThickness);
	  absrot_Rock = G4RotationMatrix();
		
	  name_phys="InnerAirSphere";
	  name_log=name_phys+"_log";
	  name_solid=name_phys+"_solid";
	  G4Sphere* InnerAirSphere = new G4Sphere(name_solid,  0., airInnerRadius,  0*degree, 360*degree, 0*degree,180*degree);
	  G4LogicalVolume* InnerAirSphere_log = new G4LogicalVolume(InnerAirSphere, CYGNOMaterials->Material("Air"), name_log);
	  Laboratory_log=InnerAirSphere_log;
	  size_Laboratory=G4ThreeVector(airInnerRadius,airInnerRadius,airInnerRadius);
	  tr = G4ThreeVector(0.,0.,0.);//translation in mother frame
	  tr_Laboratory+=(rot_Laboratory*tr);
	  rot = G4RotationMatrix();// rotation of daughter volume
	  rot_Laboratory*=rot; //equivalent to rot_Laboratory=rot_Laboratory*rot
	  InnerAirSphere_phys = new G4PVPlacement(G4Transform3D(rot,tr),InnerAirSphere_log,name_phys,OuterAirSphere_log,false,0,true);        
    }
    else if (CYGNOLab == "MuonLNGS")
      {
	  //**********************************************************************
	  // Air parallelepiped inside a parallelepiped made of rock
	  //**********************************************************************        
	  G4double airInnerRadius = 3.0*m; //3.0*m for Muon simulation
	  G4double airThickness = 5*cm; //5.0*m for Muon simulation
	  if (productionLayerThickness != -999*m)
		airThickness = productionLayerThickness;
	  if (rockThicknessInner != -999*m)
		airInnerRadius=rockThicknessInner;

	  //Air permeates the are around the detector
	  name_phys="OuterAirSphere";
	  name_log=name_phys+"_log";
	  name_solid=name_phys+"_solid";
	  //G4Sphere* OuterAirSphere = new G4Sphere(name_solid, 0., airInnerRadius+airThickness, 0*degree, 360*degree, 0*degree,180*degree); 
	  
	  //LNGS rock:
	  G4Box* OuterAirSphere = new G4Box(name_solid,(airInnerRadius+airThickness)/2.,(airInnerRadius+airThickness)/2.,(airInnerRadius+airThickness)/2.);
	  G4LogicalVolume* OuterAirSphere_log = new G4LogicalVolume(OuterAirSphere,CYGNOMaterials->Material("LNGSRock"),name_log); //LNGS rock
	  Rock_log=OuterAirSphere_log;
	  size_Rock=G4ThreeVector(airInnerRadius+airThickness,airInnerRadius+airThickness,airInnerRadius+airThickness); //seems it's not used
	  absrot_Rock = G4RotationMatrix();
		
	  name_phys="InnerAirSphere";
	  name_log=name_phys+"_log";
	  name_solid=name_phys+"_solid";
	  G4Box* InnerAirSphere = new G4Box(name_solid,airInnerRadius/2.,airInnerRadius/2.,airInnerRadius/2.);
	  G4LogicalVolume* InnerAirSphere_log = new G4LogicalVolume(InnerAirSphere, CYGNOMaterials->Material("Air"), name_log);
	  Laboratory_log=InnerAirSphere_log;
	  size_Laboratory=G4ThreeVector(airInnerRadius,airInnerRadius,airInnerRadius);
	  tr = G4ThreeVector(0.,0.,0.);//translation in mother frame
	  tr_Laboratory+=(rot_Laboratory*tr);
	  rot = G4RotationMatrix();// rotation of daughter volume
	  rot_Laboratory*=rot; //equivalent to rot_Laboratory=rot_Laboratory*rot
	  InnerAirSphere_phys = new G4PVPlacement(G4Transform3D(rot,tr),InnerAirSphere_log,name_phys,OuterAirSphere_log,false,0,true);

    }
    else
    {
      G4cout << "ERROR: Something went wrong with the definition of the variable CYGNOLab" << G4endl;
      //throw std::exception();
      exit(1);		
    }


    G4cout << "Laboratory done." << G4endl;
 
    //**********************************************************************
    // SHIELDING **********************************************************
    
          
    if (CYGNOShielding == "FullShield") 
    {
        G4cout<<"Shielding Construction Started"<<G4endl;
        
        if (thick3==-999*m || thick2==-999*m || thick1==-999*m || thick0==-999*m || Mat0=="" || Mat1=="" || Mat2=="" || Mat3=="") 
        {
            G4cout << "ERROR: Please specify the thickennses and the materials of the shielding layers if you want to use the shielding design: FullShield" << G4endl;
            //throw std::exception();
            exit(1);
        }

        // ----------------------------------- Inner room dimensions
        G4double AirBox_x;
        G4double AirBox_y;
        G4double AirBox_z;
        G4Box* AirBox;
        AirBox_x = 7.*m;
        AirBox_y = 7.*m;
        AirBox_z = 7.*m;
        tr_InsideVolume = G4ThreeVector(0.,0.,0.);
        rot_InsideVolume = G4RotationMatrix();		
        size_InsideVolume = G4ThreeVector(AirBox_x/2.,
              								AirBox_y/2.,
              								AirBox_z/2.);		
        size_Shielding = G4ThreeVector(AirBox_x/2. + thick3 + thick2 + thick1 + thick0,
              							 AirBox_y/2. + thick3 + thick2 + thick1 + thick0,
              							 AirBox_z/2. + thick3 + thick2 + thick1 + thick0);
        absrot_Shielding = G4RotationMatrix();

        // ----------------------------------- Shield 0
        G4double Shield0_x = AirBox_x + 2.*thick3 + 2.*thick2 + 2.*thick1 + 2.*thick0 ;
        G4double Shield0_y = AirBox_y + 2.*thick3 + 2.*thick2 + 2.*thick1 + 2.*thick0 ;
        G4double Shield0_z = AirBox_z + 2.*thick3 + 2.*thick2 + 2.*thick1 + 2.*thick0 ;        
        G4Material* Shield0Mat = CYGNOMaterials->Material(Mat0);
        name_phys="Shield0";
        name_log=name_phys+"_log";
        name_solid=name_phys+"_solid";
        G4Box* Shield0 = new G4Box(name_solid,0.5*Shield0_x,0.5*Shield0_y,0.5*Shield0_z);
        Shield0_log = new G4LogicalVolume(Shield0,Shield0Mat,name_log);
        Shielding_log = Shield0_log;
        Shield0_log->SetVisAttributes(CYGNOMaterials->VisAttributes(Mat0));

        // ----------------------------------- Shield 1
        G4double Shield1_x = AirBox_x + 2.*thick3 + 2.*thick2 + 2.*thick1 ;
        G4double Shield1_y = AirBox_y + 2.*thick3 + 2.*thick2 + 2.*thick1 ;
        G4double Shield1_z = AirBox_z + 2.*thick3 + 2.*thick2 + 2.*thick1 ;
        G4Material* Shield1Mat = CYGNOMaterials->Material(Mat1);
        name_phys="Shield1";
        name_log=name_phys+"_log";
        name_solid=name_phys+"_solid";
        G4Box* Shield1 = new G4Box(name_solid,0.5*Shield1_x,0.5*Shield1_y,0.5*Shield1_z);
        Shield1_log = new G4LogicalVolume(Shield1,Shield1Mat,name_log);
        Shield1_log->SetVisAttributes(CYGNOMaterials->VisAttributes(Mat1));
        
        // ----------------------------------- Shield 2        
        G4double Shield2_x = AirBox_x + 2.*thick3 + 2.*thick2 ;
        G4double Shield2_y = AirBox_y + 2.*thick3 + 2.*thick2 ;
        G4double Shield2_z = AirBox_z + 2.*thick3 + 2.*thick2 ;
        G4Material* Shield2Mat = CYGNOMaterials->Material(Mat2);
        name_phys="Shield2";
        name_log=name_phys+"_log";
        name_solid=name_phys+"_solid";
        G4Box* Shield2 = new G4Box(name_solid,0.5*Shield2_x,0.5*Shield2_y,0.5*Shield2_z);
        Shield2_log = new G4LogicalVolume(Shield2,Shield2Mat,name_log);
        Shield2_log->SetVisAttributes(CYGNOMaterials->VisAttributes(Mat2));
        
        // ----------------------------------- Shield 3        
        G4double Shield3_x = AirBox_x + 2.*thick3 ;
        G4double Shield3_y = AirBox_y + 2.*thick3 ;
        G4double Shield3_z = AirBox_z + 2.*thick3 ;
        G4Material* Shield3Mat = CYGNOMaterials->Material(Mat3);
        name_phys="Shield3";
        name_log=name_phys+"_log";
        name_solid=name_phys+"_solid";
        G4Box* Shield3 = new G4Box(name_solid,0.5*Shield3_x,0.5*Shield3_y,0.5*Shield3_z);
        Shield3_log = new G4LogicalVolume(Shield3,Shield3Mat,name_log);
        Shield3_log->SetVisAttributes(CYGNOMaterials->VisAttributes(Mat3));
        
        // ----------------------------------- Airbox
        name_phys="AirBox";
        name_log=name_phys+"_log";
        name_solid=name_phys+"_solid";
        AirBox = new G4Box(name_solid,0.5*AirBox_x,0.5*AirBox_y,0.5*AirBox_z);
        AirBox_log = new G4LogicalVolume(AirBox,CYGNOMaterials->Material("Air"),name_log);
        AirBox_log->SetVisAttributes(CYGNOMaterials->VisAttributes("Air"));
	InsideVolume_log = AirBox_log;
 
    }
    // ---------------------------------- No shielding
    else if (CYGNOShielding == "NoShield") 
    {
	  G4double AirBox_x;
	  G4double AirBox_y;
	  G4double AirBox_z;
	  G4Box* AirBox;
          AirBox_x = 7.*m;
          AirBox_y = 7.*m;
          AirBox_z = 7.*m;       
	  name_phys="AirBox";
	  name_log=name_phys+"_log";
	  name_solid=name_phys+"_solid";
	  AirBox = new G4Box(name_solid,0.5*AirBox_x,0.5*AirBox_y,0.5*AirBox_z);
	  AirBox_log = new G4LogicalVolume(AirBox,CYGNOMaterials->Material("Air"),name_log,0,0,0);

	  Shielding_log=AirBox_log;
	  size_Shielding=G4ThreeVector(AirBox_x/2.,AirBox_y/2.,AirBox_z/2.);
	  absrot_Shielding = G4RotationMatrix();
	  size_InsideVolume=G4ThreeVector(AirBox_x/2.,AirBox_y/2.,AirBox_z/2.);
	  tr_InsideVolume=G4ThreeVector(0.,0.,0.);
	  rot_InsideVolume=G4RotationMatrix();
    }
    else
    {
      G4cout << "ERROR: Something went wrong with the definition of the variable CYGNOShielding" << G4endl;
      //throw std::exception();
      exit(1);		
    }
    
    G4double tolerance = 1*mm;
    

    //**********************************************************************
    // ********* CYGNO volumes form CADMesh *****************************
    //**********************************************************************
    
    // CAD geometry is optional, as in the original analytic-gas mode.
    std::error_code pathError;
    fUseCADGeometry = std::filesystem::is_directory(CYGNOGeomPath.c_str(), pathError);
    // Identify the two supported layouts by their distinct component files.
    fUseV4Geometry = fUseCADGeometry &&
        (std::filesystem::exists(CYGNOGeomPath + std::string("/N66.stl")) ||
         std::filesystem::exists(CYGNOGeomPath + std::string("/GasBoundary.stl")));
    if (fUseCADGeometry) {
      G4cout << "Geometry mode: CAD " << (fUseV4Geometry ? "v4" : "v3")
             << " (" << CYGNOGeomPath << ")" << G4endl;
      // An existing CAD directory must be complete. Check before costly imports.
      const std::vector<G4String> required = fUseV4Geometry
          ? std::vector<G4String>{"N66.stl", "FieldCage.stl", "GEM.stl",
              "Cu_radiopure.stl", "Cu_nonradiopure.stl", "PEBase.stl", "PMMA.stl",
              "GasBoundary.stl"}
          : std::vector<G4String>{"FCSupport.stl", "GemFrame.stl", "FieldCage.stl",
              "GEM.stl", "InnerShieldCu.stl", "OuterShieldCu.stl", "PEBase.stl", "PMMABox.stl"};
      for (const auto& filename : required) {
        const G4String path = CYGNOGeomPath + "/" + filename;
        if (!std::ifstream(path).good()) {
          const G4String message = "CAD geometry directory exists, but a required mesh is missing: " + path;
          G4Exception("CYGNODetectorConstruction::Construct", "CYGNO_CADMesh",
                      FatalException, message.c_str());
        }
      }
      auto loadMesh = [this](const G4String& filename) {
        const G4String path = CYGNOGeomPath + "/" + filename;
        G4cout << path << G4endl;
        return CADMesh::TessellatedMesh::FromSTL(path);
      };
      G4cout << "Memory before loading CAD: " << GetMemoryUsageMB() << " MB" << G4endl;
      if (fUseV4Geometry) {
        mesh_N66 = loadMesh("N66.stl");
      } else {
        mesh_FCSupport = loadMesh("FCSupport.stl");
        mesh_GemFrame = loadMesh("GemFrame.stl");
      }
      mesh_FieldCage = loadMesh("FieldCage.stl");
      mesh_GEM = loadMesh("GEM.stl");
      mesh_InnerShieldCu = loadMesh(fUseV4Geometry ? "Cu_radiopure.stl" : "InnerShieldCu.stl");
      mesh_OuterShieldCu = loadMesh(fUseV4Geometry ? "Cu_nonradiopure.stl" : "OuterShieldCu.stl");
      mesh_PEBase = loadMesh("PEBase.stl");
      mesh_PMMABox = loadMesh(fUseV4Geometry ? "PMMA.stl" : "PMMABox.stl");
      if (fUseV4Geometry) {
        mesh_GasBoundary = loadMesh("GasBoundary.stl");
      }
      G4cout << "Memory after loading CAD: " << GetMemoryUsageMB() << " MB" << G4endl;
    } else {
      G4cout << "Geometry mode: nominal analytic detector; CAD directory unavailable: "
             << CYGNOGeomPath << G4endl;
    }

    //Cathode made of the same GEM material 
    G4Box* Cathode;
    const G4bool useNominalDimensions = !fUseCADGeometry || fUseV4Geometry;
    G4double Cathode_x = (useNominalDimensions ? 52. : 50.)*cm;
    G4double Cathode_y = (useNominalDimensions ? 82. : 80.)*cm;
    G4double Cathode_z = 60.*um;       
    name_phys="Cathode";
    name_log=name_phys+"_log";
    name_solid=name_phys+"_solid";
    Cathode = new G4Box(name_solid,0.5*Cathode_x,0.5*Cathode_y,0.5*Cathode_z);
    Cathode_log = new G4LogicalVolume(Cathode,CYGNOMaterials->Material("GEM"),name_log,0,0,0);

    if (fUseCADGeometry){
    
      ////Cathode 
      //cad_Cathode_solid = mesh_Cathode->GetSolid();
      //cad_Cathode_logical = new G4LogicalVolume(cad_Cathode_solid, CYGNOMaterials->Material("Cu"), "cad_Cathode_logical");
      //cad_Cathode_logical->SetVisAttributes(CYGNOMaterials->VisAttributes("Cu"));


      if (fUseV4Geometry) {
        // Combined field-cage supports and GEM frames.
        cad_N66_solid = mesh_N66->GetSolid();
        cad_N66_logical = new G4LogicalVolume(cad_N66_solid,
            CYGNOMaterials->Material("Nylon66"), "cad_N66_logical");
        cad_N66_logical->SetVisAttributes(CYGNOMaterials->VisAttributes("Perspex"));
      } else {
        cad_FCSupport_solid = mesh_FCSupport->GetSolid();
        cad_FCSupport_logical = new G4LogicalVolume(cad_FCSupport_solid,
            CYGNOMaterials->Material("Nylon66"), "cad_FCSupport_logical");
        cad_FCSupport_logical->SetVisAttributes(CYGNOMaterials->VisAttributes("Perspex"));
        cad_GemFrame_solid = mesh_GemFrame->GetSolid();
        cad_GemFrame_logical = new G4LogicalVolume(cad_GemFrame_solid,
            CYGNOMaterials->Material("Nylon66"), "cad_GemFrame_logical");
        cad_GemFrame_logical->SetVisAttributes(CYGNOMaterials->VisAttributes("Perspex"));
      }

      //FIeldCage
      cad_FieldCage_solid = mesh_FieldCage->GetSolid();
      cad_FieldCage_logical = new G4LogicalVolume(cad_FieldCage_solid, CYGNOMaterials->Material("FieldCage"), "cad_FieldCage_logical", 0, 0, 0);
      cad_FieldCage_logical->SetVisAttributes(CYGNOMaterials->VisAttributes("Perspex"));
 

      //GEM  
      cad_GEM_solid = mesh_GEM->GetSolid();
      cad_GEM_logical = new G4LogicalVolume(cad_GEM_solid, CYGNOMaterials->Material("GEM"), "cad_GEM_logical", 0, 0, 0);
      cad_GEM_logical->SetVisAttributes(CYGNOMaterials->VisAttributes("Cu"));

      //InnerShield Cu
      cad_InnerShieldCu_solid = mesh_InnerShieldCu->GetSolid();
      cad_InnerShieldCu_logical = new G4LogicalVolume(cad_InnerShieldCu_solid, CYGNOMaterials->Material("Cu"), "cad_InnerShieldCu_logical", 0, 0, 0);
      cad_InnerShieldCu_logical->SetVisAttributes(CYGNOMaterials->VisAttributes("Cu"));


      //OuterShield Cu
      cad_OuterShieldCu_solid = mesh_OuterShieldCu->GetSolid();
      cad_OuterShieldCu_logical = new G4LogicalVolume(cad_OuterShieldCu_solid, CYGNOMaterials->Material("Cu"), "cad_OuterShieldCu_logical", 0, 0, 0); 
      cad_OuterShieldCu_logical->SetVisAttributes(CYGNOMaterials->VisAttributes("Cu"));
      
        
      //PEbase
      cad_PEBase_solid = mesh_PEBase->GetSolid();
      cad_PEBase_logical = new G4LogicalVolume(cad_PEBase_solid, CYGNOMaterials->Material("PE"), "cad_PEBase_logical", 0, 0, 0);
      cad_PEBase_logical->SetVisAttributes(CYGNOMaterials->VisAttributes("PE"));
        
      //PMMABox
      cad_PMMABox_solid = mesh_PMMABox->GetSolid();
      cad_PMMABox_logical = new G4LogicalVolume(cad_PMMABox_solid, CYGNOMaterials->Material("Perspex"), "cad_PMMABox_logical", 0, 0, 0);
      cad_PMMABox_logical->SetVisAttributes(CYGNOMaterials->VisAttributes("Perspex"));

    }
  
    G4RotationMatrix new_cad_rotation;
    new_cad_rotation.rotateX(-90.*deg);
    const G4Transform3D new_cad_transform(
        new_cad_rotation, G4ThreeVector(0., 0., 1.39874923*mm));

    G4VSolid* gas_solid = nullptr;
    G4VSolid* active_positive = nullptr;
    G4VSolid* active_negative = nullptr;
    G4double CYGNO_x = (useNominalDimensions ? 500. : 484.)*mm;
    G4double CYGNO_y = (useNominalDimensions ? 800. : 784.)*mm;
    G4double CYGNO_z = (useNominalDimensions ? 510. : 470.)*mm;
    if (fUseV4Geometry) {
      // Gas follows the main PMMA wall/cover boundary; material daughters displace it.
      auto* gas_cad_solid = new G4SubtractionSolid("TPC_cad_solid",
          mesh_GasBoundary->GetSolid(), cad_PMMABox_solid);
      gas_solid = new G4DisplacedSolid("TPC_solid", gas_cad_solid, new_cad_transform);
      // Use the loaded material solids directly, so a changed support cannot
      // leave stale sensitive-gas cuts or create tessellation overlaps.
      const auto make_active = [&](const G4String& name, G4double centre_z) {
        auto* box = new G4Box(name+"_box", CYGNO_x/2., CYGNO_y/2., CYGNO_z/2.);
        const G4Transform3D material_transform(new_cad_rotation,
            G4ThreeVector(470.*mm, 0., -centre_z));
        auto* without_n66 = new G4SubtractionSolid(name+"_without_N66",
            box, cad_N66_solid, material_transform);
        return new G4SubtractionSolid(name, without_n66,
            cad_FieldCage_solid, material_transform);
      };
      const G4double active_centre = CYGNO_z/2. + Cathode_z/2.;
      active_positive = make_active("CYGNO_solid", active_centre);
      active_negative = make_active("CYGNO_negative_solid", -active_centre);
    } else {
      // Nominal gas envelope; the detailed PMMA cavity requires CAD mode.
      gas_solid = new G4Box("TPC_solid", (fUseCADGeometry ? 320. : 330.)*mm,
                              460.*mm, (fUseCADGeometry ? 530. : 524.)*mm);
      active_positive = new G4Box("CYGNO_solid", CYGNO_x/2., CYGNO_y/2., CYGNO_z/2.);
      active_negative = active_positive;

      if (!fUseCADGeometry) {
        // Six placements of this foil reproduce the reviewed STL dimensions.
        cad_GEM_solid = new G4Box("GEM_foil_solid", 509.*mm/2., 809.*mm/2., 30.*um);
        cad_GEM_logical = new G4LogicalVolume(cad_GEM_solid,
            CYGNOMaterials->Material("GEM"), "cad_GEM_logical");
        cad_GEM_logical->SetVisAttributes(CYGNOMaterials->VisAttributes("Cu"));
      }
    }
    TPC_log = new G4LogicalVolume(gas_solid,
        CYGNOMaterials->Material("CYGNO_gas"), "TPC_log", 0, 0, 0);

    CYGNO_log = new G4LogicalVolume(active_positive,
        CYGNOMaterials->Material("CYGNO_gas"), "CYGNO_log", 0, 0, 0);
    CYGNO_negative_log = new G4LogicalVolume(active_negative,
        CYGNOMaterials->Material("CYGNO_gas"), "CYGNO_negative_log", 0, 0, 0);

    //camera lenses
    G4double lens_diam = 60.*mm;
    G4double lens_thick = 83.5*mm; //set to have a correct weight
      
    name_phys="camera_lens";
    name_log=name_phys+"_log";
    name_solid=name_phys+"_solid";
    G4Tubs* camera_lens = new G4Tubs(name_solid,0.,0.5*lens_diam,0.5*lens_thick,0.*deg,360.*deg);
    camera_lens_log = new G4LogicalVolume(camera_lens,CYGNOMaterials->Material("Camera"),name_log,0,0,0);
    
    //camera body
    G4double camera_x = 84.*mm;
    G4double camera_y = 84*mm; 
    G4double camera_z = 122.3*mm; 
      
    name_phys="camera";
    name_log=name_phys+"_log";
    name_solid=name_phys+"_solid";
    G4Box* camera = new G4Box(name_solid,0.5*camera_x,0.5*camera_y,0.5*camera_z);
    camera_log = new G4LogicalVolume(camera,CYGNOMaterials->Material("Camera"),name_log,0,0,0);


   if (CYGNOLab == "LNGS"){
	tr+=G4ThreeVector(0.,-1*size_Laboratory.y()+size_Shielding.y(),size_Laboratory.z()-10*m);
	tr_cad+=G4ThreeVector(0.,1.0*m-1*size_Laboratory.y()+size_Shielding.y(),size_Laboratory.z()-10*m);	  
    
	rot = G4RotationMatrix();// rotation of daughter volume
	tr_Shielding+=(rot_Shielding*tr);
    }
    else if (CYGNOLab == "NoCave") {
	tr=G4ThreeVector(0.,0.,0.);
	tr_cad+=G4ThreeVector(0.,0.,0.);
	rot = G4RotationMatrix();
	tr_Shielding+=(rot_Shielding*tr);

    }

    Shield0_phys = new G4PVPlacement(G4Transform3D(rot,tr),Shielding_log,"Shield0",Laboratory_log,false,0,true);
    if (CYGNOShielding == "NoShield")
    {
        tr = G4ThreeVector(0.,0.,0.);//translation in mother frame
        tr_InsideVolume+=(rot_InsideVolume*tr);
        rot = G4RotationMatrix();// rotation of daughter volume
        rot_InsideVolume*=rot; //equivalent to rot_InsideVolume=rot_InsideVolume*rot
        AirBox_log = Shielding_log;
    }
    
    else if (CYGNOShielding == "FullShield") 
    {   
        // ----------------------------------- Volume placements

        tr = G4ThreeVector(0.,0.,0.);//translation in mother frame
        tr_InsideVolume+=(rot_InsideVolume*tr);
        rot = G4RotationMatrix();// rotation of daughter volume
        rot_InsideVolume*=rot; //equivalent to rot_InsideVolume=rot_InsideVolume*rot
        Shield1_phys = new G4PVPlacement(G4Transform3D(rot,tr),Shield1_log,"Shield1",Shielding_log,false,0,true);

        tr = G4ThreeVector(0.,0.,0.);//translation in mother frame
        tr_InsideVolume+=(rot_InsideVolume*tr);
        rot = G4RotationMatrix();// rotation of daughter volume
        rot_InsideVolume*=rot; //equivalent to rot_InsideVolume=rot_InsideVolume*rot
        Shield2_phys = new G4PVPlacement(G4Transform3D(rot,tr),Shield2_log,"Shield2",Shield1_log,false,0,true);

        tr = G4ThreeVector(0.,0.,0.);//translation in mother frame
        tr_InsideVolume+=(rot_InsideVolume*tr);
        rot = G4RotationMatrix();// rotation of daughter volume
        rot_InsideVolume*=rot; //equivalent to rot_InsideVolume=rot_InsideVolume*rot
        Shield3_phys = new G4PVPlacement(G4Transform3D(rot,tr),Shield3_log,"Shield3",Shield2_log,false,0,true);

        tr = G4ThreeVector(0.,0.,0.);//translation in mother frame
        tr_InsideVolume+=(rot_InsideVolume*tr);
        rot = G4RotationMatrix();// rotation of daughter volume
        rot_InsideVolume*=rot; //equivalent to rot_InsideVolume=rot_InsideVolume*rot
        AirBox_phys = new G4PVPlacement(G4Transform3D(rot,tr), AirBox_log, "AirBox", Shield3_log, false, 0,true); 
    
    }


////FIXME remove step limiter    
//    G4double maxStep = 10*um;
//    fStepLimit = new G4UserLimits(maxStep);
//    CYGNO_log->SetUserLimits(fStepLimit); 

    CYGNO_log->SetVisAttributes(CYGNOMaterials->VisAttributes("CYGNO_gas"));
    CYGNO_negative_log->SetVisAttributes(CYGNOMaterials->VisAttributes("CYGNO_gas"));
    
    //FIXME

    tr_cad=G4ThreeVector(47.*cm,0*cm,0*cm);
    rot_cad = G4RotationMatrix(0,0,0);
    rot_cad.rotateX(-90.*deg);

    const G4Transform3D shield_transform = fUseV4Geometry
        ? new_cad_transform : G4Transform3D(rot_cad,tr_cad);
    if (fUseCADGeometry){
      cad_PEBase_physical = new G4PVPlacement(G4Transform3D(rot_cad,tr_cad), 
        	    cad_PEBase_logical,"cad_PEBase_physical", AirBox_log, false, 0, true); 
      cad_InnerShieldCu_physical = new G4PVPlacement(shield_transform,
        	    cad_InnerShieldCu_logical,"cad_InnerShieldCu_physical", AirBox_log, false, 0, true);
      cad_OuterShieldCu_physical = new G4PVPlacement(shield_transform,
        	    cad_OuterShieldCu_logical,"cad_OuterShieldCu_physical", AirBox_log, false, 0, true);
      cad_PMMABox_physical = new G4PVPlacement(shield_transform,
        	    cad_PMMABox_logical,"cad_PMMABox_physical", AirBox_log, false, 0, true); 
    }
    tr=G4ThreeVector(0.,0.,0.);
    //FIXME
    TPC_phys = new G4PVPlacement(G4Transform3D(rot, tr),
      	    TPC_log,"TPC_gas", AirBox_log, false, 0, true);
    
    const G4double cathodeGap = useNominalDimensions ? 0.5*Cathode_z : 10.*mm;
    tr_CYGNO_gas_1= G4ThreeVector(0.,0.,0.5*CYGNO_z+cathodeGap);
    tr_CYGNO_gas_2= G4ThreeVector(0.,0.,-0.5*CYGNO_z-cathodeGap);
    CYGNO_phys = new G4PVPlacement(G4Transform3D(rot,tr_CYGNO_gas_1), CYGNO_log,"CYGNO_gas", TPC_log, false, 0, true);
    CYGNO_phys = new G4PVPlacement(G4Transform3D(rot,tr_CYGNO_gas_2), CYGNO_negative_log,"CYGNO_gas", TPC_log, false, 1, true);

    //camera lens
    tr_camera_lens_L0 = G4ThreeVector(0.,265.*mm,-1200.*mm);
    tr_camera_lens_L1 = G4ThreeVector(0.,0.,-1200.*mm);
    tr_camera_lens_L2 = G4ThreeVector(0.,-265.*mm,-1200.*mm);
    tr_camera_lens_R0 = G4ThreeVector(0.,265.*mm,1200.*mm);
    tr_camera_lens_R1 = G4ThreeVector(0.,0.,1200.*mm);
    tr_camera_lens_R2 = G4ThreeVector(0.,-265.*mm,1200.*mm);
 
    camera_lens_phys = new G4PVPlacement(G4Transform3D(rot,tr_camera_lens_L0), camera_lens_log,"camera_lens_L", AirBox_log, false, 0, true);
    camera_lens_phys = new G4PVPlacement(G4Transform3D(rot,tr_camera_lens_L1), camera_lens_log,"camera_lens_L", AirBox_log, false, 1, true);
    camera_lens_phys = new G4PVPlacement(G4Transform3D(rot,tr_camera_lens_L2), camera_lens_log,"camera_lens_L", AirBox_log, false, 2, true);
    camera_lens_phys = new G4PVPlacement(G4Transform3D(rot,tr_camera_lens_R0), camera_lens_log,"camera_lens_R", AirBox_log, false, 0, true);
    camera_lens_phys = new G4PVPlacement(G4Transform3D(rot,tr_camera_lens_R1), camera_lens_log,"camera_lens_R", AirBox_log, false, 1, true);
    camera_lens_phys = new G4PVPlacement(G4Transform3D(rot,tr_camera_lens_R2), camera_lens_log,"camera_lens_R", AirBox_log, false, 2, true);
   

    //camera
    tr_camera_L0 = G4ThreeVector(0.,265.*mm,-1200.*mm-0.5*camera_z-0.5*lens_thick);
    tr_camera_L1 = G4ThreeVector(0.,0.,-1200.*mm-0.5*camera_z-0.5*lens_thick);
    tr_camera_L2 = G4ThreeVector(0.,-265.*mm,-1200.*mm-0.5*camera_z-0.5*lens_thick);
    tr_camera_R0 = G4ThreeVector(0.,265.*mm,1200.*mm+0.5*camera_z+0.5*lens_thick);
    tr_camera_R1 = G4ThreeVector(0.,0.,1200.*mm+0.5*camera_z+0.5*lens_thick);
    tr_camera_R2 = G4ThreeVector(0.,-265.*mm,1200.*mm+0.5*camera_z+0.5*lens_thick);
    
    camera_phys = new G4PVPlacement(G4Transform3D(rot,tr_camera_L0), camera_log,"camera_L", AirBox_log, false, 0, true);
    camera_phys = new G4PVPlacement(G4Transform3D(rot,tr_camera_L1), camera_log,"camera_L", AirBox_log, false, 1, true);
    camera_phys = new G4PVPlacement(G4Transform3D(rot,tr_camera_L2), camera_log,"camera_L", AirBox_log, false, 2, true);
    camera_phys = new G4PVPlacement(G4Transform3D(rot,tr_camera_R0), camera_log,"camera_R", AirBox_log, false, 0, true);
    camera_phys = new G4PVPlacement(G4Transform3D(rot,tr_camera_R1), camera_log,"camera_R", AirBox_log, false, 1, true);
    camera_phys = new G4PVPlacement(G4Transform3D(rot,tr_camera_R2), camera_log,"camera_R", AirBox_log, false, 2, true);

    tr=G4ThreeVector(0.,0.,0.);
    rot = G4RotationMatrix();
    Cathode_phys = new G4PVPlacement(G4Transform3D(rot,tr), Cathode_log,"Cathode", TPC_log, false, 0, true);
    if (!fUseCADGeometry) {
      const G4double gemThickness = 60.*um;
      const G4double gemGap = 2.*mm;
      int copy = 0;
      for (const int side : {-1, 1}) {
        for (int layer=0; layer<3; ++layer) {
          const G4double z = side*(Cathode_z/2. + CYGNO_z
              + gemThickness/2. + layer*(gemThickness+gemGap));
          cad_GEM_physical = new G4PVPlacement(
              G4Transform3D(rot,G4ThreeVector(0.,0.,z)), cad_GEM_logical,
              "cad_GEM_physical", TPC_log, false, copy++, true);
        }
      }
    }
    if (fUseCADGeometry){
      cad_FieldCage_physical = new G4PVPlacement(G4Transform3D(rot_cad,tr_cad), 
        	    cad_FieldCage_logical,"cad_FieldCage_physical", TPC_log, false, 0, true);
      if (fUseV4Geometry) {
        cad_N66_physical = new G4PVPlacement(G4Transform3D(rot_cad,tr_cad),
            cad_N66_logical,"cad_N66_physical", TPC_log, false, 0, true);
      } else {
        cad_FCSupport_physical = new G4PVPlacement(G4Transform3D(rot_cad,tr_cad),
            cad_FCSupport_logical,"cad_FCSupport_physical", TPC_log, false, 0, true);
        cad_GemFrame_physical = new G4PVPlacement(G4Transform3D(rot_cad,tr_cad),
            cad_GemFrame_logical,"cad_GemFrame_physical", TPC_log, false, 0, true);
      }
      cad_GEM_physical = new G4PVPlacement(G4Transform3D(rot_cad,tr_cad), 
        	    cad_GEM_logical,"cad_GEM_physical", TPC_log, false, 0, true);
    }  
    
	//camera_phys = new G4PVPlacement(rotcam0,trcam0,camera_log,"camera",AirBox_log, false, 0, true);
        //camera_lens_phys = new G4PVPlacement(rotlens0,trlens0,camera_lens_log,"camera_lens",AirBox_log, false, 0, true);

    //
    //**********************************************************************
    // GLOBAL TRANSLATIONS ***************************************
    G4cout<<"Placement of Laboratory in the World started"<<G4endl;
    
    tr_Rock=-1*(tr_Laboratory+rot_Laboratory*(tr+rot_cad*(tr_InsideVolume+rot_InsideVolume*(tr_Shielding))));//The shift of Rock_log in the world volume to make the origin be the center of the detector

    G4RotationMatrix rot_check = absrot_Rock*(rot_Laboratory*(rot_cad*(rot_InsideVolume)));  
    name_phys="externalRock";
    name_log=name_phys+"_log";
    name_solid=name_phys+"_solid";
    externalRock_phys = new G4PVPlacement(G4Transform3D(absrot_Rock,tr_Rock),Rock_log,name_phys,WorldVolume_log,false,0,true);

    G4cout << "The main CYGNO volume is translated w.r.t the center of the rock volume of:\t x="<< -1*tr_Rock.x()/cm << " cm\t y=" << -1*tr_Rock.y()/cm << " cm\t z=" << -1*tr_Rock.z()/cm << " cm"<< G4endl;

    G4cout << "The Rock volume has been translated to put the main CYGNO volume in the center of the coordinate system"<< G4endl;
    G4cout<<"Placement of Laboratory in the World ended"<<G4endl;
 
    //======= Save volumes mass and density ======
    
    G4cout<<"Saving masses and densities of the volumes"<<G4endl;
    SaveMassAndDensity();


    //===========
    return WorldVolume_phys;


}

void CYGNODetectorConstruction::SaveMassAndDensity()
{
  CYGNODetectorProperty* CYGNOProperties = CYGNODetectorProperty::GetInstance();

  G4cout << "Saving masses and densities of the volumes of the CYGNODetectorConstruction class"<< G4endl;
  //CYGNOProperties->AddVolumeNameMassAndDensity(Rock_log);
  //CYGNOProperties->AddVolumeNameMassAndDensity(Laboratory_log);
  if (CYGNOShielding=="FullShield"){
      CYGNOProperties->AddVolumeNameMassAndDensity(Shielding_log);
      CYGNOProperties->AddVolumeNameMassAndDensity(Shield1_log);
      CYGNOProperties->AddVolumeNameMassAndDensity(Shield2_log);
      CYGNOProperties->AddVolumeNameMassAndDensity(Shield3_log);
  }
  CYGNOProperties->AddVolumeNameMassAndDensity(AirBox_log);
  CYGNOProperties->AddVolumeNameMassAndDensity(TPC_log);
  CYGNOProperties->AddVolumeNameMassAndDensity(CYGNO_log);
  CYGNOProperties->AddVolumeNameMassAndDensity(CYGNO_negative_log);
  // TPC_log mass alone excludes its same-material sensitive daughters.
  const G4double totalGasMass = TPC_log->GetMass(true,false)
      + CYGNO_log->GetMass(true,false) + CYGNO_negative_log->GetMass(true,false);
  const G4double gasDensity = TPC_log->GetMaterial()->GetDensity();
  CYGNOProperties->AddVolumeNameMass("TPC_total_gas_mass", totalGasMass);
  CYGNOProperties->AddVolumeNameDensity("TPC_total_gas_dens", gasDensity);
  G4cout << "Total chamber gas mass (kg): " << totalGasMass/kg << G4endl;
  G4cout << "Total chamber gas volume (L): "
         << totalGasMass/gasDensity/(1000.*cm3) << G4endl;
  CYGNOProperties->AddVolumeNameMassAndDensity(Cathode_log);
  if (!fUseCADGeometry) {
    // The shared analytic logical volume describes one of six physical foils.
    const G4double gemMass = 6.*cad_GEM_logical->GetMass(true,false);
    CYGNOProperties->AddVolumeNameMass("cad_GEM_logical_mass", gemMass);
    CYGNOProperties->AddVolumeNameDensity("cad_GEM_logical_dens",
        cad_GEM_logical->GetMaterial()->GetDensity());
    G4cout << "Total GEM foil mass (kg): " << gemMass/kg << G4endl;
  }
  CYGNOProperties->AddVolumeNameMassAndDensity(camera_log);
  CYGNOProperties->AddVolumeNameMassAndDensity(camera_lens_log);
  if (fUseCADGeometry) {
    //CYGNOProperties->AddVolumeNameMassAndDensity(cad_Cathode_logical);
    if (fUseV4Geometry) {
      CYGNOProperties->AddVolumeNameMassAndDensity(cad_N66_logical);
    } else {
      CYGNOProperties->AddVolumeNameMassAndDensity(cad_FCSupport_logical);
      CYGNOProperties->AddVolumeNameMassAndDensity(cad_GemFrame_logical);
    }
    CYGNOProperties->AddVolumeNameMassAndDensity(cad_FieldCage_logical);
    CYGNOProperties->AddVolumeNameMassAndDensity(cad_GEM_logical);
    CYGNOProperties->AddVolumeNameMassAndDensity(cad_InnerShieldCu_logical);
    CYGNOProperties->AddVolumeNameMassAndDensity(cad_OuterShieldCu_logical);
    CYGNOProperties->AddVolumeNameMassAndDensity(cad_PEBase_logical);
    CYGNOProperties->AddVolumeNameMassAndDensity(cad_PMMABox_logical);
  //  CYGNOProperties->AddVolumeNameMassAndDensity(cad_WaterShielding_logical);
  }

//  if ( productionRockThinTube_phys )
//	{
//	  CYGNOProperties->AddPhysVolumeNameMassAndDensity(productionRockThinTube_phys);
//	}
  G4cout << "All volume masses and densities saved"<< G4endl;
}

//FIXME UpdateGeometry() is NOT MT-safe!!!
void CYGNODetectorConstruction::UpdateGeometry()
{

  G4cout << "Updating the Geometry"<< G4endl;
  CYGNODetectorProperty* CYGNOProperties = CYGNODetectorProperty::GetInstance();
  CYGNOProperties->Refresh();

  //Removing sensitive detectors
  CYGNO_log->SetSensitiveDetector(0);
  CYGNO_negative_log->SetSensitiveDetector(0);
  CYGNO_log=0;
  CYGNO_negative_log=0;

  //Deleting all the solids, logical and physical objects
  //G4RunManager::GetRunManager()->ReinitializeGeometry(true);
  //Equivalent to
  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore* PhysicalVolumeStore = G4PhysicalVolumeStore::GetInstance();
  PhysicalVolumeStore->Clean();
  G4LogicalVolumeStore* LogicalVolumeStore = G4LogicalVolumeStore::GetInstance();
  LogicalVolumeStore->Clean();
  G4SolidStore* SolidStore = G4SolidStore::GetInstance();
  SolidStore->Clean();
  
  CYGNODetectorMaterial* CYGNOMaterials = CYGNODetectorMaterial::GetInstance();
  CYGNOMaterials->Refresh();


  //The memory for these pointers has been freed by the above Clean() methods
  CYGNODetectorLNGS* CYGNOLNGS = CYGNODetectorLNGS::GetInstance();
  CYGNOLNGS->Refresh();

  Shield0_log=0; 
  Shield1_log=0; 
  Shield2_log=0; 
  Shield3_log=0; 
  AirBox_log=0;

  //cad_Cathode_logical=0;
  cad_N66_logical=0;
  cad_FieldCage_logical=0;
  cad_InnerShieldCu_logical=0;
  cad_OuterShieldCu_logical=0;
  //cad_Cathode_logical=0;
  cad_GEM_logical=0;
  cad_PEBase_logical=0;
  cad_PMMABox_logical=0;
  
  TPC_log=0;
  CYGNO_log=0;
  CYGNO_negative_log=0;
  camera_log=0;
  camera_lens_log=0;
  Cathode_log = 0;

  InsideVolume_log=0;
  Shielding_log=0;
  Laboratory_log=0;
  Rock_log=0;
  WorldVolume_log=0;
  productionRockThinTube_phys=0;
  //log->ClearDaughters();

  // Delete all the geometry you had defined and build everything from scratch
  G4RunManager::GetRunManager()->DefineWorldVolume(Construct());
  G4RunManager::GetRunManager()->GeometryHasBeenModified();
}
void CYGNODetectorConstruction::UpdateGeometryPath(G4String newpath)
{
  G4cout << "Updating the Geometry path to "<< newpath << G4endl;
  SetGeomPath(newpath); 

}

void CYGNODetectorConstruction::ConstructSDandField()
{
  // G4SDManager::GetSDMpointer()->SetVerboseLevel(1);

  //
  // Sensitive detectors
  //
  auto cygnoSD
    = new CYGNOSensitiveDetector("cygnoSD");
  G4SDManager::GetSDMpointer()->AddNewDetector(cygnoSD);
  SetSensitiveDetector("CYGNO_log",cygnoSD);
  SetSensitiveDetector("CYGNO_negative_log",cygnoSD);

}
