
#ifndef __ENOC_CACHE_HH__
#define __ENOC_CACHE_HH__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "trafficmanager.h"
#include "mem/packet.hh"
#include "NoCMain.h"


//TODO: move these to EnoCmain.h
#define MAXLINE 16482
#define MAXWAY 5

#define M 0
#define E 1
#define S 2 
#define I 3

#define DirectoryID 31

typedef struct cacheline
{
  int MESIbits;
  int LRUbits;
  int tag;
  int address;
  int data;
	bool dirty;
}cacheline;

typedef struct ENoCData
{
  int MESIbits;
  int address;
	int owner;
}ENoCData;

class TrafficManager;

class ENoCCache
{
	private:
	public:
		int LINES;
		int WAYS;
	
	
		cacheline **Cache;
		
		void setLRUbitsToWay();
		int checkTag(int index, int tag);
		int checkLRU(int index);
		void updateLRU(int index, int ourway);
		int testIndex(int index, int way);
		void cacheDisplay();


		void sendSharedData(int src, int dst, PacketPtr pkt, int time, TrafficManager *tm);

		ENoCCache(int lines, int ways);
		~ENoCCache();
		
};

#endif
