#include "CYGNODetectorLNGS.hh"

#include "G4VisAttributes.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4EllipticalTube.hh"
#include "G4Ellipsoid.hh"
#include "G4IntersectionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4UnionSolid.hh"
#include "G4AssemblyVolume.hh"
#include "G4PVPlacement.hh"
#include "G4Transform3D.hh"
#include "G4SubtractionSolid.hh"
#include "G4PVDivision.hh"
#include "G4SDManager.hh"


CYGNODetectorLNGS* CYGNODetectorLNGS::fCYGNODetectorLNGS = NULL;

CYGNODetectorLNGS::CYGNODetectorLNGS() : 
  Rock_log(0),
  size_Rock(),
  absrot_Rock(),
  Laboratory_log(0),
  size_Laboratory(),
  tr_Laboratory(),
  rot_Laboratory(),
  //Shielding_log(0),
  //size_Shielding(),
  //absrot_Shielding(),
  InsideVolume_log(0),
  size_InsideVolume(),
  tr_InsideVolume(),
  rot_InsideVolume(),
  rockThicknessOuter(5.0*m), 
  rockThicknessInner(0.*cm),
  productionLayerThickness(40.*cm)
{


  Construct();
  
}

CYGNODetectorLNGS::~CYGNODetectorLNGS()
{
}

CYGNODetectorLNGS* CYGNODetectorLNGS::GetInstance()
{
  if (fCYGNODetectorLNGS == NULL) {
    fCYGNODetectorLNGS = new CYGNODetectorLNGS();
  }
  return fCYGNODetectorLNGS;
}

void CYGNODetectorLNGS::UpdateGeometry()
{
  Construct();
}

void CYGNODetectorLNGS::Construct()
{  
  ConstructRock();
  //ConstructShielding();
}

void CYGNODetectorLNGS::ConstructRock()
{  
  //G4cout << "Constructing materials...";
  //CYGNOMaterials = CYGNODetectorMaterial::GetInstance();
  //G4cout << "... done" << G4endl;
  
  absrot_Rock = G4RotationMatrix();

  //Name of the volumes
  G4String name_solid="";
  G4String name_log="";
  G4String name_phys="";
  G4ThreeVector  tr;
  tr_Laboratory = G4ThreeVector(0.,0.,0.);
  G4RotationMatrix rot;
  rot_Laboratory = G4RotationMatrix();
 
  // ------------ Building Hall B at LNGS according to Chiara Zarra drawings--------------   
  const G4double HallWallThickness = 0.0*cm;//No concrete at the moment
  const G4double HallSizeWidth = 15.0*m;
  const G4double HallSizeHeight = 14.0*m;
  const G4double HallSizeLength = 125.0*m;


  size_Laboratory = G4ThreeVector((HallSizeWidth)/2.,
								  (HallSizeHeight)/2.,
								  (HallSizeLength)/2.);
  size_Rock = G4ThreeVector((HallSizeWidth + 2.*rockThicknessOuter + 2.*HallWallThickness)/2.,
							(HallSizeHeight + 2.*rockThicknessOuter + 2.*HallWallThickness)/2.,
							(HallSizeLength + 2.*rockThicknessOuter + 2.*HallWallThickness)/2.);
  
  
  // ----------------------------- Layered Rock Volume
  //Outmost layer:
  G4Box* externalRockBox = new G4Box("externalRockBox",
									 size_Rock.x(),
									 size_Rock.y(),
									 size_Rock.z());

  G4EllipticalTube* externalRockTube = new G4EllipticalTube("externalRockTube",
									 size_Rock.x(),
									 size_Rock.y()*2,
									 size_Rock.z());
  
  G4ThreeVector rockTranslation(0,-1.*size_Rock.y(),0);
  name_phys="externalRock";
  name_log=name_phys+"_log";
  name_solid=name_phys+"_solid";
  G4IntersectionSolid* externalRockSolid = new G4IntersectionSolid(name_solid,
																   externalRockBox,
																   externalRockTube,
																   0,
																   rockTranslation);
  
  externalRock_log = new G4LogicalVolume(externalRockSolid,CYGNOMaterials->Material("lngsRock"),name_log,0,0,0);
  G4VisAttributes* RockExternalVisAtt = new G4VisAttributes(G4Color(1.,0.,0.));
  externalRock_log->SetVisAttributes(RockExternalVisAtt);
  Rock_log = externalRock_log;
  
  //You can set the thickness (productionLayerThickness) and the depth (rockThicknessInner) in which the generated particles are confined in the rock (productionRock). Only that sort of shell region will be the one where particles are generated.

  //Production Layer:
  G4Box* productionRockBox = new G4Box("productionRockBox",
									   size_Laboratory.x() + rockThicknessInner + productionLayerThickness + HallWallThickness,
									   size_Laboratory.y() + rockThicknessInner + productionLayerThickness + HallWallThickness,
									   size_Laboratory.z() + rockThicknessInner + productionLayerThickness + HallWallThickness);
  
  G4EllipticalTube* productionRockTube = new G4EllipticalTube("productionRockTube",
															  size_Laboratory.x() + rockThicknessInner + productionLayerThickness + HallWallThickness,
															  (size_Laboratory.y() + rockThicknessInner + productionLayerThickness + HallWallThickness)*2.,
															  size_Laboratory.z() + rockThicknessInner + productionLayerThickness + HallWallThickness);
  
  G4ThreeVector productionRockTranslation(0,-1.*(size_Laboratory.y() + rockThicknessInner + productionLayerThickness + HallWallThickness),0);
  name_phys="productionRock";
  name_log=name_phys+"_log";
  name_solid=name_phys+"_solid";
  G4IntersectionSolid* productionRockSolid = new G4IntersectionSolid(name_solid,
																	 productionRockBox,
																	 productionRockTube,
																	 0,
																	 productionRockTranslation);
  
  productionRock_log = new G4LogicalVolume(productionRockSolid,CYGNOMaterials->Material("lngsRock"),name_log,0,0,0);
  G4VisAttributes* ProductionLayerVisAtt = new G4VisAttributes(G4Color(0.,1.,0.));
  productionRock_log->SetVisAttributes(ProductionLayerVisAtt);
  
  tr = G4ThreeVector(0.,0.,0.);//translation in mother frame
  tr_Laboratory+=(rot_Laboratory*tr);
  rot = G4RotationMatrix();// rotation of daughter volume
  rot_Laboratory*=rot; //equivalent to rot_Laboratory=rot_Laboratory*rot
  productionRock_phys = new G4PVPlacement(G4Transform3D(rot,tr),productionRock_log,name_phys,externalRock_log,false,0,true);

  //Innermost Layer:
  G4Box* internalRockBox = new G4Box("internalRockBox",
									 size_Laboratory.x() + rockThicknessInner + HallWallThickness,
									 size_Laboratory.y() + rockThicknessInner + HallWallThickness,
									 size_Laboratory.z() + rockThicknessInner + HallWallThickness);
  
  G4EllipticalTube* internalRockTube = new G4EllipticalTube("internalRockTube",
															size_Laboratory.x() + rockThicknessInner + HallWallThickness,
															(size_Laboratory.y() + rockThicknessInner + HallWallThickness)*2.,
															size_Laboratory.z() + rockThicknessInner + HallWallThickness);
  
  G4ThreeVector internalRockTranslation(0,-1.*(size_Laboratory.y() + rockThicknessInner + HallWallThickness),0);
  name_phys="internalRock";
  name_log=name_phys+"_log";
  name_solid=name_phys+"_solid";
  G4IntersectionSolid* internalRockSolid = new G4IntersectionSolid(name_solid,
																   internalRockBox,
																   internalRockTube,
																   0,
																   internalRockTranslation);
  
  internalRock_log = new G4LogicalVolume(internalRockSolid,CYGNOMaterials->Material("lngsRock"),name_log,0,0,0);
  internalRock_log->SetVisAttributes(ProductionLayerVisAtt);
  
  tr = G4ThreeVector(0.,0.,0.);//translation in mother frame
  tr_Laboratory+=(rot_Laboratory*tr);
  rot = G4RotationMatrix();// rotation of daughter volume
  rot_Laboratory*=rot; //equivalent to rot_Laboratory=rot_Laboratory*rot
  internalRock_phys = new G4PVPlacement(G4Transform3D(rot,tr),internalRock_log,name_phys,productionRock_log,false,0,true);
  
  //Experimental Hall B at LNGS according to Chiara Zarra drawings
  //Concrete Wall ------------------------------------------------------------------------------------------------------
  G4Box* expHallWallBox = new G4Box("expHallWallBox",
									size_Laboratory.x() + HallWallThickness,
									size_Laboratory.y() + HallWallThickness,
									size_Laboratory.z() + HallWallThickness);
  
  G4EllipticalTube* expHallWallTube = new G4EllipticalTube("expHallWallTube",
														   size_Laboratory.x() + HallWallThickness,
														   (size_Laboratory.y() + HallWallThickness)*2.,
														   size_Laboratory.z() + HallWallThickness);
  
  G4ThreeVector expHallWallTranslation(0,-1.*(size_Laboratory.y() + rockThicknessInner + HallWallThickness),0);
  name_phys="expHallWall";
  name_log=name_phys+"_log";
  name_solid=name_phys+"_solid";
  G4IntersectionSolid* expHallWallSolid = new G4IntersectionSolid(name_solid,
																  expHallWallBox,
																  expHallWallTube,
																  0,
																  expHallWallTranslation);
  
  expHallWall_log = new G4LogicalVolume(expHallWallSolid,CYGNOMaterials->Material("concrete"),name_log,0,0,0);
//  expHallWall_log = new G4LogicalVolume(expHallWallSolid,CYGNOMaterials->Material("lngsRock"),name_log,0,0,0);
  
  tr = G4ThreeVector(0.,0.,0.);//translation in mother frame
  tr_Laboratory+=(rot_Laboratory*tr);
  rot = G4RotationMatrix();// rotation of daughter volume
  rot_Laboratory*=rot; //equivalent to rot_Laboratory=rot_Laboratory*rot
  expeHallWall_phys = new G4PVPlacement(G4Transform3D(rot,tr),expHallWall_log,name_phys,internalRock_log,false,0,true);
  
  //Hall---------------------------------------------------------------------------------------------------------------
  G4Box* expHallBox = new G4Box("expHallBox",
								size_Laboratory.x(),
								size_Laboratory.y(),
								size_Laboratory.z());

  
  G4EllipticalTube* expHallTube = new G4EllipticalTube("expHallTube",
													   size_Laboratory.x(),
													   size_Laboratory.y()*2.,
													   size_Laboratory.z());
  
  G4ThreeVector expHallTranslation(0,-1.*size_Laboratory.y(),0);
  name_phys="expHall";
  name_log=name_phys+"_log";
  name_solid=name_phys+"_solid";
  G4IntersectionSolid* expHallSolid = new G4IntersectionSolid(name_solid,
														  expHallBox,
														  expHallTube,
														  0,
														  expHallTranslation);
  expHall_log = new G4LogicalVolume(expHallSolid,CYGNOMaterials->Material("air"),name_log,0,0,0);
  Laboratory_log = expHall_log;

  tr = G4ThreeVector(0.,0.,0.);//translation in mother frame
  tr_Laboratory+=(rot_Laboratory*tr);
  rot = G4RotationMatrix();// rotation of daughter volume
  rot_Laboratory*=rot; //equivalent to rot_Laboratory=rot_Laboratory*rot
  expHall_phys = new G4PVPlacement(G4Transform3D(rot,tr),expHall_log,name_phys,expHallWall_log,false,0,true);
  
  
  //These variables are used to set the thin tube for depth studies in the externalRock_log
  rockdist_z = size_Laboratory.z() + HallWallThickness + (rockThicknessOuter/2.);
  rockdepth_z = rockThicknessOuter;
    
}

//void CYGNODetectorLNGS::ConstructShielding()
//{
//
//  absrot_Shielding = G4RotationMatrix();
//
//  //Name of the volumes
//  G4String name_solid="";
//  G4String name_log="";
//  G4String name_phys="";
//  G4ThreeVector  tr;
//  tr_InsideVolume = G4ThreeVector(0.,0.,0.);
//  G4RotationMatrix rot;
//  rot_InsideVolume = G4RotationMatrix();
//
//  size_InsideVolume = G4ThreeVector(0.,0.,0.);
//  size_Shielding = G4ThreeVector(0.,0.,0.);
//  
//  Shielding_log = 0;  
//  InsideVolume_log = 0;
//
//}


void CYGNODetectorLNGS::SaveMassAndDensity()
{

} 
void CYGNODetectorLNGS::SetDetectorMaterial(CYGNODetectorMaterial* materials)
{
	CYGNOMaterials = materials;
}
void CYGNODetectorLNGS::SetExternalRockThickness(G4double rockthick)
{
	rockThicknessOuter = rockthick;
}
void CYGNODetectorLNGS::SetProductionRockThickness(G4double rockthick)
{
	productionLayerThickness = rockthick;
	//SetExternalRockThickness(productionLayerThickness+rockThicknessInner+2*m);//The additional 2m is to consider recoils from the outer rock towards the inside of the experiment
}
void CYGNODetectorLNGS::SetInternalRockThickness(G4double rockthick)
{
	rockThicknessInner = rockthick;
	//SetExternalRockThickness(productionLayerThickness+rockThicknessInner+2*m);//The additional 2m is to consider recoils from the outer rock towards the inside of the experiment
}

void CYGNODetectorLNGS::Refresh()
{  
  InsideVolume_log = 0;
  //Shielding_log = 0;
  Laboratory_log = 0;
  Rock_log = 0;
}

