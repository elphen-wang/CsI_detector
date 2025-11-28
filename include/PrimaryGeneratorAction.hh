#ifndef PRIMARY_GENERATOR_ACTION_HH
#define PRIMARY_GENERATOR_ACTION_HH

#include <G4VUserPrimaryGeneratorAction.hh>
#include <G4ParticleGun.hh>
#include "G4GeneralParticleSource.hh"
#include <vector>

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
public:
    PrimaryGeneratorAction();
    virtual ~PrimaryGeneratorAction();

    virtual void GeneratePrimaries(G4Event* event) override;

private:
    G4ParticleGun* fParticleGun;

    // CsI晶体阵列参数，再次声明或传入
    G4int fNx, fNy, fNz;
    G4double fCrystalSize;
    G4double fGap;
    G4double fTotalX, fTotalY, fTotalZ;
    G4double fStartX, fStartY, fStartZ;

    void InitializeArrayGeometry();
};

#endif


