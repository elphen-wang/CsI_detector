#include "DetectorConstruction.hh"

#include <G4Box.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4NistManager.hh>
#include <G4SystemOfUnits.hh>
#include <G4VisAttributes.hh>   // 可视化属性
#include "DetectorSD.hh"
#include "G4SDManager.hh"

DetectorConstruction::DetectorConstruction() {}
DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume* DetectorConstruction::Construct() {
    // 材料管理器
    G4NistManager* nist = G4NistManager::Instance();

    // =========================
    // 1. 创建世界体积
    // =========================
    G4Material* worldMat = nist->FindOrBuildMaterial("G4_AIR");

    // 先计算阵列总尺寸
    G4double crystalSize = 6*cm;   // CsI 晶体边长
    G4double gap = 0.5*cm;         // 间隙

    G4int nx = 8; // x方向晶体数
    G4int ny = 8; // y方向晶体数
    G4int nz = 5; // z方向晶体数

    // 阵列总尺寸（晶体尺寸 + 间隙）* 数量
    G4double totalX = nx * crystalSize + (nx - 1) * gap;
    G4double totalY = ny * crystalSize + (ny - 1) * gap;
    G4double totalZ = nz * crystalSize + (nz - 1) * gap;

    // 世界尺寸比阵列大一些
    G4double worldSizeX = totalX + 20*cm;
    G4double worldSizeY = totalY + 20*cm;
    G4double worldSizeZ = totalZ + 20*cm;

    // 世界 Solid
    G4Box* worldBox = new G4Box("World", worldSizeX/2, worldSizeY/2, worldSizeZ/2);

    // 世界逻辑
    G4LogicalVolume* worldLV = new G4LogicalVolume(worldBox, worldMat, "World");

    // 世界物理
    G4VPhysicalVolume* worldPV = new G4PVPlacement(
        0,
        G4ThreeVector(),
        worldLV,
        "World",
        0,
        false,
        0
    );

    // =========================
    // 2. 创建 CsI 晶体阵列
    // =========================
    G4Material* csiMat = nist->FindOrBuildMaterial("G4_CESIUM_IODIDE");

    // CsI Solid
    G4Box* csiBox = new G4Box("CsI", crystalSize/2, crystalSize/2, crystalSize/2);

    // CsI 逻辑体
    G4LogicalVolume* csiLV = new G4LogicalVolume(csiBox, csiMat, "CsI");

    // 阵列中心对齐到世界中心
    G4double startX = -totalX/2 + crystalSize/2;
    G4double startY = -totalY/2 + crystalSize/2;
    G4double startZ = -totalZ/2 + crystalSize/2;

    // 三重循环放置晶体
    for (G4int ix = 0; ix < nx; ix++) {
        for (G4int iy = 0; iy < ny; iy++) {
            for (G4int iz = 0; iz < nz; iz++) {
                G4double posX = startX + ix * (crystalSize + gap);
                G4double posY = startY + iy * (crystalSize + gap);
                G4double posZ = startZ + iz * (crystalSize + gap);

                new G4PVPlacement(
                    0,
                    G4ThreeVector(posX, posY, posZ),
                    csiLV,
                    "CsI",
                    worldLV,
                    false,
                    ix * ny * nz + iy * nz + iz  // 副本编号
                );
            }
        }
    }
    
    // 阵列逻辑体csiLV 已创建
    // 挂载敏感探测器
    G4SDManager* sdManager = G4SDManager::GetSDMpointer();
    DetectorSD* detectorSD = new DetectorSD("CsISD");
    sdManager->AddNewDetector(detectorSD);
    csiLV->SetSensitiveDetector(detectorSD);

    // =========================
    // 3. 设置可视化属性
    // =========================
    // 世界体积：蓝色半透明
    G4VisAttributes* visWorld = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 0.1)); // 蓝色, 透明度0.1
    visWorld->SetForceSolid(true); // 实体填充但半透明
    worldLV->SetVisAttributes(visWorld);

    // CsI 晶体：白色半透明
    G4VisAttributes* visCsI = new G4VisAttributes(G4Colour(1.0, 1.0, 1.0, 0.5)); // 白色, 透明度0.5
    visCsI->SetForceSolid(true);
    csiLV->SetVisAttributes(visCsI);

    return worldPV;
}
