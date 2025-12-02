// DetectorSD.cc

#include "DetectorSD.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4VProcess.hh"
#include "G4ios.hh"

G4ThreadLocal G4Allocator<CsIHit> *CsIHitAllocator = 0;

CsIHit::CsIHit()
    : G4VHit(), fTrackID(-1), fChamberNb(-1), fEdep(0.), fPos(G4ThreeVector()),
      fTime(0.), fPDG(0), fParentID(-1), fMomentumDirection(G4ThreeVector()),
      fKineticEnergy(0.), fCreatorProcess(""), fTrackLength(0.) {}
CsIHit::~CsIHit() {}
CsIHit::CsIHit(const CsIHit &right) : G4VHit() {
  fTrackID = right.fTrackID;
  fChamberNb = right.fChamberNb;
  fEdep = right.fEdep;
  fPos = right.fPos;
  fTime = right.fTime;
  fPDG = right.fPDG;
  fParentID = right.fParentID;
  fMomentumDirection = right.fMomentumDirection;
  fKineticEnergy = right.fKineticEnergy;
  fCreatorProcess = right.fCreatorProcess;
  fTrackLength = right.fTrackLength;
}
const CsIHit &CsIHit::operator=(const CsIHit &right) {
  fTrackID = right.fTrackID;
  fChamberNb = right.fChamberNb;
  fEdep = right.fEdep;
  fPos = right.fPos;
  fTime = right.fTime;
  fPDG = right.fPDG;
  fParentID = right.fParentID;
  fMomentumDirection = right.fMomentumDirection;
  fKineticEnergy = right.fKineticEnergy;
  fCreatorProcess = right.fCreatorProcess;
  fTrackLength = right.fTrackLength;
  return *this;
}
int CsIHit::operator==(const CsIHit &right) const {
  return (this == &right) ? 1 : 0;
}
void CsIHit::Draw() {}
void CsIHit::Print() {}

DetectorSD::DetectorSD(const G4String &name, const G4String &hitsCollectionName)
    : G4VSensitiveDetector(name), fHitsCollection(nullptr) {
  collectionName.insert(hitsCollectionName);
}

DetectorSD::~DetectorSD() {}

void DetectorSD::Initialize(G4HCofThisEvent *hce) {
  // Create hits collection
  fHitsCollection =
      new CsIHitsCollection(SensitiveDetectorName, collectionName[0]);

  // Add this collection in hce
  G4int hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  hce->AddHitsCollection(hcID, fHitsCollection);
}

G4bool DetectorSD::ProcessHits(G4Step *step, G4TouchableHistory *) {
  G4double edep = step->GetTotalEnergyDeposit();
  if (edep == 0.)
    return false;

  G4StepPoint *preStepPoint = step->GetPreStepPoint();
  G4TouchableHistory *touchable =
      (G4TouchableHistory *)(preStepPoint->GetTouchable());
  G4int copyNo = touchable->GetReplicaNumber(0);

  // Check if this crystal already has a hit
  G4int nHits = fHitsCollection->entries();
  CsIHit *hit = nullptr;
  for (G4int i = 0; i < nHits; i++) {
    if ((*fHitsCollection)[i]->GetChamberNb() == copyNo) {
      hit = (*fHitsCollection)[i];
      break;
    }
  }

  if (hit) {
    // Add energy to existing hit
    hit->AddEdep(edep);
    // Add step length to track length
    hit->AddTrackLength(step->GetStepLength());
    // Keep the earliest time
    if (preStepPoint->GetGlobalTime() < hit->GetTime()) {
      hit->SetTime(preStepPoint->GetGlobalTime());
    }
  } else {
    // Create new hit
    hit = new CsIHit();
    hit->SetChamberNb(copyNo);
    hit->SetEdep(edep);
    hit->SetPos(preStepPoint->GetPosition());
    hit->SetTrackID(step->GetTrack()->GetTrackID());
    hit->SetTime(preStepPoint->GetGlobalTime());
    hit->SetPDG(step->GetTrack()->GetDefinition()->GetPDGEncoding());
    hit->SetParentID(step->GetTrack()->GetParentID());
    hit->SetMomentumDirection(preStepPoint->GetMomentumDirection());
    hit->SetKineticEnergy(preStepPoint->GetKineticEnergy());
    hit->SetTrackLength(step->GetStepLength());
    const G4VProcess *creatorProcess = step->GetTrack()->GetCreatorProcess();
    if (creatorProcess) {
      hit->SetCreatorProcess(creatorProcess->GetProcessName());
    } else {
      hit->SetCreatorProcess("Primary");
    }
    fHitsCollection->insert(hit);
  }
  return true;
}

void DetectorSD::EndOfEvent(G4HCofThisEvent *) {
  if (verboseLevel > 1) {
    G4int nofHits = fHitsCollection->entries();
    G4cout << "\n-------->Hits Collection: in this event they are " << nofHits
           << " hits in the tracker chambers: " << "\n";
  }
}
