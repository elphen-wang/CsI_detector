cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(CsI_Axion)

find_package(Geant4 REQUIRED ui_all vis_all)

include(${Geant4_USE_FILE})

include_directories(
  ${PROJECT_SOURCE_DIR}/include    # 你自己的 public headers
  ${Geant4_INCLUDE_DIRS}           # Geant4 的头文件
)

add_executable(CsI_Axion
  src/main.cc
  src/DetectorConstruction.cc
  src/PhysicsList.cc
  src/DetectorSD.cc
  src/PrimaryGeneratorAction.cc
  src/TrackingAction.cc
  src/ActionInitialization.cc
  src/Trajectory.cc  # 确保这一行存在
)

target_include_directories(CsI_Axion PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(CsI_Axion ${Geant4_LIBRARIES})

