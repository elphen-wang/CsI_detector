// DetectorSD.hh

#include "G4Allocator.hh"
#include "G4Step.hh"
#include "G4THitsCollection.hh"
#include "G4ThreeVector.hh"
#include "G4VHit.hh"
#include "G4VSensitiveDetector.hh"

class CsIHit : public G4VHit {
public:
  CsIHit();
  virtual ~CsIHit();
  CsIHit(const CsIHit &right);
  const CsIHit &operator=(const CsIHit &right);
  int operator==(const CsIHit &right) const;

  inline void *operator new(size_t);
  inline void operator delete(void *);

  void Draw() override;
  void Print() override;

  void SetTrackID(G4int tid) { fTrackID = tid; }
  void SetChamberNb(G4int chamb) { fChamberNb = chamb; }
  void SetEdep(G4double de) { fEdep = de; }
  void AddEdep(G4double de) { fEdep += de; }
  void SetPos(const G4ThreeVector &xyz) { fPos = xyz; }
  void SetTime(G4double t) { fTime = t; }
  void SetPDG(G4int pdg) { fPDG = pdg; }
  void SetParentID(G4int id) { fParentID = id; }
  void SetMomentumDirection(const G4ThreeVector &dir) {
    fMomentumDirection = dir;
  }
  void SetKineticEnergy(G4double e) { fKineticEnergy = e; }
  void SetCreatorProcess(const G4String &proc) { fCreatorProcess = proc; }
  void SetTrackLength(G4double len) { fTrackLength = len; }
  void AddTrackLength(G4double len) { fTrackLength += len; }

  G4int GetTrackID() const { return fTrackID; }
  G4int GetChamberNb() const { return fChamberNb; }
  G4double GetEdep() const { return fEdep; }
  G4ThreeVector GetPos() const { return fPos; }
  G4double GetTime() const { return fTime; }
  G4int GetPDG() const { return fPDG; }
  G4int GetParentID() const { return fParentID; }
  G4ThreeVector GetMomentumDirection() const { return fMomentumDirection; }
  G4double GetKineticEnergy() const { return fKineticEnergy; }
  G4String GetCreatorProcess() const { return fCreatorProcess; }
  G4double GetTrackLength() const { return fTrackLength; }

private:
  G4int fTrackID;
  G4int fChamberNb; // Copy Number
  G4double fEdep;
  G4ThreeVector fPos;
  G4double fTime;
  G4int fPDG;
  G4int fParentID;
  G4ThreeVector fMomentumDirection;
  G4double fKineticEnergy;
  G4String fCreatorProcess;
  G4double fTrackLength;
};

typedef G4THitsCollection<CsIHit> CsIHitsCollection;

extern G4ThreadLocal G4Allocator<CsIHit> *CsIHitAllocator;

inline void *CsIHit::operator new(size_t) {
  if (!CsIHitAllocator)
    CsIHitAllocator = new G4Allocator<CsIHit>;
  return (void *)CsIHitAllocator->MallocSingle();
}

inline void CsIHit::operator delete(void *hit) {
  CsIHitAllocator->FreeSingle((CsIHit *)hit);
}

class DetectorSD : public G4VSensitiveDetector {
public:
  DetectorSD(const G4String &name, const G4String &hitsCollectionName);
  virtual ~DetectorSD();

  virtual void Initialize(G4HCofThisEvent *hitCollection) override;
  virtual G4bool ProcessHits(G4Step *step,
                             G4TouchableHistory *history) override;
  virtual void EndOfEvent(G4HCofThisEvent *hitCollection) override;

private:
  CsIHitsCollection *fHitsCollection;
};
