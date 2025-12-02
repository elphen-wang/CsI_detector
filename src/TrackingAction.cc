// TrackingAction.cc
#include "TrackingAction.hh"
#include "G4EventManager.hh"
#include "G4OpticalPhoton.hh"
#include "G4Track.hh"
#include "G4TrackingManager.hh"
#include "Trajectory.hh"

TrackingAction::TrackingAction() = default;
TrackingAction::~TrackingAction() = default;

void TrackingAction::PreUserTrackingAction(const G4Track *track) {
  // 避免为光子创建轨迹，如果数量太多
  // if (track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition())
  // return;

  // if (track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
  //   G4cout << "Optical photon created\n";
  // }

  G4TrackingManager *tm =
      G4EventManager::GetEventManager()->GetTrackingManager();
  Trajectory *trajectory = new Trajectory(track);
  tm->SetTrajectory(trajectory);
}
