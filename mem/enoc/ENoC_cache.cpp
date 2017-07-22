
#include "ENoC_cache.h"

ENoCCache::ENoCCache(int lines, int ways)
{
	LINES = lines-1;
	WAYS = ways-1;

/*
	Cache = new (cacheline*)[lines];
	for (int i = 0; i < lines; i++) {
     Cache[i] = new cacheline[ways];
	}
*/

	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
	Cache = new cacheline*[lines];
	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
	for (int i = 0; i < lines; i++) {
     Cache[i] = new cacheline[ways];
	}
	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
}


ENoCCache::~ENoCCache()
{
	for (int i = 0; i < LINES; i++) {
     delete[] Cache[i];
	}
	delete[] Cache;
}

// step through the cache and set initial values
// LRU bit will be the same value as the way
// MESI bit begins invalid (empty)
// Tag bits set to null because access decisions based on MESI

void ENoCCache::setLRUbitsToWay()
{
  int index;
  int way;
  for (index = 0; index < LINES; index++)
  {
    for (way = 0; way < WAYS; way++)
    {
       Cache[index][way].LRUbits = way;
       Cache[index][way].MESIbits = 3;
       Cache[index][way].tag = 0;
    }
  }
}

// check every way in the given index to see if the given tag exists
// tag in each way of the appropriate index
int ENoCCache::checkTag(int index, int tag)
{
  int way;
	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
  for (way = 0; way < WAYS; way++)
  {
	
	printf("AMIN TEST: %s : %d: index: %d, way: %d\n", __func__, __LINE__, index, way);
	if (index >= 8192)
		index = 1000;

    if (Cache[index][way].tag == tag)
       return way;
	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
  } 
	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
  return WAYS;
}

// given an index, this function returns the least recently used way
// LRU bits of 0 give the least recently used way
// LRU bits of MAXWAY give most recently used way
int ENoCCache::checkLRU(int index)
{
  int way;
  for (way = 0; way < WAYS; way++) 
  {
    if (Cache[index][way].LRUbits == 0)
       return way;
  }
  return WAYS; 
}

// this function updates the LRU bits to reflect a new most recently used way
void ENoCCache::updateLRU(int index, int ourway)
{
  // if the LRU bits of our way are already the most recently used, we do nothing
  if (Cache[index][ourway].LRUbits == WAYS)
     return;
  else 
  {
     int ourbits = Cache[index][ourway].LRUbits;
     int testbits = ourbits++;
     int testway;
     for (testbits = 0; testbits < WAYS; testbits++)
     {
         testway = 0;
         while (testway < WAYS)
         {
            if (testbits == Cache[index][testway].LRUbits)
               {
                  Cache[index][testway].LRUbits--;
                  break;
               }
            else 
               testway++;
	 			 }  
     }
     Cache[index][ourway].LRUbits = WAYS;
  }
}

// This function takes in an index and tests all ways within that index,
// returning a 0 if it finds any valid way and a 1 if it does not.

int ENoCCache::testIndex(int index, int way)
{
   for (way = 0; way < WAYS; way++)
   {
       if (Cache[index][way].MESIbits != I)
       {
          return 0;
       }
   }
   return 1;
}

// The cache displays all indices containing at least one way with a 
// valid MESI bit

void ENoCCache::cacheDisplay()
{
/*
   int index;
   int way;
   ofp = fopen("display.txt", "a");

   for (index = 0; index <= MAXLINE; index++)
   {
      if (testIndex(index,way) == 0)
       {

           fprintf(ofp,"INDEX: 0x%-8x\n",index);
           fflush(ofp);

           for (way = 0; way <= MAXWAY; way++)
          {
             fprintf(ofp,"WAY %-8d LRU: %-4d MESI: %-10d TAG: %-8d"
                          " ADDR: 0x%-8x\n",
                          way,
                          Cache[index][way].LRUbits,
                          Cache[index][way].MESIbits,
                          Cache[index][way].tag,
                          Cache[index][way].address);
             fflush(ofp);
          }
      }
   }

   fprintf(ofp,"---------------------------------------------------------------------\n");
   fflush(ofp);
*/
}


/*
void ENoCCache::process()
{
	int tag = addr >> 20;

	int L1Dway = 0;
	int L1Iway = 0;
	int L2way = 0;

	int cmd = 1; //read


	PacketNoC packetNoC;

	//TODO: replace them with GEM5 versions
  int L1Dindex = (pkt->getAddr() >> 6) & L1DCache->LINES;
  int L1Iindex = ( pkt->getAddr() >> 6) & L1ICache->LINES;
  int L2index = ( pkt->getAddr() >> 6) & L2Cache->LINES;


	if (pkt->req->isInstFetch())
		cmd = 0;
	else if (pkt->isRead())
		cmd = 1;
	else if (pkt->isWrite())
		cmd = 2;
	else if (pkt->IsUpgrade())
		cmd = 3;
	else if (pkt->IsInvalidate 	())
		cmd = 4;
	else if (pkt->NeedsExclusive())
		cmd = 5;
	else if (pkt->IsRequest())
		cmd = 6;
	else if (pkt->IsResponse())
		cmd = 7;
	else if (pkt->NeedsResponse())
		cmd = 8;

	switch(cmd)
		case 0:
			L1Iway = L1ICache->checkTag(L1Iindex, tag);
      // if the tag exists
			if (L1Iway < L1ICache->WAYS)
			{
			  int MESI = L1ICache->Cache[L1Iindex][L1Iway].MESIbits;
	   		// if this tag exists and it's valid as per its MESI bits
	   		if (MESI == M || MESI == E || MESI == S)
	   		{
					printf("L1Ihit is %d\n", L1IhitCount);
	      	L1ICache->updateLRU(L1Iindex, L1Iway);
	      	L2Cache->updateLRU(L2index, L2way);
          // MESI remains unchanged
  	   	}
        // if this tag exists but it's been invalidated and can't be used...
        // we fetch from L2 and pass on to L1 cache, update to exclusive
	   		else 
        {
					printf("L1Imiss 1 is %d\n", L1ImissCount);

					ENoCData data = Directory->AskDirectory(pkt);

					packetNoC.src_id = ((int)pkt->req->masterId()) % 6;
					packetNoC.dst_id = DirectoreyID;
					packetNoC.time = (int) pkt->req->time();

					trafficManager->Req_owner(packetNoC, packetNoC.src_id);
					trafficManager->clk((int)curTick());



          L1ICache->updateLRU(L1Iindex, L1Iway);

					if (data.)
	      	L1ICache->Cache[L1Iindex][L1Iway].MESIbits = E;
          L2Cache->updateLRU(L2index, L2way);
	      	L2Cache->Cache[L2index][L2way].MESIbits = E;
	   		}
			}
        // this tag simply doesn't exist in the cache in any form
      else
			{
	   		L1ImissCount++;
				printf("L1Imiss 2 is %d\n", L1ImissCount);
					
				// TODO : send message to L2Cache


			  // use the LRU bits to determine which way to evict
	  		L1Iway = L1ICache->checkLRU(L1Iindex);
        L1ICache->updateLRU(L1Iindex, L1Iway);
        L1ICache->Cache[L1Iindex][L1Iway].tag = tag;
			  L1ICache->Cache[L1Iindex][L1Iway].MESIbits = E;
      }
      L1ICache->Cache[L1Iindex][L1Iway].address = addr;
			break;






	packetNoC.src_id = ((int)pkt->req->masterId()) % 6;
	packetNoC.dst_id = (pkt->getAddr()) >> 20;
	packetNoC.time = (int) pkt->req->time();
	packetNoC.data = *(pkt->getPtr<int>());

	trafficManager->load_traffic(packetNoC, packetNoC.src_id);
	trafficManager->clk((int)curTick());
}	
*/

void ENoCCache::sendSharedData(int src, int node, PacketPtr pkt, int time, TrafficManager *trafficmanager )
{
	int addr = pkt->getAddr();
	PacketNoC packetNoC;
	//int node = ((int)pkt->req->masterId())% 6;

	int tag = (pkt->getAddr()) >> 20;
	int Lway = 0;
	int Lindex = (pkt->getAddr() >> 6) & LINES;
	int type = 2;

		Lway = checkTag(Lindex, tag);

		if (Lway < WAYS)
		{
			Cache[Lindex][Lway].MESIbits = S;
  	  updateLRU(Lindex, Lway);
		}
		else
		{
		 		Lway = checkLRU(Lindex);
        updateLRU(Lindex, Lway);
        Cache[Lindex][Lway].tag = tag;
        Cache[Lindex][Lway].address = addr;
			  Cache[Lindex][Lway].MESIbits = S;
	
				//TODO: write data from i cache to node cache

		}
	 
		//message from directory to memory
		packetNoC.src_id = src;
		packetNoC.dst_id = node;
		packetNoC.time = time + 5;
		packetNoC.data = "abcdefg"; //TODO: send actual data
	
		type = 2; //memory
		trafficmanager->packet_table[src][type].push(packetNoC);	


}	















