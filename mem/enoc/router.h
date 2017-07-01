
#ifndef _Router_H_
#define _Router_H_

#include <string>
#include <queue>
#include "allocator.h"
#include "Buffer.h"
#include "flit.h"
#include "flitchannel.h"
#include "statsNoC.h"
#include "NoCMain.h"
#include "element.h"



using namespace std;

class Router : public TimedModule{

public:
  
  int _id;
  int _vcs;
  int _inputs;
  int _outputs;
  bool _active;
  
  vector<FlitChannel *> _input_channels;
  vector<FlitChannel *> _output_channels;
  
  vector<Buffer *> _buf;
  map<int, Flit *> _in_queue_flits;
  
  vector<queue<Flit *> > _output_buffer;
 // tRoutingFunction   _rf;
    
  

  vector<Flit *> _output_channels_last_flit;
  
  Allocator *_vc_allocator;
  Allocator *_sw_allocator;
  
  deque<pair<int, pair<pair<int, int>, int> > > _vc_alloc_vcs;  
  deque<pair<int, pair<pair<int, int>, int> > > _sw_alloc_vcs;
  
  deque<pair<int, pair<pair<int, int>, int> > > _vc_alloc_vcs_tmp; 
  deque<pair<int, pair<pair<int, int>, int> > > _sw_alloc_vcs_tmp;  
  
  StatsNoC stats;
 // Crossbar  *m_CrossbarTable;
 // int _output_buffer_size;
 // vector<queue<Flit *> > _output_buffer;

  // For Energy
    SWAllocator _swa;
    VCAllocator _vca;
    InputBuffer _inb;
    CrossBar    _xbar;

  bool _ReceiveFlits( );
  void clk(int global_clk);
  void _InputQueuing(int global_clk);
  void ReadInputs(int global_clk);
  void AddInputChannel( FlitChannel *channel  );
  void AddOutputChannel( FlitChannel *channel  );
  void _InternalStep( );
  void _VCAllocEvaluate( );
  void _SWAllocEvaluate( );
  void _SendFlits( );
  void From8bitINT(uint8_t c, bool b[8]);
  void From32bitINT(uint32_t c, bool b[32]);
  void  Get_Bits_of_Flits(Flit* flit, bool  flit_bits[34]);
  void  Bus_Invert_coding(Flit* flit, bool  cur_flit[34], bool last_flit[34] );
  void  Calc_Flips(int signaling,int encoding, Flit* cur, Flit* last);  
  double GetStaticEnergy();
  double GetDynamicEnergy();  
  virtual void WriteOutputs(int global_clk);
  int FindIdleVC(int out_port, int dst_id);
  bool FindFreeSpace(int out_port, int vc_number);  
  void SetIdleVC(int out_port, int vc_number );

public:

 
  inline int GetID( ) const {return _id;}
 
  static  Router* NewRouter(Module *parent, const string & name, int id,int inputs, int outputs);
  Router(Module *parent, const string & name, int id,int inputs, int outputs);
  
};

#endif
