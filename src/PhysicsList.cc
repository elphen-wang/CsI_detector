// PhysicsList.cc
#include "PhysicsList.hh"
#include "G4DecayPhysics.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4OpticalPhysics.hh"
#include "G4SystemOfUnits.hh"

PhysicsList::PhysicsList()
    : G4VModularPhysicsList() // 从头构建，不继承FTFP_BERT
{
  fMessenger =
      new G4GenericMessenger(this, "/CsI/physics/", "Physics List Control");
  fMessenger->DeclareMethod("optical", &PhysicsList::SetOpticalPhysics,
                            "Enable Optical Physics");
  fMessenger->DeclareMethod("verbose", &PhysicsList::SetVerboseLevel,
                            "Set physics list verbose level");

  SetVerboseLevel(1);

  // 对于低能电子/正电子（0-4 MeV）模拟，只需要：
  // 1. 高精度电磁物理（option4是最精确的标准选项）
  RegisterPhysics(new G4EmStandardPhysics_option4());

  // 2. 粒子衰变（包括正电子湮灭）
  RegisterPhysics(new G4DecayPhysics());

  // 3. 光学物理 (默认开启，方便调试)
  // RegisterPhysics(new G4OpticalPhysics());

  // 设置次级粒子产生阈值（可选，提高精度）
  SetDefaultCutValue(0.001 * mm); // 默认产生次级粒子的阈值
}

PhysicsList::~PhysicsList() { delete fMessenger; }

void PhysicsList::SetOpticalPhysics(G4bool on) {
  G4cout << ">>> SetOpticalPhysics called with: " << on << '\n';
  if (on) {
    RegisterPhysics(new G4OpticalPhysics());
    G4cout << ">>> Optical Physics Enabled!" << '\n';
  }
}
