

#ifndef _TRAFFICMANAGER_H_
#define _TRAFFICMANAGER_H_

#include <list>
#include <map>
#include <set>
#include <cassert>
#include "module.h"
#include "Network.h"
#include "flit.h"
//#include "routefunc.h"
#include "NoCMain.h"
#include "VirtualChannel.h"
#include "ENoC_cache.h"
#include "ENoC_Dir.h"
#include "mem/packet.hh"

class ENoCDir;
class ENoCCache;

class TrafficManager : public Module {

	//friend class ENoCCache;
private:

  vector<vector<int> > _packet_size;
  vector<vector<int> > _packet_size_rate;
  vector<int> _packet_size_max_val;

protected:
  int _nodes;
  int _routers;
  int _vcs;

  NetworkNoC * _net;
  vector<ENoCRouter *>  _router;
  
	int packet_queue_length;
  vector<vector<int> > in_use_vc;                

public:
  
	ENoCCache **L1DCache;
	ENoCCache **L1ICache;
	ENoCCache **L2Cache;
	
	ENoCDir *Directory;

  vector<vector<queue < PacketNoC > > > packet_table;

	vector<vector<queue < PacketNoC > > > packet_queue;	// Local queue of packets


  //static TrafficManager * New();
  static TrafficManager *New( const Configuration &config, NetworkNoC * net);

  TrafficManager(NetworkNoC *net);
  virtual ~TrafficManager( );

  void process(PacketPtr pkt);
  bool load_traffic(PacketNoC packetNoC, int src, int type);
  bool ReadPacket(PacketNoC & pk, int time , int src , int type);
  Flit*  nextFlit(int src, int global_clk , int type);
  void clk( int global_clk);
  unsigned int toInt(char c);
  bool IsNextFlitHead(int nodes, int type);
  void SetIdleStateForVC(int nodes, int vc_number, VirtualChannel::VCState s);
  bool FindFreeSpace(int nodes, int vc_number);
  int FindAvailableVC(int nodes);
  int FindIdleVC(int nodes);  
  void SendFlit(int nodes, int global_clk, int type);
  
};


#endif
