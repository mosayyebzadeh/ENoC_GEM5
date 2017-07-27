
#ifndef __ENOC_DIR_HH__
#define __ENOC_DIR_HH__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "mem/packet.hh"
#include "ENoC_cache.h"
#include "trafficmanager.h"
#include "NoCMain.h"

class TrafficManager;
class ENoCCache;

class ENoCDir
{
	private:
	public:
		vector<vector <int> > lines;
		
		void invalidatePacket(PacketPtr pkt, ENoCCache **L1DCache, ENoCCache **L1ICache, ENoCCache **L2Cache, int time, TrafficManager *tm, int dir_index);
		void receivePacket(PacketPtr pkt, ENoCCache **L1DCache, ENoCCache **L1ICache, ENoCCache **L2Cache, int time, TrafficManager *tm, int dir_index);
		
		void evictPacket(PacketPtr pkt);

		ENoCDir(ENoCCache **L1DCache, ENoCCache **L1ICache, ENoCCache **L2Cache);

		~ENoCDir();

		int *L1DLines;
		int *L1ILines;
		int *L2Lines;

		int *L1DWays;
		int *L1IWays;
		int *L2Ways;
		
};

#endif
