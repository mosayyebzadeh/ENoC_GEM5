
#include "Network.h"
#include "NoCMain.h"
#include <stdio.h>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
using namespace std;





int NetworkNoC::_LeftChannel( int node, int dim )
{
  // The base channel for a node is 2*_n*node
  int base = 2*_n*node;
  // The offset for a left channel is 2*dim + 1
  int off  = 2*dim + 1;

  return ( base + off );
}

int NetworkNoC::_RightChannel( int node, int dim )
{
  // The base channel for a node is 2*_n*node
  int base = 2*_n*node;
  // The offset for a right channel is 2*dim 
  int off  = 2*dim;
  return ( base + off );
}

int NetworkNoC::powi( int x, int y ) // compute x to the y
{
  int r = 1;

  for ( int i = 0; i < y; ++i ) {
    r *= x;
  }

  return r;
}
int NetworkNoC::_LeftNode( int node, int dim )
{
  int k_to_dim = powi( _k, dim );
  int loc_in_dim = ( node / k_to_dim ) % _k;
  int left_node;
  // if at the left edge of the dimension, wraparound
  if ( loc_in_dim == 0 ) {
    left_node = node + (_k-1)*k_to_dim;
  } else {
    left_node = node - k_to_dim;
  }

  return left_node;
}

int NetworkNoC::_RightNode( int node, int dim )
{
  int k_to_dim = powi( _k, dim );
  int loc_in_dim = ( node / k_to_dim ) % _k;
  int right_node;
  // if at the right edge of the dimension, wraparound
  if ( loc_in_dim == ( _k-1 ) ) {
    right_node = node - (_k-1)*k_to_dim;
  } else {
    right_node = node + k_to_dim;
  }

  return right_node;
}

void NetworkNoC::clk(int global_clk)
{

    //if(GlobalParams::global_clk %1000 == 0)
    //    cout << " clk :  "  << GlobalParams::global_clk << endl;
    //if(global_clk %1000 == 0)
    //    cout << " clk :  "  << global_clk << endl;

    
        ReadInputs(global_clk);
        deque<TimedModule *>::const_iterator iter;
        for( iter = _timed_modules.begin(); iter != _timed_modules.end(); ++iter) {
            (*iter)->clk( global_clk);
        }

	WriteOutputs(global_clk);
  
}



void NetworkNoC::ReadInputs(int global_clk)
{
  for(deque<TimedModule *>::const_iterator iter = _timed_modules.begin();iter != _timed_modules.end();  ++iter) {
    (*iter)->ReadInputs(global_clk);
  }
}

void NetworkNoC::WriteOutputs(int global_clk)
{
  for(deque<TimedModule *>::const_iterator iter = _timed_modules.begin();
      iter != _timed_modules.end();
      ++iter) {
    (*iter)->WriteOutputs(global_clk);
  }
}


void NetworkNoC::_Alloc( )
{

  _routers.resize(_size);
   
  //int  gNodes = _size; //_nodes;

  int _classes = 1;
  _inject.resize(_nodes);

  for ( int s = 0; s < _nodes; ++s ) {
    ostringstream name;
    name << Name() << "_fchan_ingress" << s;
    _inject[s] = new FlitChannel(this, name.str(), _classes);
    _inject[s]->SetSource(NULL, s);
    _timed_modules.push_back(_inject[s]);

  }
  _eject.resize(_nodes);

  for ( int d = 0; d < _nodes; ++d ) {
    ostringstream name;
    name << Name() << "_fchan_egress" << d;
    _eject[d] = new FlitChannel(this, name.str(), _classes);
    _eject[d]->SetSink(NULL, d);
    _timed_modules.push_back(_eject[d]);

  }
  _chan.resize(_channels);

  for ( int c = 0; c < _channels; ++c ) {
    ostringstream name;
    name << Name() << "_fchan_" << c;
    _chan[c] = new FlitChannel(this, name.str(), _classes);
    _timed_modules.push_back(_chan[c]);

  }
}

void NetworkNoC::buildMesh(const Configuration &config)
{
   
        int left_node;
        int right_node;

        int right_input;
        int left_input;

        int right_output;
        int left_output;
        
        int latency = 1;
        
        //int _size = GlobalParams::mesh_dim_x * GlobalParams::mesh_dim_y;
        
        for ( int node = 0; node < _size; ++node ) {

                ostringstream router_name;
                router_name << "router";
                
                if ( _k > 1 ) {
                for ( int dim_offset = _size / _k; dim_offset >= 1; dim_offset /= _k ) {
                    router_name << "_" << ( node / dim_offset ) % _k;
					}
                }
            
             _routers[node] = ENoCRouter::NewRouter( config, this, router_name.str( ), node, 2*_n + 1, 2*_n + 1 );
            
            _timed_modules.push_back(_routers[node]);
                      
            for ( int dim = 0; dim < _n; ++dim ) {

                //find the neighbor 
                left_node  = _LeftNode( node, dim );
                right_node = _RightNode( node, dim );

                //get the input channel number
                right_input = _LeftChannel( right_node, dim );
                left_input  = _RightChannel( left_node, dim );
                
                //_routers[node]->SetID(node);

                //add the input channel
                _routers[node]->AddInputChannel( _chan[right_input] );
                _routers[node]->AddInputChannel( _chan[left_input] );
                             
                
                //get the output channel number
                right_output = _RightChannel( node, dim );
                left_output  = _LeftChannel( node, dim );
                
                //add the output channel
                _routers[node]->AddOutputChannel( _chan[right_output]  );
                _routers[node]->AddOutputChannel( _chan[left_output]   );

                _chan[right_input]->SetLatency( latency );
                _chan[left_input]->SetLatency( latency );
                    
                _chan[right_output]->SetLatency( latency );
                _chan[left_output]->SetLatency( latency );      

            }

			_routers[node]->AddInputChannel(_inject[node]);
			_routers[node]->AddOutputChannel(_eject[node]);
			_inject[node]->SetLatency(latency);
			_eject[node]->SetLatency(latency);
        }
}


NetworkNoC::NetworkNoC( const Configuration &config, const string & name ) :
  TimedModule( 0, name )
{





   //_k = config.GetInt("dimx") ;//GlobalParams::mesh_dim_x;
   _k = DEFAULT_MESH_DIM_X ;//GlobalParams::mesh_dim_x;
    _n = 2;
   _size     = powi( _k, _n );

//	printf("AMIIIIIN TEST: %s: K is %d, n is %d, size is %d\n", __func__, _k, _n, _size);

   _nodes = _size;
  _channels = 2*_n*_size;  
  
    _Alloc( );
    buildMesh(config);  
}

NetworkNoC::~NetworkNoC( )
{
  for ( int r = 0; r < _size; ++r ) {
    if ( _routers[r] ) delete _routers[r];
  }
  for ( int s = 0; s < _nodes; ++s ) {
    if ( _inject[s] ) delete _inject[s];
 
  }
  for ( int d = 0; d < _nodes; ++d ) {
    if ( _eject[d] ) delete _eject[d];
  
  }
  for ( int c = 0; c < _channels; ++c ) {
    if ( _chan[c] ) delete _chan[c];
 
  }
}



void NetworkNoC::WriteFlit( Flit *f, int source )
{
  assert( ( source >= 0 ) && ( source < _nodes ) );
  _inject[source]->Send(f);
}

Flit *NetworkNoC::ReadFlit( int dest )
{
  assert( ( dest >= 0 ) && ( dest < _nodes ) );
  return _eject[dest]->Receive();
}
