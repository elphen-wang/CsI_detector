#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "PhysicsList.hh"

#include <G4RunManager.hh>
#include <G4UIExecutive.hh>
#include <G4UImanager.hh>
#include <G4VisExecutive.hh>

int main(int argc, char **argv) {
  auto runManager = new G4RunManager();

  // Register detector construction and physics list
  runManager->SetUserInitialization(new DetectorConstruction());
  runManager->SetUserInitialization(new PhysicsList());

  // Register user action initialization
  runManager->SetUserInitialization(new ActionInitialization());

  auto visManager = new G4VisExecutive();
  visManager->Initialize();

  auto UImanager = G4UImanager::GetUIpointer();

  G4UIExecutive *ui = nullptr;
  if (argc == 1) {
    // No macro file provided: start interactive session
    ui = new G4UIExecutive(argc, argv);
    UImanager->ApplyCommand("/control/execute init_vis.mac");
  } else {
    // Execute the macro file (e.g., ./program run.mac)
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    UImanager->ApplyCommand(command + fileName);
  }

  if (ui) {
    ui->SessionStart();
    delete ui;
  }

  delete visManager;
  delete runManager;
  return 0;
}
