// PhysicsList.hh
#ifndef PHYSICS_LIST_HH
#define PHYSICS_LIST_HH

#include "G4GenericMessenger.hh"
#include "G4VModularPhysicsList.hh"

class PhysicsList : public G4VModularPhysicsList {
public:
  PhysicsList();
  virtual ~PhysicsList();

  void SetOpticalPhysics(G4bool on);

private:
  G4GenericMessenger *fMessenger;
};

#endif
