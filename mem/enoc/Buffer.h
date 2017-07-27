
#ifndef _Buffer_H_
#define _Buffer_H_

#include <vector>
#include "VirtualChannel.h"
#include "flit.h"
#include "config.h"


class Buffer : public Module {
  
  int _size;

  vector<VirtualChannel * >   _vc;
  //vector<int>               _vc_time_stamp;
  vector <int>              _occupancy;


public:
  
  Buffer(  const Configuration &config,Module *parent, const string& name );
//  ~Buffer();

  void AddFlit( int vc, Flit *f );


  inline Flit *RemoveFlit( int vc )
  {
    --_occupancy[vc];
    return _vc[vc]->RemoveFlit( );
  }
  
  inline Flit *FrontFlit( int vc ) const
  {
    return _vc[vc]->FrontFlit( );
  }
  
  inline bool Empty( int vc ) const
  {
    return _vc[vc]->Empty( );
  }

  inline bool Full( int vc) const
  {
    return _occupancy[vc] >= _size;
  }

  inline VirtualChannel::VCState GetState( int vc ) const
  {
    return _vc[vc]->GetState( );
  }
  
  inline void SetState( int vc, VirtualChannel::VCState s )
  {
    _vc[vc]->SetState(s);
  }  
  inline int GetOutputPort( int vc ) const
  {
    return _vc[vc]->GetOutputPort( );
  }
  inline int GetOutputVC( int vc ) const
  {
    return _vc[vc]->GetOutputVC( );
  }  
/*  inline void SetRouteSet( int vc, OutputSet * output_set )
  {
    _vc[vc]->SetRouteSet(output_set);
  }*/
  inline void Route( int vc, int id, Flit* f, int in_channel )
  {
    _vc[vc]->Route(id, f, in_channel);
  }  

  inline void SetOutput( int vc, int out_port, int out_vc )
  {
    _vc[vc]->SetOutput(out_port, out_vc);
  }
  inline int GetOccupancy( int vc ) const
  {
    return _vc[vc]->GetOccupancy( );
  }  
};

#endif
