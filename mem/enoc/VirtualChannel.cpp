

#include <sstream>
#include "VirtualChannel.h"

const char * const VirtualChannel::VCSTATE[] = {"idle", "routing", "vc_alloc", "active"};

VirtualChannel::VirtualChannel( Module *parent, const string& name )
  : Module( parent, name ), 
    _state(idle), _out_port(-1), _out_vc(-1)
{    
    
  
    occupancy = 0;
  

}

/*void VirtualChannel::SetRouteSet( OutputSet * output_set )
{
  _route_set = output_set;
  _out_port = -1;
  _out_vc = -1;
}
*/

void VirtualChannel::SetOutput( int port, int vc )
{
  _out_port = port;
  _out_vc   = vc;
}


void VirtualChannel::AddFlit( Flit *f )
{
  
    _buffer.push_back(f);
    occupancy++;

}

Flit *VirtualChannel::RemoveFlit( )
{
  Flit *f = NULL;

  if ( !_buffer.empty( ) ) {

    f = _buffer.front( );
    _buffer.pop_front( );
    occupancy--;

  } else {
    cout << "Trying to remove flit from empty buffer.";
  }
  return f;
}

void VirtualChannel::Route( int id,  Flit* f, int in_channel )
{

    xy_yx_mesh( id,  f, in_channel  );
}




int VirtualChannel::dor_next_mesh( int cur, int dest, bool descending )
{
int gN = 2 ; // Number  of dimensions  
//int gK = GlobalParams::mesh_dim_x;
//int gNodes = GlobalParams::mesh_dim_x * GlobalParams::mesh_dim_x;
int gK = DEFAULT_MESH_DIM_X;
int gNodes = DEFAULT_MESH_DIM_X * DEFAULT_MESH_DIM_X;
  if ( cur == dest ) {
    return 2*gN;  // Eject
  }

  int dim_left;

  if(descending) {
    for ( dim_left = ( gN - 1 ); dim_left > 0; --dim_left ) {
      if ( ( cur * gK / gNodes ) != ( dest * gK / gNodes ) ) { break; }
      cur = (cur * gK) % gNodes; dest = (dest * gK) % gNodes;
    }
    cur = (cur * gK) / gNodes;
    dest = (dest * gK) / gNodes;
  } else {
    for ( dim_left = 0; dim_left < ( gN - 1 ); ++dim_left ) {
      if ( ( cur % gK ) != ( dest % gK ) ) { break; }
      cur /= gK; dest /= gK;
    }
    cur %= gK;
    dest %= gK;
  }

  if ( cur < dest ) {
    return 2*dim_left;     // Right
  } else {
    return 2*dim_left + 1; // Left
  }
}

void VirtualChannel::xy_yx_mesh( int id,  Flit *f, int in_channel  )
{
    int out_port; 
    int dst = f->dst_id;
 	//int cur_coordx = id % GlobalParams::mesh_dim_x;
	//int cur_coordy = id / GlobalParams::mesh_dim_x;
    
 	//int dst_coordx = dst % GlobalParams::mesh_dim_x;
	//int dst_coordy = dst / GlobalParams::mesh_dim_x;    
                
 	int cur_coordx = id % DEFAULT_MESH_DIM_X;
	int cur_coordy = id / DEFAULT_MESH_DIM_X;
    
 	int dst_coordx = dst % DEFAULT_MESH_DIM_X;
	int dst_coordy = dst / DEFAULT_MESH_DIM_X;    

  if( dst_coordx < cur_coordx)
      out_port = 1;   // west
  else if( dst_coordx > cur_coordx)  
      out_port = 0;   //east  
  else if( dst_coordy < cur_coordy)
      out_port = 3;   //up
  else if( dst_coordy > cur_coordy)      
      out_port = 2;   //down
  else
      out_port = 4;      
  
   f->output_port             = out_port;
}
void VirtualChannel::SetState( VCState s )
{
  //FrontFlit();
  
 
  _state = s;
}
