#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "G4Types.hh"
#include "G4UserSteppingAction.hh"
#include <iostream>
#include <map>
class SteppingAction : public G4UserSteppingAction {
public:
  SteppingAction();
  virtual ~SteppingAction();

  virtual void UserSteppingAction(const G4Step *step) override;

  const std::map<G4int, G4int> &GetPhotonExitCounts() const {
    std::cout << "Accessing PhotonExitCounts with size: "
              << fPhotonExitCounts.size() << std::endl;
    return fPhotonExitCounts;
  }
  void ResetCounts();

private:
  std::map<G4int, G4int> fPhotonExitCounts; // key: crystalID, value: photon
                                            // count exiting that crystal
};

#endif