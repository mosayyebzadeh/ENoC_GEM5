
#ifndef _VirtualChannel_H_
#define _VirtualChannel_H_


#include <deque>
#include "flit.h"
#include "module.h"


class VirtualChannel : public Module {
  
public:
    enum VCState { idle , routing, vc_alloc, active };
    static const char * const VCSTATE[];
    VirtualChannel(  Module *parent, const string& name );
  
    //  ~VirtualChannel();
  
private:

  deque<Flit *> _buffer;
  
  VCState _state;
 // OutputSet *_route_set;
  
  int _out_port, _out_vc;

  int occupancy;

public:
  void AddFlit( Flit *f );
  Flit *RemoveFlit( );
  

  //void SetRouteSet( OutputSet * output_set );
    
  //void Route( tRoutingFunction rf, const Router* router,  Flit* f, int in_channel );
    void Route( int id,  Flit* f, int in_channel );

  inline Flit *FrontFlit( ) const
  {
    return _buffer.empty() ? NULL : _buffer.front();
  }

  inline bool Empty( ) const
  {
    return _buffer.empty( );
  }


  inline int GetOutputPort( ) const
  {
    return _out_port;
  }
  inline int GetOutputVC( ) const
  {
    return _out_vc;
  }
  
  inline VirtualChannel::VCState GetState( ) const
  {
    return _state;
  }
  inline int GetOccupancy() const
  {
     return occupancy; //return (int)_buffer.size();
  }


  
void SetState( VCState s );
void xy_yx_mesh(int id, Flit *f, int in_channel);
int dor_next_mesh(int cur, int dest, bool descending);
void SetOutput( int port, int vc );
};

#endif
