 


#include "allocator.h"
//#include "arbiter.h"
#include <vector>
#include <iostream>
#include <cstring>

//==================================================
// Allocator base class
//==================================================

Allocator::Allocator( Module *parent, const string& name,
		      int inputs, int outputs ) :
Module( parent, name ), _inputs( inputs ), _outputs( outputs ), _dirty( false )
{
  _inmatch.resize(_inputs, -1);   
  _outmatch.resize(_outputs, -1);
}

void Allocator::Clear( )
{
  if(_dirty) {
    _inmatch.assign(_inputs, -1);
    _outmatch.assign(_outputs, -1);
    _dirty = false;
  }
}

void Allocator::AddRequest( int in, int out, int label, int in_pri,
			    int out_pri ) {

 // assert( ( in >= 0 ) && ( in < _inputs ) );
 // assert( ( out >= 0 ) && ( out < _outputs ) );
 // assert( label >= 0 );
  _dirty = true;
}

int Allocator::OutputAssigned( int in ) const
{
//  assert( ( in >= 0 ) && ( in < _inputs ) );

  return _inmatch[in];
}

int Allocator::InputAssigned( int out ) const
{
 // assert( ( out >= 0 ) && ( out < _outputs ) );

  return _outmatch[out];
}

void Allocator::PrintGrants( ostream * os ) const
{
  if(!os) os = &cout;

  *os << "Input grants = [ ";
  for ( int input = 0; input < _inputs; ++input ) {
    if(_inmatch[input] >= 0) {
      *os << input << " -> " << _inmatch[input] << "  ";
    }
  }
  *os << "], output grants = [ ";
  for ( int output = 0; output < _outputs; ++output ) {
    if(_outmatch[output] >= 0) {
      *os << output << " -> " << _outmatch[output] << "  ";
    }
  }
  *os << "]." << endl;
}

//==================================================
// SparseAllocator
//==================================================

SparseAllocator::SparseAllocator( Module *parent, const string& name,
				  int inputs, int outputs ) :
  Allocator( parent, name, inputs, outputs )
{
  _in_req.resize(_inputs);
  _out_req.resize(_outputs);
}


void SparseAllocator::Clear( )
{
  for ( int i = 0; i < _inputs; ++i ) {
    if(!_in_req[i].empty())
      _in_req[i].clear( );
  }

  for ( int j = 0; j < _outputs; ++j ) {
    if(!_out_req[j].empty())
      _out_req[j].clear( );
  }

  _in_occ.clear( );
  _out_occ.clear( );

  Allocator::Clear();
}

int SparseAllocator::ReadRequest( int in, int out ) const
{
  sRequest r;

  if ( ! ReadRequest( r, in, out ) ) {
    r.label = -1;
  } 

  return r.label;
}

bool SparseAllocator::ReadRequest( sRequest &req, int in, int out ) const
{
  bool found;

  //assert( ( in >= 0 ) && ( in < _inputs ) );
  //assert( ( out >= 0 ) && ( out < _outputs ) );

  map<int, sRequest>::const_iterator match = _in_req[in].find(out);
  if ( match != _in_req[in].end( ) ) {
    req = match->second;
    found = true;
  } else {
    found = false;
  }

  return found;
}

void SparseAllocator::AddRequest( int in, int out, int label, 
				  int in_pri, int out_pri )
{
  Allocator::AddRequest(in, out, label, in_pri, out_pri);
 // assert( _in_req[in].count(out) == 0 );
 // assert( _out_req[out].count(in) == 0 );

  // insert into occupied inputs set if
  // input is currently empty
  if ( _in_req[in].empty( ) ) {
    _in_occ.insert(in);
  }

  // similarly for the output
  if ( _out_req[out].empty( ) ) {
    _out_occ.insert(out);
  }

  sRequest req;
  req.port    = out;
  req.label   = label;
  req.in_pri  = in_pri;
  req.out_pri = out_pri;

  _in_req[in][out] = req;

  req.port  = in;

  _out_req[out][in] = req;
}

void SparseAllocator::RemoveRequest( int in, int out, int label )
{
//  assert( ( in >= 0 ) && ( in < _inputs ) );
// assert( ( out >= 0 ) && ( out < _outputs ) ); 
  
//  assert( _in_req[in].count( out ) > 0 );
//  assert( _in_req[in][out].label == label );
  _in_req[in].erase( out );

  // remove from occupied inputs list if
  // input is now empty
  if ( _in_req[in].empty( ) ) {
    _in_occ.erase(in);
  }

  // similarly for the output
//  assert( _out_req[out].count( in ) > 0 );
//  assert( _out_req[out][in].label == label );
  _out_req[out].erase( in );

  if ( _out_req[out].empty( ) ) {
    _out_occ.erase(out);
  }
  _inmatch[in] = -1;
  _outmatch[out] = -1;
}

bool SparseAllocator::InputHasRequests( int in ) const
{
  return _in_occ.count(in) > 0;
}

bool SparseAllocator::OutputHasRequests( int out ) const
{
  return _out_occ.count(out) > 0;
}

int SparseAllocator::NumInputRequests( int in ) const
{
  return _in_occ.count(in);
}

int SparseAllocator::NumOutputRequests( int out ) const
{
  return _out_occ.count(out);
}

void SparseAllocator::PrintRequests( ostream * os ) const
{
  map<int, sRequest>::const_iterator iter;
  
  if(!os) os = &cout;
  
  *os << "Input requests = [ ";
  for ( int input = 0; input < _inputs; ++input ) {
    if(!_in_req[input].empty()) {
      *os << input << " -> [ ";
      for ( iter = _in_req[input].begin( ); 
	    iter != _in_req[input].end( ); iter++ ) {
	*os << iter->second.port << "@" << iter->second.in_pri << " ";
      }
      *os << "]  ";
    }
  }
  *os << "], output requests = [ ";
  for ( int output = 0; output < _outputs; ++output ) {
    if(!_out_req[output].empty()) {
      *os << output << " -> ";
      *os << "[ ";
      for ( iter = _out_req[output].begin( ); 
	    iter != _out_req[output].end( ); iter++ ) {
	*os << iter->second.port << "@" << iter->second.out_pri << " ";
      }
      *os << "]  ";
    }
  }
  *os << "]." << endl;
}
/*
SeparableInputFirstAllocator::SeparableInputFirstAllocator( Module* parent, const string& name, int inputs,
			      int outputs, const string& arb_type ) : SeparableAllocator( parent, name, inputs, outputs, arb_type )
{}

void SeparableInputFirstAllocator::Allocate() {
  
  set<int>::const_iterator port_iter = _in_occ.begin();
  while(port_iter != _in_occ.end()) {
    
    const int & input = *port_iter;

    // add requests to the input arbiter

    map<int, sRequest>::const_iterator req_iter = _in_req[input].begin();
    while(req_iter != _in_req[input].end()) {

      const sRequest & req = req_iter->second;
      
      _input_arb[input]->AddRequest(req.port, req.label, req.in_pri);

      ++req_iter;
    }

    // Execute the input arbiters and propagate the grants to the
    // output arbiters.

    int label = -1;
    const int output = _input_arb[input]->Arbitrate(&label, NULL);
    assert(output > -1);

    const sRequest & req = _out_req[output][input]; 
    assert((req.port == input) && (req.label == label));

    _output_arb[output]->AddRequest(req.port, req.label, req.out_pri);

    ++port_iter;
  }

  port_iter = _out_occ.begin();
  while(port_iter != _out_occ.end()) {

    const int & output = *port_iter;

    // Execute the output arbiters.
    
    const int input = _output_arb[output]->Arbitrate(NULL, NULL);

    if(input > -1) {
      assert((_inmatch[input] == -1) && (_outmatch[output] == -1));

      _inmatch[input] = output ;
      _outmatch[output] = input ;
      _input_arb[input]->UpdateState() ;
      _output_arb[output]->UpdateState() ;
    }

    ++port_iter;
  }
}
*/

iSLIP_Sparse::iSLIP_Sparse( Module *parent, const string& name,
			    int inputs, int outputs, int iters ) :
  SparseAllocator( parent, name, inputs, outputs ),
  _iSLIP_iter(iters)
{
  _gptrs.resize(_outputs, 0);
  _aptrs.resize(_inputs, 0);
}

void iSLIP_Sparse::Allocate( )
{
  int input;
  int output;

  int input_offset;
  int output_offset;

  map<int, sRequest>::iterator p;
  bool wrapped;

  for ( int iter = 0; iter < _iSLIP_iter; ++iter ) {
    // Grant phase

    vector<int> grants(_outputs, -1);

    for ( output = 0; output < _outputs; ++output ) {

      // Skip loop if there are no requests
      // or the output is already matched
      if ( ( _out_req[output].empty( ) ) ||
	   ( _outmatch[output] != -1 ) ) {
	continue;
      }

      // A round-robin arbiter between input requests
      input_offset = _gptrs[output];

      p = _out_req[output].begin( );
      while( ( p != _out_req[output].end( ) ) &&
	     ( p->second.port < input_offset ) ) {
	p++;
      }

      wrapped = false;
      while( (!wrapped) || 
	     ( ( p != _out_req[output].end( ) ) &&
	       ( p->second.port < input_offset ) ) ) {
	if ( p == _out_req[output].end( ) ) {
	  if ( wrapped ) { break; }
	  // p is valid here because empty lists
	  // are skipped (above)
	  p = _out_req[output].begin( );
	  wrapped = true;
	}

	input = p->second.port;

	// we know the output is free (above) and
	// if the input is free, grant request
	if ( _inmatch[input] == -1 ) {
	  grants[output] = input;
	  break;
	}

	p++;
      }      
    }

    // Accept phase

    for ( input = 0; input < _inputs; ++input ) {

      if ( _in_req[input].empty( ) ) {
	continue;
      }

      // A round-robin arbiter between output grants
      output_offset = _aptrs[input];

      p = _in_req[input].begin( );
      while( ( p != _in_req[input].end( ) ) &&
	     ( p->second.port < output_offset ) ) {
	p++;
      }

      wrapped = false;
      while( (!wrapped) || 
	     ( ( p != _in_req[input].end( ) ) &&
	       ( p->second.port < output_offset ) ) ) {
	if ( p == _in_req[input].end( ) ) {
	  if ( wrapped ) { break; }
	  // p is valid here because empty lists
	  // are skipped (above)
	  p = _in_req[input].begin( );
	  wrapped = true;
	}

	output = p->second.port;

	// we know the output is free (above) and
	// if the input is free, grant request
	if ( grants[output] == input ) {
	  // Accept
	  _inmatch[input]   = output;
	  _outmatch[output] = input;

	  // Only update pointers if accepted during the 1st iteration
	  if ( iter == 0 ) {
	    _gptrs[output] = ( input + 1 ) % _inputs;
	    _aptrs[input]  = ( output + 1 ) % _outputs;
	  }

	  break;
	}

	p++;
      } 
    }
  }


}

