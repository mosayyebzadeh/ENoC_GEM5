
#include "routefunc.h"



map<string, tRoutingFunction> gRoutingFunctionMap;



int dor_next_mesh( int cur, int dest, bool descending )
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

void xy_yx_mesh( const ENoCRouter *r,  Flit *f, int in_channel  )
{
  //int vcBegin = 0, vcEnd = GlobalParams::num_vcs-1;
  int vcBegin = 0, vcEnd = DEFAULT_NUM_VCS-1;
  int gN = 2 ; // Number  of dimensions

  int out_port;

  if(r->GetID() == f->dst_id) {

    // at destination router, we don't need to separate VCs by dim order
    out_port = 2*gN;

  } else {

    //each class must have at least 2 vcs assigned or else xy_yx will deadlock
    int const available_vcs = (vcEnd - vcBegin + 1) / 2;
    

    // Route order (XY or YX) determined when packet is injected
    //  into the network
    bool x_then_y = ((in_channel < 2*gN) ?
		     (f->vc < (vcBegin + available_vcs)) :
		     (RandomInt(1) > 0));

    if(x_then_y) {
      out_port = dor_next_mesh( r->GetID(), f->dst_id, false );
      vcEnd -= available_vcs;
    } else {
      out_port = dor_next_mesh( r->GetID(), f->dst_id, true );
      vcBegin += available_vcs;
    }

  }

 f->output_port_vc_start    = vcBegin; 
 f->output_port_vc_end      = vcEnd;
 f->output_port             = out_port;
  
}

void InitializeRoutingMap(  )
{
      gRoutingFunctionMap["xy_yx_mesh"]          = &xy_yx_mesh;
    
}

