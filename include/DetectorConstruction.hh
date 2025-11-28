// DetectorConstruction.hh
#ifndef DETECTOR_CONSTRUCTION_HH
#define DETECTOR_CONSTRUCTION_HH

#include <G4VUserDetectorConstruction.hh>
#include <G4LogicalVolume.hh>
#include <G4VPhysicalVolume.hh>

// DetectorConstruction 继承自 G4VUserDetectorConstruction
// 这是 Geant4 中定义几何结构的基类
class DetectorConstruction : public G4VUserDetectorConstruction {
public:
    DetectorConstruction();
    ~DetectorConstruction();
    // Construct() 是必须实现的函数
    // 它会被 G4RunManager 调用，用来定义几何和材料
    virtual G4VPhysicalVolume* Construct();
};

#endif


