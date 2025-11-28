cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(CsI_Axion)

find_package(Geant4 REQUIRED ui_all vis_all)

include(${Geant4_USE_FILE})

include_directories(
  ${PROJECT_SOURCE_DIR}/include    # 你自己的 public headers
  ${Geant4_INCLUDE_DIRS}           # Geant4 的头文件
)

add_executable(CsI_Axion
  main.cc
  src/DetectorConstruction.cc
  src/PhysicsList.cc
  src/DetectorSD.cc
  src/PrimaryGeneratorAction.cc
  src/TrackingAction.cc
  src/ActionInitialization.cc
  src/Trajectory.cc
  src/RunAction.cc
  src/EventAction.cc
)

target_include_directories(CsI_Axion PRIVATE ${PROJECT_SOURCE_DIR}/include)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

target_link_libraries(CsI_Axion ${Geant4_LIBRARIES})

# Copy all scripts in mac/ to the build directory
file(GLOB MACRO_FILES "${PROJECT_SOURCE_DIR}/mac/*.mac")
file(COPY ${MACRO_FILES} DESTINATION ${CMAKE_BINARY_DIR})

