#include "SteppingAction.hh"
#include "G4OpticalPhoton.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4Track.hh"
#include "G4ios.hh"

SteppingAction::SteppingAction() {}

SteppingAction::~SteppingAction() {}

void SteppingAction::UserSteppingAction(const G4Step *step) {
  G4Track *track = step->GetTrack();
  if (track->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition())
    return;

  G4StepPoint *prePoint = step->GetPreStepPoint();
  G4StepPoint *postPoint = step->GetPostStepPoint();
  // Only consider steps that cross a geometry boundary
  if (postPoint->GetStepStatus() != fGeomBoundary)
    return;

  // Determine pre/post volumes
  G4VPhysicalVolume *preVol = prePoint->GetPhysicalVolume();
  G4VPhysicalVolume *postVol = postPoint->GetPhysicalVolume();
  G4String preName = preVol ? preVol->GetName() : "";
  G4String postName = postVol ? postVol->GetName() : "";

  // If photon goes from CsI to World, count it for that crystal
  if (preName == "CsI" && postName == "World") {
    // G4cout << "Photon exited CsI crystal\n";
    G4int crystalID = prePoint->GetTouchable()->GetCopyNumber();
    fPhotonExitCounts[crystalID]++;
  }
}

void SteppingAction::ResetCounts() { fPhotonExitCounts.clear(); }