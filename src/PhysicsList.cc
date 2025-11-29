// PhysicsList.cc
#include "PhysicsList.hh"
#include "G4DecayPhysics.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4SystemOfUnits.hh"

PhysicsList::PhysicsList()
    : G4VModularPhysicsList() // 从头构建，不继承FTFP_BERT
{
  SetVerboseLevel(1);

  // 对于低能电子/正电子（0-4 MeV）模拟，只需要：
  // 1. 高精度电磁物理（option4是最精确的标准选项）
  RegisterPhysics(new G4EmStandardPhysics_option4());

  // 2. 粒子衰变（包括正电子湮灭）
  RegisterPhysics(new G4DecayPhysics());

  // 设置次级粒子产生阈值（可选，提高精度）
  SetDefaultCutValue(0.1 * mm); // 默认产生次级粒子的阈值
}

PhysicsList::~PhysicsList() {}
