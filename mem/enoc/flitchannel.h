

#ifndef FLITCHANNEL_H
#define FLITCHANNEL_H


#include "channel.h"
#include "flit.h"

using namespace std;

class ENoCRouter ;

class FlitChannel : public Channel<Flit> {
public:
  FlitChannel(Module * parent, string const & name, int classes);

  void SetSource(ENoCRouter const * const router, int port) ;
  
  inline ENoCRouter const * GetSource() const {
    return _routerSource;
  }
  
  inline int const & GetSourcePort() const {
    return _routerSourcePort;
  }
  
  void SetSink(ENoCRouter const * const router, int port) ;
  
  inline ENoCRouter const * GetSink() const {
    return _routerSink;
  }
  inline int const & GetSinkPort() const {
    return _routerSinkPort;
  }
  inline vector<int> const & GetActivity() const {
    return _active;
  }

  // Send flit 
    void Send(Flit * flit);
    void clk(int global_clk) ;
    void ReadInputs(int global_clk);
    void WriteOutputs(int global_clk);

private:
  
  ////////////////////////////////////////
  //
  // Power Models OBSOLETE
  //
  ////////////////////////////////////////

  ENoCRouter const * _routerSource;
  int _routerSourcePort;
  ENoCRouter const * _routerSink;
  int _routerSinkPort;

  // Statistics for Activity Factors
  vector<int> _active;
  int _idle;
};

#endif
