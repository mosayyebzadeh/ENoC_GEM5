

#ifndef _TRAFFICMANAGER_H_
#define _TRAFFICMANAGER_H_

#include <list>
#include <map>
#include <set>
#include <cassert>
#include "module.h"
#include "Network.h"
#include "flit.h"
#include "routefunc.h"
#include "NoCMain.h"
#include "VirtualChannel.h"



class TrafficManager : public Module {

private:

  vector<vector<int> > _packet_size;
  vector<vector<int> > _packet_size_rate;
  vector<int> _packet_size_max_val;

protected:
  int _nodes;
  int _routers;
  int _vcs;

  NetworkNoC * _net;
  vector<Router *>  _router;
  
  
  	vector<queue < PacketNoC > > packet_table;
	vector<queue < PacketNoC > > packet_queue;	// Local queue of packets

	int packet_queue_length;
        
        vector<int> in_use_vc;


public:

  //static TrafficManager * New();
  static TrafficManager *New(NetworkNoC * net);

  TrafficManager(NetworkNoC *net);
  virtual ~TrafficManager( );

  bool load_traffic(PacketNoC packetNoC, int src);
  bool ReadPacket(PacketNoC & pk, int time , int src);
  Flit*  nextFlit(int src, int global_clk);
  void clk( int global_clk);
  unsigned int toInt(char c);
  bool IsNextFlitHead(int nodes);
  void SetIdleStateForVC(int nodes, int vc_number, VirtualChannel::VCState s);
  bool FindFreeSpace(int nodes, int vc_number);
  int FindAvailableVC(int nodes);
  int FindIdleVC(int nodes);  
  
};


#endif
