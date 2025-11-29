#include "EventAction.hh"
#include "DetectorSD.hh"
#include "RunAction.hh"

#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "g4root.hh"

EventAction::EventAction() : G4UserEventAction(), fHCID(-1) {}

EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event *) {}

void EventAction::EndOfEventAction(const G4Event *event) {
  // Get hits collection ID (only once)
  if (fHCID == -1) {
    fHCID = G4SDManager::GetSDMpointer()->GetCollectionID("CsIHitsCollection");
  }

  // Get hits collection
  auto hce = event->GetHCofThisEvent();
  if (!hce)
    return;

  auto hitsCollection = static_cast<CsIHitsCollection *>(hce->GetHC(fHCID));
  if (!hitsCollection)
    return;

  // Get analysis manager
  auto analysisManager = G4AnalysisManager::Instance();

  // Get RunAction to access vectors
  auto runAction = static_cast<const RunAction *>(
      G4RunManager::GetRunManager()->GetUserRunAction());
  // 注意：在多线程模式下，UserRunAction 是 const 的，我们需要 const_cast
  // 或者更好的设计。 但 G4AnalysisManager 处理了数据填充，我们只需要填充
  // vector。 实际上，G4AnalysisManager 的 vector column 需要绑定到成员变量。
  // 由于 RunAction 是在 Master 和 Worker 线程中分别实例化的，
  // 我们需要确保我们操作的是当前线程 RunAction 的 vector。

  // 修正：更好的做法是在 EventAction 中持有 vector，并在 RunAction 中注册这些
  // vector 的地址。 但是 RunAction 的构造函数是在 EventAction 之前调用的。
  // 让我们使用更简单的方法：直接在 EventAction 中填充 AnalysisManager，不使用
  // vector column 绑定， 而是使用 FillNtupleDColumn 等，但这不支持 vector。
  // 如果要支持 vector，必须绑定引用。

  // 让我们回退一步：为了简单起见，我们先修改 RunAction，让它暴露非 const 的
  // vector 引用， 并在 EventAction 中获取它。

  RunAction *nonConstRunAction = const_cast<RunAction *>(runAction);
  auto &crystalIDs = nonConstRunAction->GetCrystalIDs();
  auto &crystalEdeps = nonConstRunAction->GetCrystalEdeps();
  auto &crystalTimes = nonConstRunAction->GetCrystalTimes();
  auto &crystalPosX = nonConstRunAction->GetCrystalPosX();
  auto &crystalPosY = nonConstRunAction->GetCrystalPosY();
  auto &crystalPosZ = nonConstRunAction->GetCrystalPosZ();
  auto &crystalPDGs = nonConstRunAction->GetCrystalPDGs();
  auto &crystalParentIDs = nonConstRunAction->GetCrystalParentIDs();
  auto &crystalDirX = nonConstRunAction->GetCrystalDirX();
  auto &crystalDirY = nonConstRunAction->GetCrystalDirY();
  auto &crystalDirZ = nonConstRunAction->GetCrystalDirZ();
  auto &crystalKineticEnergy = nonConstRunAction->GetCrystalKineticEnergy();
  auto &crystalProcessIDs = nonConstRunAction->GetCrystalProcessIDs();
  auto &crystalTrackLength = nonConstRunAction->GetCrystalTrackLength();

  // Primary Particle Vectors
  auto &primaryPDG = nonConstRunAction->GetPrimaryPDG();
  auto &primaryEnergy = nonConstRunAction->GetPrimaryEnergy();
  auto &primaryPosX = nonConstRunAction->GetPrimaryPosX();
  auto &primaryPosY = nonConstRunAction->GetPrimaryPosY();
  auto &primaryPosZ = nonConstRunAction->GetPrimaryPosZ();
  auto &primaryDirX = nonConstRunAction->GetPrimaryDirX();
  auto &primaryDirY = nonConstRunAction->GetPrimaryDirY();
  auto &primaryDirZ = nonConstRunAction->GetPrimaryDirZ();

  // Clear vectors
  crystalIDs.clear();
  crystalEdeps.clear();
  crystalTimes.clear();
  crystalPosX.clear();
  crystalPosY.clear();
  crystalPosZ.clear();
  crystalPDGs.clear();
  crystalParentIDs.clear();
  crystalDirX.clear();
  crystalDirY.clear();
  crystalDirZ.clear();
  crystalKineticEnergy.clear();
  crystalProcessIDs.clear();
  crystalTrackLength.clear();

  primaryPDG.clear();
  primaryEnergy.clear();
  primaryPosX.clear();
  primaryPosY.clear();
  primaryPosZ.clear();
  primaryDirX.clear();
  primaryDirY.clear();
  primaryDirZ.clear();

  // Fill Primary Particles
  G4int nVertex = event->GetNumberOfPrimaryVertex();
  for (G4int i = 0; i < nVertex; i++) {
    G4PrimaryVertex *vertex = event->GetPrimaryVertex(i);
    G4double x = vertex->GetX0();
    G4double y = vertex->GetY0();
    G4double z = vertex->GetZ0();

    G4int nParticle = vertex->GetNumberOfParticle();
    for (G4int j = 0; j < nParticle; j++) {
      G4PrimaryParticle *particle = vertex->GetPrimary(j);
      primaryPDG.push_back(particle->GetPDGcode());
      primaryEnergy.push_back(particle->GetTotalEnergy());
      primaryPosX.push_back(x);
      primaryPosY.push_back(y);
      primaryPosZ.push_back(z);
      primaryDirX.push_back(particle->GetMomentumDirection().x());
      primaryDirY.push_back(particle->GetMomentumDirection().y());
      primaryDirZ.push_back(particle->GetMomentumDirection().z());
    }
  }

  G4double totalEdep = 0.;
  G4int nHits = hitsCollection->entries();

  for (G4int i = 0; i < nHits; i++) {
    auto hit = (*hitsCollection)[i];
    G4double edep = hit->GetEdep();
    if (edep > 0.) {
      totalEdep += edep;
      crystalIDs.push_back(hit->GetChamberNb());
      crystalEdeps.push_back(edep);
      crystalTimes.push_back(hit->GetTime());
      crystalPosX.push_back(hit->GetPos().x());
      crystalPosY.push_back(hit->GetPos().y());
      crystalPosZ.push_back(hit->GetPos().z());
      crystalPDGs.push_back(hit->GetPDG());
      crystalParentIDs.push_back(hit->GetParentID());
      crystalDirX.push_back(hit->GetMomentumDirection().x());
      crystalDirY.push_back(hit->GetMomentumDirection().y());
      crystalDirZ.push_back(hit->GetMomentumDirection().z());
      crystalKineticEnergy.push_back(hit->GetKineticEnergy());
      crystalProcessIDs.push_back(
          nonConstRunAction->GetProcessID(hit->GetCreatorProcess()));
      crystalTrackLength.push_back(hit->GetTrackLength());
    }
  } // Fill Ntuple
  analysisManager->FillNtupleIColumn(0, event->GetEventID());
  analysisManager->FillNtupleDColumn(1, totalEdep);
  analysisManager->FillNtupleIColumn(2, crystalIDs.size());
  // vector columns are automatically filled because they are bound by reference
  analysisManager->AddNtupleRow();
}
