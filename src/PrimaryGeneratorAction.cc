// PrimaryGeneratorAction.cc
#include "PrimaryGeneratorAction.hh"
#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"
#include "G4RandomDirection.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include <G4Event.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleTable.hh>

#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction()
    : G4VUserPrimaryGeneratorAction(), fParticleGun(nullptr),
      fMaxEnergy(4 * MeV), fElectron(nullptr), fPositron(nullptr), fNx(8),
      fNy(8), fNz(5), fCrystalSize(6 * cm), fGap(0.5 * cm) {

  fParticleGun = new G4ParticleGun(1);

  // Cache particle definitions
  G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
  fElectron = particleTable->FindParticle("e-");
  fPositron = particleTable->FindParticle("e+");

  // Define commands
  fMessenger = new G4GenericMessenger(this, "/CsI/generator/",
                                      "Primary generator control");
  fMessenger->DeclarePropertyWithUnit("maxEnergy", "MeV", fMaxEnergy,
                                      "Maximum energy for electrons");

  // 计算阵列总尺寸和起点位置
  fTotalX = fNx * fCrystalSize + (fNx - 1) * fGap;
  fTotalY = fNy * fCrystalSize + (fNy - 1) * fGap;
  fTotalZ = fNz * fCrystalSize + (fNz - 1) * fGap;

  fStartX = -fTotalX / 2 + fCrystalSize / 2;
  fStartY = -fTotalY / 2 + fCrystalSize / 2;
  fStartZ = -fTotalZ / 2 + fCrystalSize / 2;
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
  delete fParticleGun;
  delete fMessenger;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event *event) {
  // static G4int pairCount = 0;

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
  G4double energyMeV = G4UniformRand() * fMaxEnergy;

  // 生成一对电子对，方向共线反向（假设沿随机单位矢量n和-n）
  G4ThreeVector dir1 = G4RandomDirection();
  G4ThreeVector dir2 = -dir1;

  // --- 产生电子 ---
  fParticleGun->SetParticleDefinition(fElectron);
  fParticleGun->SetParticleEnergy(energyMeV);
  fParticleGun->SetParticlePosition(vertexPos);
  fParticleGun->SetParticleMomentumDirection(dir1);
  fParticleGun->GeneratePrimaryVertex(event);

  // --- 产生正电子 ---
  fParticleGun->SetParticleDefinition(fPositron);
  fParticleGun->SetParticleEnergy(energyMeV);
  fParticleGun->SetParticlePosition(vertexPos);
  fParticleGun->SetParticleMomentumDirection(dir2);
  fParticleGun->GeneratePrimaryVertex(event);
}
