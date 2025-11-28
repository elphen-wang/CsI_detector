// DetectorSD.cc

#include "DetectorSD.hh"
#include "G4Step.hh"
#include "G4SDManager.hh"

DetectorSD::DetectorSD(const G4String& name)
    : G4VSensitiveDetector(name), fEdep(0.) {
}

DetectorSD::~DetectorSD() {}

G4bool DetectorSD::ProcessHits(G4Step* step, G4TouchableHistory*) {
    G4double edep = step->GetTotalEnergyDeposit();
    if (edep > 0.) {
        fEdep += edep;
    }
    return true;
}
