#include "DetectorConstruction.hh"

#include "DetectorSD.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4OpticalSurface.hh"
#include "G4SDManager.hh"
#include <G4Box.hh>
#include <G4LogicalVolume.hh>
#include <G4NistManager.hh>
#include <G4PVPlacement.hh>
#include <G4SystemOfUnits.hh>
#include <G4VisAttributes.hh> // 可视化属性

DetectorConstruction::DetectorConstruction() : fGapMaterial("Air") {
  fMessenger = new G4GenericMessenger(this, "/CsI/detector/",
                                      "Detector construction control");
  fMessenger->DeclareProperty(
      "gapMaterial", fGapMaterial,
      "Material for gaps between crystals: Air or OpticalGrease");
}

DetectorConstruction::~DetectorConstruction() { delete fMessenger; }

G4VPhysicalVolume *DetectorConstruction::Construct() {
  // 定义所有材料
  DefineMaterials();

  // 先计算阵列总尺寸
  G4int nx = 8; // x方向晶体数
  G4int ny = 8; // y方向晶体数
  G4int nz = 5; // z方向晶体数

  // 阵列总尺寸（晶体尺寸 + 间隙）* 数量
  G4double totalX = nx * crystalSize + (nx - 1) * gap;
  G4double totalY = ny * crystalSize + (ny - 1) * gap;
  G4double totalZ = nz * crystalSize + (nz - 1) * gap;

  // =========================
  // 1. 创建世界体积
  // =========================
  // 世界尺寸比阵列大一些
  G4double worldSizeX = totalX + 20 * cm;
  G4double worldSizeY = totalY + 20 * cm;
  G4double worldSizeZ = totalZ + 20 * cm;

  // 世界 Solid
  G4Box *worldBox =
      new G4Box("World", worldSizeX / 2, worldSizeY / 2, worldSizeZ / 2);

  // 世界逻辑
  G4LogicalVolume *worldLV = new G4LogicalVolume(worldBox, fAir, "World");

  // 世界物理
  G4VPhysicalVolume *worldPV =
      new G4PVPlacement(0, G4ThreeVector(), worldLV, "World", 0, false, 0);

  // =========================
  // 1.5. 创建间隙体积
  // =========================
  G4Material *gapMat;
  if (fGapMaterial == "OpticalGrease") {
    gapMat = fOpticalGrease;
  } else {
    gapMat = fAir;
  }

  G4Box *gapBox = new G4Box("Gap", totalX / 2 - 0.1 * mm, totalY / 2 - 0.1 * mm,
                            totalZ / 2 - 0.1 * mm);
  G4LogicalVolume *gapLV = new G4LogicalVolume(gapBox, gapMat, "Gap");
  new G4PVPlacement(0, G4ThreeVector(), gapLV, "Gap", worldLV, false, 0);

  // =========================
  // 2. 创建 CsI 晶体阵列
  // =========================
  // CsI Solid
  G4Box *csiBox =
      new G4Box("CsI", crystalSize / 2, crystalSize / 2, crystalSize / 2);

  // CsI 逻辑体
  G4LogicalVolume *csiLV = new G4LogicalVolume(csiBox, fCsI, "CsI");

  // --- Optical Surface Properties ---
  // 定义晶体表面的光学性质（例如：抛光表面，或者包裹了反射层）
  // G4OpticalSurface *opSurface = new G4OpticalSurface("CsISurface");
  // opSurface->SetType(dielectric_dielectric); // 介质-介质界面
  // opSurface->SetFinish(
  //     ground); // 表面处理：ground (磨砂/粗糙) 或 polished (抛光)
  // opSurface->SetModel(unified);  // 统一模型 (Unified Model)
  // opSurface->SetSigmaAlpha(0.1); // 表面粗糙度 (弧度)，仅对 ground 有效

  // 定义表面材质属性 (例如反射率)
  // G4MaterialPropertiesTable *mptSurface = new G4MaterialPropertiesTable();
  // 假设全反射或者部分反射
  // 如果是抛光表面在空气中，通常不需要额外设置 REFLECTIVITY，因为 Fresnel
  // 已经处理了 但如果是包裹了特氟龙(Teflon)等反射层，可以设置 dielectric_metal
  // 或者 dielectric_dielectric + REFLECTIVITY

  // 示例：设置反射率 (Reflectivity) 和 效率 (Efficiency)
  // G4double reflectivity[nEntries] = {0.0, 0.0}; // 设为0以允许光子离开
  // G4double efficiency[nEntries] = {0.0, 0.0}; //
  // 探测效率（如果是光电倍增管阴极则不为0）
  // mptSurface->AddProperty("REFLECTIVITY", photonEnergy, reflectivity,
  // nEntries);

  // opSurface->SetMaterialPropertiesTable(mptSurface);

  // 将光学表面应用到 CsI 逻辑体表面 (Skin Surface)
  // 这意味着晶体的所有面都具有这种性质
  // new G4LogicalSkinSurface("CsISkinSurface", csiLV, opSurface);
  // ----------------------------------

  // 阵列中心对齐到世界中心
  G4double startX = -totalX / 2 + crystalSize / 2;
  G4double startY = -totalY / 2 + crystalSize / 2;
  G4double startZ = -totalZ / 2 + crystalSize / 2;

  // 三重循环放置晶体
  for (G4int ix = 0; ix < nx; ix++) {
    for (G4int iy = 0; iy < ny; iy++) {
      for (G4int iz = 0; iz < nz; iz++) {
        G4double posX = startX + ix * (crystalSize + gap);
        G4double posY = startY + iy * (crystalSize + gap);
        G4double posZ = startZ + iz * (crystalSize + gap);

        // 修改 ID 生成规则：XXYYZZ 格式
        // 例如: 30502 代表 ix=3, iy=5, iz=2
        G4int copyNo = ix * 10000 + iy * 100 + iz;

        new G4PVPlacement(0, G4ThreeVector(posX, posY, posZ), csiLV, "CsI",
                          gapLV, false,
                          copyNo // 使用新的编码编号
        );
      }
    }
  }

  // 阵列逻辑体csiLV 已创建
  // 挂载敏感探测器移至 ConstructSDandField()

  // =========================
  // 3. 设置可视化属性
  // =========================
  SetVisualizationAttributes(worldLV, gapLV, csiLV);

  return worldPV;
}

void DetectorConstruction::SetVisualizationAttributes(G4LogicalVolume *worldLV,
                                                      G4LogicalVolume *gapLV,
                                                      G4LogicalVolume *csiLV) {
  // 世界体积：蓝色半透明
  G4VisAttributes *visWorld =
      new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 0.1)); // 蓝色, 透明度0.1
  visWorld->SetForceSolid(true);                         // 实体填充但半透明
  worldLV->SetVisAttributes(visWorld);

  // 间隙体积：根据材料设置颜色
  G4VisAttributes *visGap;
  if (fGapMaterial == "OpticalGrease") {
    visGap =
        new G4VisAttributes(G4Colour(1.0, 0.5, 0.0, 0.3)); // 橙色, 透明度0.3
  } else {
    visGap =
        new G4VisAttributes(G4Colour(0.5, 0.5, 0.5, 0.2)); // 灰色, 透明度0.2
  }
  visGap->SetForceSolid(true);
  gapLV->SetVisAttributes(visGap);

  // CsI 晶体：白色半透明
  G4VisAttributes *visCsI =
      new G4VisAttributes(G4Colour(1.0, 1.0, 1.0, 0.5)); // 白色, 透明度0.5
  visCsI->SetForceSolid(true);
  csiLV->SetVisAttributes(visCsI);
}

void DetectorConstruction::ConstructSDandField() {
  G4SDManager *sdManager = G4SDManager::GetSDMpointer();

  // 检查是否已经存在，避免重复添加
  G4String sdName = "CsISD";
  if (!sdManager->FindSensitiveDetector(sdName, false)) {
    DetectorSD *detectorSD = new DetectorSD(sdName, "CsIHitsCollection");
    sdManager->AddNewDetector(detectorSD);

    // 关键：通过逻辑体名称来设置 SD，而不是指针
    SetSensitiveDetector("CsI", detectorSD);
  }
}

void DetectorConstruction::DefineMaterials() {
  // 材料管理器
  G4NistManager *nist = G4NistManager::Instance();

  // 空气材料
  fAir = nist->FindOrBuildMaterial("G4_AIR");

  // 空气的光学属性
  fMptAir = new G4MaterialPropertiesTable();
  const G4int nEntriesAir = 2;
  G4double photonEnergyAir[nEntriesAir] = {2.0 * eV, 4.0 * eV};
  G4double rIndexAir[nEntriesAir] = {1.0, 1.0};
  fMptAir->AddProperty("RINDEX", photonEnergyAir, rIndexAir, nEntriesAir);
  fAir->SetMaterialPropertiesTable(fMptAir);

  // 光学硅脂材料
  fOpticalGrease = new G4Material("OpticalGrease", 1.05 * g / cm3, 2);
  G4Element *elSi = nist->FindOrBuildElement("Si");
  G4Element *elO = nist->FindOrBuildElement("O");
  fOpticalGrease->AddElement(elSi, 1);
  fOpticalGrease->AddElement(elO, 2);

  // 光学硅脂的光学属性
  fMptGrease = new G4MaterialPropertiesTable();
  const G4int nEntriesGrease = 2;
  G4double photonEnergyGrease[nEntriesGrease] = {2.0 * eV, 4.0 * eV};
  G4double rIndexGrease[nEntriesGrease] = {1.5, 1.5};
  fMptGrease->AddProperty("RINDEX", photonEnergyGrease, rIndexGrease,
                          nEntriesGrease);
  fOpticalGrease->SetMaterialPropertiesTable(fMptGrease);

  // CsI 材料
  fCsI = nist->FindOrBuildMaterial("G4_CESIUM_IODIDE");

  // CsI 的光学属性
  fMptCsI = new G4MaterialPropertiesTable();

  const G4int nEntries = 2;
  G4double photonEnergy[nEntries] = {2.0 * eV, 4.0 * eV};
  G4double rIndex[nEntries] = {1.79, 1.79};
  fMptCsI->AddProperty("RINDEX", photonEnergy, rIndex, nEntries);

  fMptCsI->AddConstProperty("SCINTILLATIONYIELD", 54000. / MeV);
  fMptCsI->AddConstProperty("RESOLUTIONSCALE", 1.0);
  fMptCsI->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 1000. * ns);

  G4double component[nEntries] = {1.0, 1.0};
  fMptCsI->AddProperty("SCINTILLATIONCOMPONENT1", photonEnergy, component,
                       nEntries);

  fMptCsI->AddConstProperty("FASTTIMECONSTANT", 1000. * ns);
  fMptCsI->AddConstProperty("SLOWTIMECONSTANT", 1000. * ns);
  fMptCsI->AddConstProperty("YIELDRATIO", 1.0);

  G4double fastComponent[nEntries] = {1.0, 1.0};
  fMptCsI->AddProperty("FASTCOMPONENT", photonEnergy, fastComponent, nEntries);

  G4double absLength[nEntries] = {100. * cm, 100. * cm};
  fMptCsI->AddProperty("ABSLENGTH", photonEnergy, absLength, nEntries);

  fCsI->SetMaterialPropertiesTable(fMptCsI);
}
