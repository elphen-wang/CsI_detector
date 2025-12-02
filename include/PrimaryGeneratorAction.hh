#ifndef PRIMARY_GENERATOR_ACTION_HH
#define PRIMARY_GENERATOR_ACTION_HH

#include "G4GeneralParticleSource.hh"
#include "G4GenericMessenger.hh"
#include <G4ParticleGun.hh>
#include <G4VUserPrimaryGeneratorAction.hh>
#include <vector>

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
public:
  PrimaryGeneratorAction();
  virtual ~PrimaryGeneratorAction();

  virtual void GeneratePrimaries(G4Event *event) override;

private:
  G4ParticleGun *fParticleGun;
  G4GenericMessenger *fMessenger;
  G4GenericMessenger *fRandMessenger;

  // Configurable parameters
  G4double fMaxEnergy;
  G4String fMode;           // "ePair", "ePairOpposite", "ePairDeflected"
  G4double fDeflectAngle;   // in degrees for deflected two-particle mode
  G4double fParticleEnergy; // energy for generated particles (MeV)
  // Random seed control
  G4bool fAutoSeed;
  G4long fSeed;

  // Cached particle definitions
  G4ParticleDefinition *fElectron;
  G4ParticleDefinition *fPositron;

  // CsI晶体阵列参数，再次声明或传入
  G4int fNx, fNy, fNz;
  G4double fCrystalSize;
  G4double fGap;
  G4double fTotalX, fTotalY, fTotalZ;
  G4double fStartX, fStartY, fStartZ;

  void InitializeArrayGeometry();
  // Random seed control: apply seeds at runtime
  void ApplyRandomSeed();
};

#endif
