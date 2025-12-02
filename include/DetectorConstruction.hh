// DetectorConstruction.hh
#ifndef DETECTOR_CONSTRUCTION_HH
#define DETECTOR_CONSTRUCTION_HH

#include "G4GenericMessenger.hh"
#include "G4SystemOfUnits.hh"
#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4MaterialPropertiesTable.hh>
#include <G4VPhysicalVolume.hh>
#include <G4VUserDetectorConstruction.hh>

// DetectorConstruction 继承自 G4VUserDetectorConstruction
// 这是 Geant4 中定义几何结构的基类
class DetectorConstruction : public G4VUserDetectorConstruction {
public:
  DetectorConstruction();
  ~DetectorConstruction();
  // Construct() 是必须实现的函数
  // 它会被 G4RunManager 调用，用来定义几何和材料
  G4double crystalSize = 10 * cm; // CsI 晶体边长
  G4double gap = 1 * mm;          // 间隙
  virtual G4VPhysicalVolume *Construct();
  virtual void ConstructSDandField();

private:
  G4GenericMessenger *fMessenger;
  G4String fGapMaterial;
  G4Material *fAir;
  G4Material *fOpticalGrease;
  G4Material *fCsI;
  G4MaterialPropertiesTable *fMptAir;
  G4MaterialPropertiesTable *fMptGrease;
  G4MaterialPropertiesTable *fMptCsI;
  void DefineMaterials();
  void SetVisualizationAttributes(G4LogicalVolume *worldLV,
                                  G4LogicalVolume *gapLV,
                                  G4LogicalVolume *csiLV);
};

#endif
