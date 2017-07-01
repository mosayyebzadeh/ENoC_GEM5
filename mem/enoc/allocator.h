#ifndef _Allocator_H_
#define _Allocator_H_

#include <string>
#include <vector>
#include <set>
#include <map>
#include "module.h"

using namespace std;


class Allocator : public Module {
protected:
  const int _inputs;
  const int _outputs;

  bool _dirty;

  vector<int> _inmatch;
  vector<int> _outmatch;

public:

  struct sRequest {
    int port;
    int label;
    int in_pri;
    int out_pri;
  };

  Allocator( Module *parent, const string& name,
	     int inputs, int outputs );

  virtual void Clear( );
  
  virtual int  ReadRequest( int in, int out ) const = 0;
  virtual bool ReadRequest( sRequest &req, int in, int out ) const = 0;

  virtual void AddRequest( int in, int out, int label = 1, 
			   int in_pri = 0, int out_pri = 0 );
  virtual void RemoveRequest( int in, int out, int label = 1 ) = 0;
  
  virtual void Allocate( ) = 0;

  int OutputAssigned( int in ) const;
  int InputAssigned( int out ) const;

  virtual bool OutputHasRequests( int out ) const = 0;
  virtual bool InputHasRequests( int in ) const = 0;

  virtual int NumOutputRequests( int out ) const = 0;
  virtual int NumInputRequests( int in ) const = 0;

  virtual void PrintRequests( ostream * os = NULL ) const = 0;
  void PrintGrants( ostream * os = NULL ) const;

  //static Allocator *NewAllocator( Module *parent, const string& name,  const string &alloc_type, int inputs, int outputs );
};

class SparseAllocator : public Allocator {
protected:
  set<int> _in_occ;
  set<int> _out_occ;
  
  vector<map<int, sRequest> > _in_req;
  vector<map<int, sRequest> > _out_req;

public:
  SparseAllocator( Module *parent, const string& name,
		   int inputs, int outputs );

  void Clear( );
  
  int  ReadRequest( int in, int out ) const;
  bool ReadRequest( sRequest &req, int in, int out ) const;

  void AddRequest( int in, int out, int label = 1, 
		   int in_pri = 0, int out_pri = 0 );
  void RemoveRequest( int in, int out, int label = 1 );
  
  bool OutputHasRequests( int out ) const;
  bool InputHasRequests( int in ) const;

  int NumOutputRequests( int out ) const;
  int NumInputRequests( int in ) const;

  void PrintRequests( ostream * os = NULL ) const;

};

/*class SeparableAllocator : public SparseAllocator {
  
protected:

  vector<Arbiter*> _input_arb ;
  vector<Arbiter*> _output_arb ;

public:
  
  SeparableAllocator( Module* parent, const string& name, int inputs,
		      int outputs, const string& arb_type ) ;
  
  virtual ~SeparableAllocator() ;

  virtual void Clear() ;

} ;

class SeparableInputFirstAllocator : public SeparableAllocator {

public:
  
  SeparableInputFirstAllocator( Module* parent, const string& name, int inputs,
				int outputs, const string& arb_type ) ;

  virtual void Allocate() ;

} ;

class SeparableInputFirstAllocator : public SeparableAllocator {

public:
  
  SeparableInputFirstAllocator( Module* parent, const string& name, int inputs,
				int outputs, const string& arb_type ) ;

  virtual void Allocate() ;

} ;
*/

class iSLIP_Sparse : public SparseAllocator {
  int _iSLIP_iter;

  vector<int> _gptrs;
  vector<int> _aptrs;

public:
  iSLIP_Sparse( Module *parent, const string& name,
		int inputs, int outputs, int iters );

  void Allocate( );
};
 
#endif