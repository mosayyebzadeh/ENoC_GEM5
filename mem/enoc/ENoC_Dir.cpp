
#include "ENoC_Dir.h"


ENoCDir::ENoCDir(ENoCCache **L1DCache, ENoCCache **L1ICache, ENoCCache **L2Cache)
{
	//we consider one line for each core for thre begining.
	//we will increase the size of the directory over time
	//each cache line can have NUMBER_OF_NODES owners
	//in addition, first element holds the address of the memory location
	lines.resize(NUMBER_OF_NODES);
	for (int i = 0; i <  NUMBER_OF_NODES; i++)
		lines[i].resize(NUMBER_OF_NODES+1);

	L1DLines = new int[NUMBER_OF_NODES];
	L1ILines = new int[NUMBER_OF_NODES];
	L2Lines = new int[NUMBER_OF_NODES];

	L1DWays = new int[NUMBER_OF_NODES];
	L1IWays = new int[NUMBER_OF_NODES];
	L2Ways = new int[NUMBER_OF_NODES];

	for (int i= 0; i < NUMBER_OF_NODES; i++)
	{
		L1DLines[i] = L1DCache[i]->LINES;
		L1ILines[i] = L1ICache[i]->LINES;
		L2Lines[i] = L2Cache[i]->LINES;

		L1DWays[i] = L1DCache[i]->WAYS;
		L1IWays[i] = L1ICache[i]->WAYS;
		L2Ways[i] = L2Cache[i]->WAYS;
	}

}


ENoCDir::~ENoCDir()
{
}

void ENoCDir::invalidatePacket(PacketPtr pkt, ENoCCache **L1DCache, ENoCCache **L1ICache, ENoCCache **L2Cache, int time, TrafficManager *trafficmanager, int dir_index)
{
	int addr = pkt->getAddr();
	bool find = false; 
	int index = 0;
	PacketNoC packetNoC;
	int node = ((int)pkt->req->masterId())% 6;

	int tag = (pkt->getAddr()) >> 20;

	int L1Dway = 0;
	int L1Iway = 0;
	int L2way = 0;

	int L1Dindex = (pkt->getAddr() >> 6) & (L1DLines[node]-1);
  int L1Iindex = ( pkt->getAddr() >> 6) & (L1ILines[node]-1);
  int L2index = ( pkt->getAddr() >> 6) & (L2Lines[node]-1);

	int type = 2;

	for (int i = 0; i < lines.size(); i ++)
		if (lines[i][0] == addr)
		{
			find = true;
			index = i;
			break;
		}
	
	if (find == true)
	{
		for (int i = 1; i < NUMBER_OF_NODES+1; i ++)
		{
			if (lines[index][i] != I) //it is not invalid
			{
				L1Iway = L1ICache[i]->checkTag(L1Iindex, tag);
				L1Dway = L1DCache[i]->checkTag(L1Dindex, tag);
				L2way = L2Cache[i]->checkTag(L2index, tag);

				if (L1Dway < L1DCache[i]->WAYS)
					L1DCache[i]->Cache[L1Dindex][L1Dway].MESIbits = I;

				if (L1Iway < L1ICache[i]->WAYS)
					L1ICache[i]->Cache[L1Iindex][L1Iway].MESIbits = I;

				if (L2way < L2Cache[i]->WAYS)
					L2Cache[i]->Cache[L2index][L2way].MESIbits = I;

				if (dir_index == 0)
					packetNoC.src_id = DIRECTORY_ADDRESS_0;
				else if (dir_index == 1)
					packetNoC.src_id = DIRECTORY_ADDRESS_1;
				else if (dir_index == 2)
					packetNoC.src_id = DIRECTORY_ADDRESS_2;
				else if (dir_index == 3)
					packetNoC.src_id = DIRECTORY_ADDRESS_3;
				
				packetNoC.dst_id = i;
				packetNoC.time = time + 5;
	
				type = 2; //cache
				trafficmanager->packet_table[packetNoC.src_id][type].push(packetNoC);	


				lines[index][i] = I;
			}
		}
		lines[index][node] = E; //exclusive
	}
	
}

void ENoCDir::receivePacket(PacketPtr pkt, ENoCCache **L1DCache, ENoCCache **L1ICache, ENoCCache **L2Cache, int time, TrafficManager *trafficmanager, int dir_index)
{
	//this function will call a Cache function to update source cache
	//in addition, it will update directory's entry for that data

	int addr = pkt->getAddr();
	bool find = false; 
	int index = 0;
	PacketNoC packetNoC;
	int node = ((int)pkt->req->masterId())% 6;

	int tag = (pkt->getAddr()) >> 20;

	int L1Dway = 0;
	int L1Iway = 0;
	int L2way = 0;

	int L1Dindex = (pkt->getAddr() >> 6) & (L1DLines[node]-1);
  int L1Iindex = ( pkt->getAddr() >> 6) & (L1ILines[node]-1);
  int L2index = ( pkt->getAddr() >> 6) & (L2Lines[node]-1);

	int i = 0;
	int type = 2;

	//char TestData = "abcdefgh";

	for (int j = 0; j < lines.size(); j ++)
		if (lines[j][0] == addr)
		{
			find = true;
			index = j;
			break;
		}
	
	if (find == true)
	{
		find = false;
		for (i = 1; i < NUMBER_OF_NODES+1; i ++)
		{
			//the line is shared
			if (lines[index][i] == S)
			{
				lines[index][node] = S;
				find = true;
				break;
			}
			//if line is in M, it should be downgraded
			else if (lines[index][i] == M || lines[index][i] == E)
			{
				lines[index][node] = S;
				lines[index][i] = S;
				find = true;
				break;
			}
		}
	}
	else
	{
			lines[index][node] = E;
	}

	if (find == true)
	{

//	printf("AMIN TEST: %s : %d: before L1I\n", __func__, __LINE__);
		L1Iway = L1ICache[i]->checkTag(L1Iindex, tag);
//	printf("AMIN TEST: %s : %d: before L1D\n", __func__, __LINE__);
		L1Dway = L1DCache[i]->checkTag(L1Dindex, tag);
//	printf("AMIN TEST: %s : %d: before L2\n", __func__, __LINE__);
		L2way = L2Cache[i]->checkTag(L2index, tag);
//	printf("AMIN TEST: %s : %d: after L2\n", __func__, __LINE__);

		if (L1Dway < L1DCache[i]->WAYS)
		{
			L1DCache[i]->Cache[L1Dindex][L1Dway].MESIbits = S;
		}

		if (L1Iway < L1ICache[i]->WAYS)
		{
			L1ICache[i]->Cache[L1Iindex][L1Iway].MESIbits = S;
		}

		if (L2way < L2Cache[i]->WAYS)
		{
			L2Cache[i]->Cache[L2index][L2way].MESIbits = S;
			L2Cache[i]->sendSharedData(i, node, pkt, time + 5, trafficmanager); //L2 will send data to requesting core

			//message from directory to line holding cache
			if (dir_index == 0)
				packetNoC.src_id = DIRECTORY_ADDRESS_0;
			else if (dir_index == 1)
				packetNoC.src_id = DIRECTORY_ADDRESS_1;
			else if (dir_index == 2)
				packetNoC.src_id = DIRECTORY_ADDRESS_2;
			else if (dir_index == 3)
				packetNoC.src_id = DIRECTORY_ADDRESS_3;

			packetNoC.dst_id = i;
			packetNoC.time = time + 5;
	
			type = 2; //cache
			trafficmanager->packet_table[packetNoC.src_id][type].push(packetNoC);	

		}
		else
			find = false;
	}
	//nobody has the data, asking memory
	else if (find == false)
	{

//	printf("AMIN TEST: %s : %d: before L1I\n", __func__, __LINE__);
		L1Iway = L1ICache[node]->checkTag(L1Iindex, tag);
//	printf("AMIN TEST: %s : %d: before L1D\n", __func__, __LINE__);
		L1Dway = L1DCache[node]->checkTag(L1Dindex, tag);
//	printf("AMIN TEST: %s : %d: before L2\n", __func__, __LINE__);
		L2way = L2Cache[node]->checkTag(L2index, tag);
//	printf("AMIN TEST: %s : %d: after L2\n", __func__, __LINE__);

		if (pkt->req->isInstFetch())
		{
//	printf("AMIN TEST: %s : %d: after L2\n", __func__, __LINE__);
			if (L1Iway < L1ICache[node]->WAYS)
			{
//	printf("AMIN TEST: %s : %d: after L2\n", __func__, __LINE__);
				L1ICache[node]->Cache[L1Iindex][L1Iway].MESIbits = E;
        L1ICache[node]->updateLRU(L1Iindex, L1Iway);
//	printf("AMIN TEST: %s : %d: after L2\n", __func__, __LINE__);
			}
		}
		else
		{
//	printf("AMIN TEST: %s : %d: after L2\n", __func__, __LINE__);
			if (L1Dway < L1DCache[node]->WAYS)
			{
//	printf("AMIN TEST: %s : %d: after L2\n", __func__, __LINE__);
				L1DCache[node]->Cache[L1Dindex][L1Dway].MESIbits = E;
        L1DCache[node]->updateLRU(L1Dindex, L1Dway);
//	printf("AMIN TEST: %s : %d: after L2\n", __func__, __LINE__);
			}
		}
		// if L2 has the data
		if (L2way < L2Cache[node]->WAYS)
		{
//	printf("AMIN TEST: %s : %d: after L2\n", __func__, __LINE__);
			L2Cache[node]->Cache[L2index][L2way].MESIbits = E;
      L2Cache[node]->updateLRU(L2index, L2way);
//	printf("AMIN TEST: %s : %d: after L2\n", __func__, __LINE__);
		}
		else
		{
//	printf("AMIN TEST: %s : %d: after L2\n", __func__, __LINE__);
		 		L2way = L2Cache[node]->checkLRU(L2index);
        L2Cache[node]->updateLRU(L2index, L2way);
        L2Cache[node]->Cache[L2index][L2way].tag = tag;
        L2Cache[node]->Cache[L2index][L2way].address = pkt->getAddr();
			  L2Cache[node]->Cache[L2index][L2way].MESIbits = E;
	
//	printf("AMIN TEST: %s : %d: after L2\n", __func__, __LINE__);
				//TODO: write data from memory to cache

		}
	 
		//message from directory to memory
		if (dir_index == 0)
			packetNoC.src_id = DIRECTORY_ADDRESS_0;
		else if (dir_index == 1)
			packetNoC.src_id = DIRECTORY_ADDRESS_1;
		else if (dir_index == 2)
			packetNoC.src_id = DIRECTORY_ADDRESS_2;
		else if (dir_index == 3)
			packetNoC.src_id = DIRECTORY_ADDRESS_3;

		packetNoC.dst_id = (pkt->getAddr()) >> 26;
		packetNoC.time = time + 5;
	
		type = 1; //memory
		trafficmanager->packet_table[packetNoC.src_id][type].push(packetNoC);	

		//message from  memory to cache
		packetNoC.src_id = (pkt->getAddr()) >> 26;
		packetNoC.dst_id = node;
		packetNoC.time = time + 10;
		packetNoC.data = "abcdefg"; //TODO: send actual data
//	printf("AMIN TEST: %s : %d: address is: %ld\n", __func__, __LINE__, pkt->getAddr()>>26);

		type = 2; //cache
		trafficmanager->packet_table[(pkt->getAddr()) >> 26][type].push(packetNoC);	
//	printf("AMIN TEST: %s : %d: after L2\n", __func__, __LINE__);



	}

}




