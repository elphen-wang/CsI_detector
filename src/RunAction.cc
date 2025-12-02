#include "RunAction.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include <fstream>

// #include "G4AnalysisManager.hh" // Not needed if included in header or using
// g4root.hh

RunAction::RunAction() : G4UserRunAction() {
  // Create analysis manager
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->SetVerboseLevel(1);
  analysisManager->SetNtupleMerging(true);

  // Creating ntuple
  analysisManager->CreateNtuple("CsI", "CsI Hits");
  analysisManager->CreateNtupleIColumn("EventID");
  analysisManager->CreateNtupleDColumn("TotalEdep");
  analysisManager->CreateNtupleIColumn("HitCount");
  // 使用 vector 存储每个 hit 的信息
  analysisManager->CreateNtupleIColumn("CrystalID", fCrystalIDs);
  analysisManager->CreateNtupleDColumn("CrystalEdep", fCrystalEdeps);
  analysisManager->CreateNtupleDColumn("CrystalTime", fCrystalTimes);
  analysisManager->CreateNtupleDColumn("CrystalPosX", fCrystalPosX);
  analysisManager->CreateNtupleDColumn("CrystalPosY", fCrystalPosY);
  analysisManager->CreateNtupleDColumn("CrystalPosZ", fCrystalPosZ);
  analysisManager->CreateNtupleIColumn("CrystalPDG", fCrystalPDGs);
  analysisManager->CreateNtupleIColumn("CrystalTrackID", fCrystalTrackIDs);
  analysisManager->CreateNtupleIColumn("CrystalParentID", fCrystalParentIDs);
  analysisManager->CreateNtupleDColumn("CrystalDirX", fCrystalDirX);
  analysisManager->CreateNtupleDColumn("CrystalDirY", fCrystalDirY);
  analysisManager->CreateNtupleDColumn("CrystalDirZ", fCrystalDirZ);
  analysisManager->CreateNtupleDColumn("CrystalKineticEnergy",
                                       fCrystalKineticEnergy);
  analysisManager->CreateNtupleIColumn("CrystalProcessID", fCrystalProcessIDs);
  analysisManager->CreateNtupleDColumn("CrystalTrackLength",
                                       fCrystalTrackLength);

  // Primary Particle Columns
  analysisManager->CreateNtupleIColumn("PrimaryPDG", fPrimaryPDG);
  analysisManager->CreateNtupleDColumn("PrimaryEnergy", fPrimaryEnergy);
  analysisManager->CreateNtupleDColumn("PrimaryPosX", fPrimaryPosX);
  analysisManager->CreateNtupleDColumn("PrimaryPosY", fPrimaryPosY);
  analysisManager->CreateNtupleDColumn("PrimaryPosZ", fPrimaryPosZ);
  analysisManager->CreateNtupleDColumn("PrimaryDirX", fPrimaryDirX);
  analysisManager->CreateNtupleDColumn("PrimaryDirY", fPrimaryDirY);
  analysisManager->CreateNtupleDColumn("PrimaryDirZ", fPrimaryDirZ);

  // Photon Exit Columns
  analysisManager->CreateNtupleIColumn("PhotonExitCrystalID",
                                       fPhotonExitCrystalIDs);
  analysisManager->CreateNtupleIColumn("PhotonExitCount", fPhotonExitCounts);

  analysisManager->FinishNtuple();
}
RunAction::~RunAction() { delete G4AnalysisManager::Instance(); }

int RunAction::GetProcessID(const G4String &processName) {
  if (fProcessMap.find(processName) == fProcessMap.end()) {
    int id = fProcessMap.size();
    fProcessMap[processName] = id;
  }
  return fProcessMap[processName];
}

void RunAction::BeginOfRunAction(const G4Run *) {
  auto analysisManager = G4AnalysisManager::Instance();
  G4String fileName = "CsI_Axion";
  analysisManager->OpenFile(fileName);
}

void RunAction::EndOfRunAction(const G4Run *) {
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->Write();
  analysisManager->CloseFile();

  // Save Process Mapping to file
  if (IsMaster()) {
    std::ofstream outFile("ProcessIDMap.txt");
    outFile << "ID\tProcessName" << G4endl;
    for (const auto &pair : fProcessMap) {
      outFile << pair.second << "\t" << pair.first << G4endl;
    }
    outFile.close();
    G4cout << "Process ID mapping saved to 'ProcessIDMap.txt'" << G4endl;
  }
}
