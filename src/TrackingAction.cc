// TrackingAction.cc
#include "TrackingAction.hh"
#include "Trajectory.hh"
#include "G4TrackingManager.hh"
#include "G4EventManager.hh"
#include "G4Track.hh"

TrackingAction::TrackingAction() = default;
TrackingAction::~TrackingAction() = default;

void TrackingAction::PreUserTrackingAction(const G4Track* track) {
    G4TrackingManager* tm = G4EventManager::GetEventManager()->GetTrackingManager();
    Trajectory* trajectory = new Trajectory(track);
    tm->SetTrajectory(trajectory);
}
