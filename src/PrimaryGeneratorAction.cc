// PrimaryGeneratorAction.cc
#include "PrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "Randomize.hh"
#include <G4Event.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleTable.hh>
#include <G4SystemOfUnits.hh>

PrimaryGeneratorAction::PrimaryGeneratorAction()
    : G4VUserPrimaryGeneratorAction(), fParticleGun(nullptr), fNx(8), fNy(8),
      fNz(5), fCrystalSize(6 * cm), fGap(0.5 * cm) {

  fParticleGun = new G4ParticleGun(1);

  // 计算阵列总尺寸和起点位置
  fTotalX = fNx * fCrystalSize + (fNx - 1) * fGap;
  fTotalY = fNy * fCrystalSize + (fNy - 1) * fGap;
  fTotalZ = fNz * fCrystalSize + (fNz - 1) * fGap;

  fStartX = -fTotalX / 2 + fCrystalSize / 2;
  fStartY = -fTotalY / 2 + fCrystalSize / 2;
  fStartZ = -fTotalZ / 2 + fCrystalSize / 2;
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() { delete fParticleGun; }

void PrimaryGeneratorAction::GeneratePrimaries(G4Event *event) {
  // static G4int pairCount = 0;

  G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
  G4ParticleDefinition *eMinus = particleTable->FindParticle("e-");
  G4ParticleDefinition *ePlus = particleTable->FindParticle("e+");

  // 随机选择一个晶体
  G4int ix = G4UniformRand() * fNx;
  if (ix >= fNx)
    ix = fNx - 1;
  G4int iy = G4UniformRand() * fNy;
  if (iy >= fNy)
    iy = fNy - 1;
  G4int iz = G4UniformRand() * fNz;
  if (iz >= fNz)
    iz = fNz - 1;

  // 晶体中心坐标
  G4double centerX = fStartX + ix * (fCrystalSize + fGap);
  G4double centerY = fStartY + iy * (fCrystalSize + fGap);
  G4double centerZ = fStartZ + iz * (fCrystalSize + fGap);

  // 晶体内均匀撒点，坐标偏移[-crystalSize/2, crystalSize/2]
  G4double localX = (G4UniformRand() - 0.5) * fCrystalSize;
  G4double localY = (G4UniformRand() - 0.5) * fCrystalSize;
  G4double localZ = (G4UniformRand() - 0.5) * fCrystalSize;

  G4ThreeVector vertexPos(centerX + localX, centerY + localY, centerZ + localZ);

  // 动能在0到8 MeV之间均匀分布
  G4double energyMeV = G4UniformRand() * 4 * MeV;

  // 生成一对电子对，方向共线反向（假设沿随机单位矢量n和-n）
  G4double theta = acos(1 - 2 * G4UniformRand()); // 0~pi 均匀分布
  G4double phi = 2 * M_PI * G4UniformRand();

  G4double dx = sin(theta) * cos(phi);
  G4double dy = sin(theta) * sin(phi);
  G4double dz = cos(theta);

  G4ThreeVector dir1(dx, dy, dz);
  G4ThreeVector dir2 = -dir1;

  // --- 产生电子 ---
  fParticleGun->SetParticleDefinition(eMinus);
  fParticleGun->SetParticleEnergy(energyMeV);
  fParticleGun->SetParticlePosition(vertexPos);
  fParticleGun->SetParticleMomentumDirection(dir1);
  fParticleGun->GeneratePrimaryVertex(event);

  // --- 产生正电子 ---
  fParticleGun->SetParticleDefinition(ePlus);
  fParticleGun->SetParticleEnergy(energyMeV);
  fParticleGun->SetParticlePosition(vertexPos);
  fParticleGun->SetParticleMomentumDirection(dir2);
  fParticleGun->GeneratePrimaryVertex(event);
}
