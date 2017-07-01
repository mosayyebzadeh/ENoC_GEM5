
#include "flitchannel.h"

#include <iostream>
#include <iomanip>

#include "router.h"
#include "NoCMain.h"


FlitChannel::FlitChannel(Module * parent, string const & name, int classes)
: Channel<Flit>(parent, name), _routerSource(NULL), _routerSourcePort(-1), 
  _routerSink(NULL), _routerSinkPort(-1), _idle(0) {
  _active.resize(classes, 0);
}

void FlitChannel::SetSource(Router const * const router, int port) {
  _routerSource = router;
  _routerSourcePort = port;
}

void FlitChannel::SetSink(Router const * const router, int port) {
  _routerSink = router;
  _routerSinkPort = port;
}

void FlitChannel::Send(Flit * f) {
  if(f) {
  } else {
    ++_idle;
  }
  Channel<Flit>::Send(f);
}

void FlitChannel::ReadInputs(int global_clk) {
  //Flit const * const & f = _input;
  Channel<Flit>::ReadInputs(global_clk);
}

void FlitChannel::WriteOutputs(int global_clk) {
  Channel<Flit>::WriteOutputs(global_clk);
}

void FlitChannel::clk(int global_clk){
    
}
