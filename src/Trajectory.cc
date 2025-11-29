#include "Trajectory.hh"
#include "G4ParticleDefinition.hh"
#include "G4Polyline.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4TrajectoryPoint.hh"
#include "G4UIcommand.hh"
#include "G4UnitsTable.hh"
#include "G4VVisManager.hh"
#include "G4VisAttributes.hh"

// 如果使用内存分配器版本，需要这个定义
G4ThreadLocal G4Allocator<Trajectory> *TrajectoryAllocator = nullptr;

Trajectory::Trajectory(const G4Track *aTrack)
    : G4VTrajectory(), fTrackID(aTrack->GetTrackID()),
      fParentID(aTrack->GetParentID()),
      fCharge(aTrack->GetDynamicParticle()->GetCharge()),
      fPDGEncoding(aTrack->GetDefinition()->GetPDGEncoding()),
      fParticleName(aTrack->GetDefinition()->GetParticleName()),
      fInitialMomentum(aTrack->GetMomentum()),
      fInitialPosition(aTrack->GetPosition()),
      fInitialKineticEnergy(aTrack->GetKineticEnergy()) {
  // 使用 vector 对象而不是指针
  fPositionRecord.push_back(new G4TrajectoryPoint(aTrack->GetPosition()));
}

Trajectory::~Trajectory() {
  // 清理所有轨迹点
  for (auto point : fPositionRecord) {
    delete point;
  }
  fPositionRecord.clear();
}

void Trajectory::AppendStep(const G4Step *aStep) {
  fPositionRecord.push_back(
      new G4TrajectoryPoint(aStep->GetPostStepPoint()->GetPosition()));
}

void Trajectory::MergeTrajectory(G4VTrajectory *secondTrajectory) {
  if (!secondTrajectory)
    return;

  auto second = dynamic_cast<Trajectory *>(secondTrajectory);
  if (!second)
    return;

  // 将第二个轨迹的点合并到当前轨迹
  fPositionRecord.insert(fPositionRecord.end(), second->fPositionRecord.begin(),
                         second->fPositionRecord.end());
  // 清空第二个轨迹的点，避免重复删除
  second->fPositionRecord.clear();
}

void Trajectory::DrawTrajectory() const {
  auto visManager = G4VVisManager::GetConcreteInstance();
  if (!visManager)
    return;

  // 创建多段线来绘制轨迹
  G4Polyline trajectoryLine;
  for (auto point : fPositionRecord) {
    trajectoryLine.push_back(point->GetPosition());
  }

  // 设置轨迹颜色
  G4Colour colour = GetColor();
  G4VisAttributes trajectoryAtts(colour);
  trajectoryLine.SetVisAttributes(&trajectoryAtts);

  // 绘制轨迹
  visManager->Draw(trajectoryLine);
}

void Trajectory::ShowTrajectory(std::ostream &os) const {
  os << "Trajectory: TrackID=" << fTrackID << ", ParentID=" << fParentID
     << ", Particle=" << fParticleName << ", PDGEncoding=" << fPDGEncoding
     << ", Charge=" << fCharge / CLHEP::eplus << " e"
     << ", Points=" << fPositionRecord.size() << G4endl;
}

G4Colour Trajectory::GetColor() const {
  G4String pname = GetParticleName();
  if (pname == "e-") {
    return G4Colour::Blue(); // 电子轨迹蓝色
  } else if (pname == "e+") {
    return G4Colour::Red(); // 正电子轨迹红色
  } else if (pname == "gamma") {
    return G4Colour::Green(); // 光子绿色
  } else if (pname == "proton") {
    return G4Colour::Yellow(); // 质子黄色
  } else if (pname == "alpha") {
    return G4Colour::Cyan(); // α粒子青色
  } else if (pname == "opticalphoton") {
    return G4Colour(0.0, 1.0, 1.0); // 光子青色 (Cyan)
  } else {
    return G4Colour::White(); // 其他粒子白色
  }
}