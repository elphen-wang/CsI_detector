// DetectorSD.hh

#include "G4VSensitiveDetector.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"

class DetectorSD : public G4VSensitiveDetector {
public:
    DetectorSD(const G4String& name);
    virtual ~DetectorSD();

    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;

    // 你还可以增加接口获得累积能量、事件总结等
private:
    G4double fEdep;
};
