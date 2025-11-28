// PhysicsList.cc
#include "PhysicsList.hh"
#include "G4EmStandardPhysics.hh"
// 必须包含单位头文件，确保 mm 可用
#include "G4SystemOfUnits.hh"

PhysicsList::PhysicsList()
    : FTFP_BERT() // 调用父类构造函数
{
  SetVerboseLevel(1); // 输出详细程度
  // 如果想替换默认电磁物理过程
  RegisterPhysics(new G4EmStandardPhysics());
}

PhysicsList::~PhysicsList() {}
