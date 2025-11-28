#ifndef Trajectory_h
#define Trajectory_h 1

#include "G4VTrajectory.hh"
#include "G4TrajectoryPoint.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4Colour.hh"
#include <vector>

class Trajectory : public G4VTrajectory {
public:
    Trajectory(const G4Track* aTrack);
    virtual ~Trajectory();

    // 必须实现的纯虚函数
    virtual G4int GetTrackID() const override { return fTrackID; }
    virtual G4int GetParentID() const override { return fParentID; }
    virtual G4String GetParticleName() const override { return fParticleName; }
    virtual G4double GetCharge() const override { return fCharge; }
    virtual G4int GetPDGEncoding() const override { return fPDGEncoding; }
    virtual G4ThreeVector GetInitialMomentum() const override { return fInitialMomentum; }
    
    virtual int GetPointEntries() const override { return fPositionRecord.size(); }
    virtual G4VTrajectoryPoint* GetPoint(G4int i) const override { 
        return (i >= 0 && i < GetPointEntries()) ? fPositionRecord[i] : nullptr; 
    }
    
    virtual void AppendStep(const G4Step* aStep) override;
    virtual void MergeTrajectory(G4VTrajectory* secondTrajectory) override;
    
    virtual void DrawTrajectory() const override;
    virtual void ShowTrajectory(std::ostream& os = G4cout) const override;
    
    G4Colour GetColor() const;

private:
    G4int fTrackID;
    G4int fParentID;
    G4double fCharge;
    G4int fPDGEncoding;
    G4String fParticleName;
    G4ThreeVector fInitialMomentum;
    G4ThreeVector fInitialPosition;
    G4double fInitialKineticEnergy;
    std::vector<G4VTrajectoryPoint*> fPositionRecord;
};

#endif