
#ifndef _Network_H_
#define _Network_H_

#include <vector>
#include "router.h"
#include "flitchannel.h"
#include "NoCMain.h"


class NetworkNoC : public TimedModule{


private:
    // Tile  *t[MAX_STATIC_DIM][MAX_STATIC_DIM];
     
     vector<ENoCRouter *> _routers;
     vector<FlitChannel *> _chan;
     vector<FlitChannel *> _inject;
     vector<FlitChannel *> _eject;
     
     deque<TimedModule *> _timed_modules;
     
     int _k  ;
     int _n  ;
     int _channels;
     int _size;
     int _nodes ;
  
public: 
     NetworkNoC( const Configuration &config, const string & name );
     ~NetworkNoC( );
     void _Alloc( );
     void buildMesh( const Configuration &config);
     bool load_traffic(char* fname);
     void clk(int global_clk);
     
     const vector<FlitChannel *> & GetInject() {return _inject;}
     FlitChannel * GetInject(int index) {return _inject[index];}
     const vector<FlitChannel *> & GetEject(){return _eject;}
     FlitChannel * GetEject(int index) {return _eject[index];}
     
     int NumNodes( ) const {return _size;}
     int NumRouters() const {return _size;}
     const vector<ENoCRouter *> & GetRouters(){return _routers;}
       void WriteFlit( Flit *f, int source );
       Flit *ReadFlit( int dest );
       void ReadInputs(int global_clk);
     int powi( int x, int y );
     int _LeftNode( int node, int dim );
     int _RightNode( int node, int dim );
     int _LeftChannel( int node, int dim );
     int _RightChannel( int node, int dim );
     //void WriteFlit( Flit *f, int source );
     // Flit *ReadFlit( int dest );
     void WriteOutputs(int global_clk);

};

#endif
