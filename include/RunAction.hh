#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"
// #include "G4AnalysisManager.hh" // For Geant4 11+
#include "g4root.hh" // For Geant4 10.x
#include "globals.hh"
#include <map>

class RunAction : public G4UserRunAction {
public:
  RunAction();
  virtual ~RunAction();

  virtual void BeginOfRunAction(const G4Run *);
  virtual void EndOfRunAction(const G4Run *);

  std::vector<int> &GetCrystalIDs() { return fCrystalIDs; }
  std::vector<double> &GetCrystalEdeps() { return fCrystalEdeps; }
  std::vector<double> &GetCrystalTimes() { return fCrystalTimes; }
  std::vector<double> &GetCrystalPosX() { return fCrystalPosX; }
  std::vector<double> &GetCrystalPosY() { return fCrystalPosY; }
  std::vector<double> &GetCrystalPosZ() { return fCrystalPosZ; }
  std::vector<int> &GetCrystalPDGs() { return fCrystalPDGs; }
  std::vector<int> &GetCrystalParentIDs() { return fCrystalParentIDs; }
  std::vector<double> &GetCrystalDirX() { return fCrystalDirX; }
  std::vector<double> &GetCrystalDirY() { return fCrystalDirY; }
  std::vector<double> &GetCrystalDirZ() { return fCrystalDirZ; }
  std::vector<double> &GetCrystalKineticEnergy() {
    return fCrystalKineticEnergy;
  }
  std::vector<int> &GetCrystalProcessIDs() { return fCrystalProcessIDs; }
  std::vector<double> &GetCrystalTrackLength() { return fCrystalTrackLength; }

  // Primary Particle Getters
  std::vector<int> &GetPrimaryPDG() { return fPrimaryPDG; }
  std::vector<double> &GetPrimaryEnergy() { return fPrimaryEnergy; }
  std::vector<double> &GetPrimaryPosX() { return fPrimaryPosX; }
  std::vector<double> &GetPrimaryPosY() { return fPrimaryPosY; }
  std::vector<double> &GetPrimaryPosZ() { return fPrimaryPosZ; }
  std::vector<double> &GetPrimaryDirX() { return fPrimaryDirX; }
  std::vector<double> &GetPrimaryDirY() { return fPrimaryDirY; }
  std::vector<double> &GetPrimaryDirZ() { return fPrimaryDirZ; }

  int GetProcessID(const G4String &processName);

private:
  std::vector<int> fCrystalIDs;
  std::vector<double> fCrystalEdeps;
  std::vector<double> fCrystalTimes;
  std::vector<double> fCrystalPosX;
  std::vector<double> fCrystalPosY;
  std::vector<double> fCrystalPosZ;
  std::vector<int> fCrystalPDGs;
  std::vector<int> fCrystalParentIDs;
  std::vector<double> fCrystalDirX;
  std::vector<double> fCrystalDirY;
  std::vector<double> fCrystalDirZ;
  std::vector<double> fCrystalKineticEnergy;
  std::vector<int> fCrystalProcessIDs;
  std::vector<double> fCrystalTrackLength;

  // Primary Particle Vectors
  std::vector<int> fPrimaryPDG;
  std::vector<double> fPrimaryEnergy;
  std::vector<double> fPrimaryPosX;
  std::vector<double> fPrimaryPosY;
  std::vector<double> fPrimaryPosZ;
  std::vector<double> fPrimaryDirX;
  std::vector<double> fPrimaryDirY;
  std::vector<double> fPrimaryDirZ;

  std::map<G4String, int> fProcessMap;
};

#endif
