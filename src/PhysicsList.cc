// PhysicsList.cc
#include "PhysicsList.hh"
#include "G4EmStandardPhysics.hh"
// 必须包含单位头文件，确保 mm 可用
#include "G4SystemOfUnits.hh"

PhysicsList::PhysicsList()
 : FTFP_BERT()   // 调用父类构造函数
{
    SetVerboseLevel(1); // 输出详细程度
    // 如果想替换默认电磁物理过程
    RegisterPhysics(new G4EmStandardPhysics());
}

PhysicsList::~PhysicsList() {}

/*
    什么是粒子切割（Range Cut）？
    Range Cut 指的是“距离切割长度”——即Geant4在生成二次粒子（例如光子在物质中产生的电子、电子产生的次级粒子等）时，给出的一个阈值距离。

    换句话说，当一个粒子在介质中的剩余路径长度（范围，range）小于这个切割距离时，Geant4不会为它产生二次粒子，而是把剩余能量直接沉积掉（本地耗散）。

    为什么要用粒子切割？
    真实情况下，带电粒子经过物质时会产生大量极低能量的二级粒子，如果对所有这些粒子都精确追踪，计算量会非常大，模拟时间长。

    通过设置合理的切割距离，Geant4只产生路径较长、对探测器响应明显的二级粒子，忽略极低能量二次粒子，使得模拟更高效。

    同时切割距离的大小也影响模拟的物理精度，小切割距离能获得更细节的模拟，但计算时间增加。
*/

void PhysicsList::SetCuts()
{
    // 设定cut value范围，例如1 mm
    SetCutsWithDefault();

    G4double cutValue = 1.0 * mm;
    SetCutValue(cutValue, "e-");
    SetCutValue(cutValue, "e+");
    SetCutValue(cutValue, "gamma");

    // 输出切割信息，方便调试
    DumpCutValuesTable();
}
