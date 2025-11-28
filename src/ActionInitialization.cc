// ActionInitialization.cc
#include "ActionInitialization.hh"
#include "EventAction.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "TrackingAction.hh"

ActionInitialization::ActionInitialization() {}

ActionInitialization::~ActionInitialization() {}

void ActionInitialization::BuildForMaster() const {
  SetUserAction(new RunAction());
}

void ActionInitialization::Build() const {
  SetUserAction(new PrimaryGeneratorAction());
  SetUserAction(new TrackingAction());
  SetUserAction(new RunAction());
  SetUserAction(new EventAction());
}
