

#include <sstream>
//#include <cmath>
#include <iostream>
#include <fstream>
//#include <limits>
#include <cstdlib>
//#include <ctime>

//#include "NoCMain.h"
#include "trafficmanager.h"
//#include "VirtualChannel.h"

TrafficManager * TrafficManager::New(NetworkNoC * net)
{
    TrafficManager * result = NULL;
    result = new TrafficManager( net);
    return result;

}

TrafficManager::TrafficManager(  NetworkNoC *  net )
    : Module( 0, "traffic_manager" ), _net(net)
{

    _nodes = _net->NumNodes( );
    _routers = _net->NumRouters( );

    //_vcs = GlobalParams::num_vcs;
    _vcs = DEFAULT_NUM_VCS;

    int _num_of_type  = 3;

	packet_table.resize(_nodes);
	packet_queue.resize(_nodes);
    in_use_vc.resize(_nodes);

	L1DCache = new ENoCCache*[NUMBER_OF_NODES]; 
	L1ICache = new ENoCCache*[NUMBER_OF_NODES]; 
	L2Cache = new ENoCCache*[NUMBER_OF_NODES]; 

    for(int i = 0; i < _nodes; i++){
			L1DCache[i] = new ENoCCache(8192, 4);
			L1ICache[i] = new ENoCCache(8192, 4);
			L2Cache[i] = new ENoCCache(16484, 8);

			L1DCache[i]->setLRUbitsToWay();
			L1ICache[i]->setLRUbitsToWay();
			L2Cache[i]->setLRUbitsToWay();

		}

    for(int i = 0; i < _nodes; i++){

		packet_table[i].resize(_num_of_type);
		packet_queue[i].resize(_num_of_type);
	    in_use_vc[i].resize(_num_of_type);

    }

		//TODO: directory		
		Directory = new ENoCDir(L1DCache, L1ICache, L2Cache);

        _router = _net->GetRouters();

}

TrafficManager::~TrafficManager( )
{
//	printf("\n\n\n\nAMIN TEST: distrcutor %s\n\n\n", __func__);
}

unsigned int TrafficManager::toInt(char c)
{
  if (c >= '0' && c <= '9') return      c - '0';
  if (c >= 'A' && c <= 'F') return 10 + c - 'A';
  if (c >= 'a' && c <= 'f') return 10 + c - 'a';
  return -1;
}

void TrafficManager::process(PacketPtr pkt)
{
	int node = ((int)pkt->req->masterId())% 6;
	int type = 2; //cache
	int tag = (pkt->getAddr()) >> 20;
	int addr = pkt->getAddr();

	int L1Dway = 0;
	int L1Iway = 0;
	int L2way = 0;

	int cmd = 1; //read
	int MESI = 0;

	int L1Dindex = (pkt->getAddr() >> 6) & (L1DCache[node]->LINES-1);
  int L1Iindex = ( pkt->getAddr() >> 6) & (L1ICache[node]->LINES-1);
  int L2index = ( pkt->getAddr() >> 6) & (L2Cache[node]->LINES-1);

	int Dir_index = 0;

//	printf("AMIN TEST: %s : %d: ADDRESS is %ld\n", __func__, __LINE__, pkt->getAddr());

	PacketNoC packetNoC;

	if (pkt->req->isInstFetch())
		cmd = 0;
	else if (pkt->isRead())
		cmd = 1;
	else if (pkt->isWrite())
		cmd = 2;
	else if (pkt->isUpgrade())
		cmd = 3;
	else if (pkt->isInvalidate())
		cmd = 4;
	//else if (pkt->needsExclusive())
	//	cmd = 5;
	else if (pkt->isRequest())
		cmd = 6;
	else if (pkt->isResponse())
		cmd = 7;
	else if (pkt->needsResponse())
		cmd = 8;

	//printf("AMIN TEST: %s : %d\n", __func__, __LINE__);

	switch(cmd)
	{
		case 0:
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
			L1Iway = L1ICache[node]->checkTag(L1Iindex, tag);
      // if the tag exists
			if (L1Iway < L1ICache[node]->WAYS)
			{
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
			  MESI = L1ICache[node]->Cache[L1Iindex][L1Iway].MESIbits;
	   		// if this tag exists and it's valid as per its MESI bits
	   		if (MESI == M || MESI == E || MESI == S)
	   		{
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
	      	L1ICache[node]->updateLRU(L1Iindex, L1Iway);
					//TODO: send data to GEM5
  	   	}
	   		// data is invalidated in L1, checking L2
				else 
        {
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
					L2way = L2Cache[node]->checkTag(L2index, tag);
					if (L2way < L2Cache[node]->WAYS)
					{
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
						//data in L2. update L1 and L2
			  		MESI = L2Cache[node]->Cache[L2index][L2way].MESIbits;
	  		 		if (MESI == M || MESI == E || MESI == S)
	   				{
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
							//printf("L1Ihit is %d\n", L2hitCount);
	  		    	L2Cache[node]->updateLRU(L2index, L2way);
	      			L1ICache[node]->updateLRU(L1Iindex, L1Iway);
			      	L1ICache[node]->Cache[L1Iindex][L1Iway].address = addr;
	      			L1ICache[node]->Cache[L1Iindex][L1Iway].MESIbits = L2Cache[node]->Cache[L2index][L2way].MESIbits;
							//TODO: send data to GEM5
		  	   	}
						//Tag is in L2 but invaidated 
						//asking directory. The destination node will update source's cache
						else
						{
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);

							packetNoC.src_id = node;

							Dir_index = node/16;
							Directory->receivePacket(pkt, L1DCache, L1ICache, L2Cache, (int)pkt->req->time(), this, Dir_index);
	
							if (node < 16)
								packetNoC.dst_id = DIRECTORY_ADDRESS_0;
							else if (node < 32)
								packetNoC.dst_id = DIRECTORY_ADDRESS_1;
							else if (node < 48)
								packetNoC.dst_id = DIRECTORY_ADDRESS_2;
							else if (node < 64)
								packetNoC.dst_id = DIRECTORY_ADDRESS_3;



							packetNoC.time = (int) pkt->req->time();
							packetNoC.data = *(pkt->getPtr<int>());
	
							type = 0; //directory
							packet_table[node][type].push(packetNoC);	
						
						}	
					}
					//it is not in L2 in any form 
					//asking dirctory
					else
					{
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);

							packetNoC.src_id = node;
							//packetNoC.dst_id = DIRECTORY_ADDRESS;

							Dir_index = node/16;
							Directory->receivePacket(pkt, L1DCache, L1ICache, L2Cache, (int)pkt->req->time(), this, Dir_index);

							if (node < 16)
								packetNoC.dst_id = DIRECTORY_ADDRESS_0;
							else if (node < 32)
								packetNoC.dst_id = DIRECTORY_ADDRESS_1;
							else if (node < 48)
								packetNoC.dst_id = DIRECTORY_ADDRESS_2;
							else if (node < 64)
								packetNoC.dst_id = DIRECTORY_ADDRESS_3;


							packetNoC.time = (int) pkt->req->time();
							//packetNoC.data = *(pkt->getptr<int>());
	
							type = 0; //directory
							packet_table[node][type].push(packetNoC);	
	
					}
				}
			}
      // this tag simply doesn't exist in the cache in any form
      // in L1 and L2
      else
			{
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);

				packetNoC.src_id = node;

				Dir_index = node/16;
				Directory->receivePacket(pkt, L1DCache, L1ICache, L2Cache, (int)pkt->req->time(), this, Dir_index);

				if (node < 16)
					packetNoC.dst_id = DIRECTORY_ADDRESS_0;
				else if (node < 32)
					packetNoC.dst_id = DIRECTORY_ADDRESS_1;
				else if (node < 48)
					packetNoC.dst_id = DIRECTORY_ADDRESS_2;
				else if (node < 64)
					packetNoC.dst_id = DIRECTORY_ADDRESS_3;


				packetNoC.time = (int) pkt->req->time();
				//packetNoC.data = *(pkt->getptr<int>());
	
				type = 0; //directory
				packet_table[node][type].push(packetNoC);	
	
			}
			break;
		case 1:
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
			L1Dway = L1DCache[node]->checkTag(L1Dindex, tag);
      // if the tag exists
			if (L1Dway < L1DCache[node]->WAYS)
			{
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
			  MESI = L1DCache[node]->Cache[L1Dindex][L1Dway].MESIbits;
	   		// if this tag exists and it's valid as per its MESI bits
	   		if (MESI == M || MESI == E || MESI == S)
	   		{
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
	      	L1DCache[node]->updateLRU(L1Dindex, L1Dway);
					//TODO: send data to GEM5
  	   	}
	   		// data is invalidated in L1, checking L2
				else 
        {
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
					L2way = L2Cache[node]->checkTag(L2index, tag);
					if (L2way < L2Cache[node]->WAYS)
					{
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
						//data in L2. update L1 and L2
			  		MESI = L2Cache[node]->Cache[L2index][L2way].MESIbits;
	  		 		if (MESI == M || MESI == E || MESI == S)
	   				{
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
							//printf("L1Dhit is %d\n", L2hitCount);
	  		    	L2Cache[node]->updateLRU(L2index, L2way);
	      			L1DCache[node]->updateLRU(L1Dindex, L1Dway);
			      	L1DCache[node]->Cache[L1Dindex][L1Dway].address = addr;
	      			L1DCache[node]->Cache[L1Dindex][L1Dway].MESIbits = L2Cache[node]->Cache[L2index][L2way].MESIbits;
							//TODO: send data to GEM5
		  	   	}
						//Tag is in L2 but invaidated 
						//asking directory. The destination node will update source's cache
						else
						{

							packetNoC.src_id = node;

							Dir_index = node/16;
							Directory->receivePacket(pkt, L1DCache, L1ICache, L2Cache, (int)pkt->req->time(), this, Dir_index);

							if (node < 16)
								packetNoC.dst_id = DIRECTORY_ADDRESS_0;
							else if (node < 32)
								packetNoC.dst_id = DIRECTORY_ADDRESS_1;
							else if (node < 48)
								packetNoC.dst_id = DIRECTORY_ADDRESS_2;
							else if (node < 64)
								packetNoC.dst_id = DIRECTORY_ADDRESS_3;


							packetNoC.time = (int) pkt->req->time();
							packetNoC.data = *(pkt->getPtr<int>());
	
							type = 0; //directory
							packet_table[node][type].push(packetNoC);	
						
						}	
					}
					//it is not in L2 in any form 
					//asking dirctory
					else
					{
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);

							packetNoC.src_id = node;

							Dir_index = node/16;
							Directory->receivePacket(pkt, L1DCache, L1ICache, L2Cache, (int)pkt->req->time(), this, Dir_index);

							if (node < 16)
								packetNoC.dst_id = DIRECTORY_ADDRESS_0;
							else if (node < 32)
								packetNoC.dst_id = DIRECTORY_ADDRESS_1;
							else if (node < 48)
								packetNoC.dst_id = DIRECTORY_ADDRESS_2;
							else if (node < 64)
								packetNoC.dst_id = DIRECTORY_ADDRESS_3;


							packetNoC.time = (int) pkt->req->time();
							//packetNoC.data = *(pkt->getptr<int>());
	
							type = 0; //directory
							packet_table[node][type].push(packetNoC);	
	
					}
				}
			}
      // this tag simply doesn't exist in the cache in any form
      // in L1 and L2
      else
			{
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);

				packetNoC.src_id = node;

				Dir_index = node/16;
				Directory->receivePacket(pkt, L1DCache, L1ICache, L2Cache, (int)pkt->req->time(), this, Dir_index);

				if (node < 16)
					packetNoC.dst_id = DIRECTORY_ADDRESS_0;
				else if (node < 32)
					packetNoC.dst_id = DIRECTORY_ADDRESS_1;
				else if (node < 48)
					packetNoC.dst_id = DIRECTORY_ADDRESS_2;
				else if (node < 64)
					packetNoC.dst_id = DIRECTORY_ADDRESS_3;


				packetNoC.time = (int) pkt->req->time();
				//packetNoC.data = *(pkt->getptr<int>());
	
				type = 0; //directory
				packet_table[node][type].push(packetNoC);	
	
			}
			break;
		case 2:
		case 3:
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
			L1Dway = L1DCache[node]->checkTag(L1Dindex, tag);
      // if the tag exists
			if (L1Dway < L1DCache[node]->WAYS)
			{
			}
      // this tag simply doesn't exist in the cache in any form
      // in L1 and L2
      else
			{
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
				L1Dway = L1DCache[node]->checkLRU(L1Dindex);
       	L1DCache[node]->Cache[L1Dindex][L1Dway].tag = tag;
			}

//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
			//ask directory to invalidate others

			packetNoC.src_id = node;

			Dir_index = node/16;
			Directory->receivePacket(pkt, L1DCache, L1ICache, L2Cache, (int)pkt->req->time(), this, Dir_index);

			if (node < 16)
				packetNoC.dst_id = DIRECTORY_ADDRESS_0;
			else if (node < 32)
				packetNoC.dst_id = DIRECTORY_ADDRESS_1;
			else if (node < 48)
				packetNoC.dst_id = DIRECTORY_ADDRESS_2;
			else if (node < 64)
				packetNoC.dst_id = DIRECTORY_ADDRESS_3;

			packetNoC.time = (int) pkt->req->time();
			packetNoC.data = *(pkt->getPtr<int>());
	
			type = 0; //directory
			packet_table[node][type].push(packetNoC);	

	   	L1DCache[node]->updateLRU(L1Dindex, L1Dway);
		 	L1DCache[node]->Cache[L1Dindex][L1Dway].MESIbits = M;
		  L1DCache[node]->Cache[L1Dindex][L1Dway].address = addr;

			//updating L2
			L2way = L2Cache[node]->checkTag(L2index, tag);
			if (L2way < L2Cache[node]->WAYS)
			{
			}
			else
			{
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
				L2way = L2Cache[node]->checkLRU(L2index);
     		L2Cache[node]->Cache[L2index][L2way].tag = tag;
			}
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
	    L2Cache[node]->updateLRU(L2index, L2way);
		  L2Cache[node]->Cache[L2index][L2way].address = addr;
	   	L2Cache[node]->Cache[L2index][L2way].MESIbits = M;

			break;
		case 4:
		case 5:
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
			L1Dway = L1DCache[node]->checkTag(L1Dindex, tag);
      // if the tag exists
			if (L1Dway < L1DCache[node]->WAYS)
			{
			}
      // this tag simply doesn't exist in the cache in any form
      // in L1 and L2
      else
			{
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
				L1Dway = L1DCache[node]->checkLRU(L1Dindex);
       	L1DCache[node]->Cache[L1Dindex][L1Dway].tag = tag;
			}

//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
			//ask directory to invalidate others

			packetNoC.src_id = node;

			Dir_index = node/16;
			Directory->receivePacket(pkt, L1DCache, L1ICache, L2Cache, (int)pkt->req->time(), this, Dir_index);

			if (node < 16)
				packetNoC.dst_id = DIRECTORY_ADDRESS_0;
			else if (node < 32)
				packetNoC.dst_id = DIRECTORY_ADDRESS_1;
			else if (node < 48)
				packetNoC.dst_id = DIRECTORY_ADDRESS_2;
			else if (node < 64)
				packetNoC.dst_id = DIRECTORY_ADDRESS_3;


			packetNoC.time = (int) pkt->req->time();
			packetNoC.data = *(pkt->getPtr<int>());
	
			type = 0; //directory
			packet_table[node][type].push(packetNoC);	

	   	L1DCache[node]->updateLRU(L1Dindex, L1Dway);
		 	L1DCache[node]->Cache[L1Dindex][L1Dway].MESIbits = E;
		  L1DCache[node]->Cache[L1Dindex][L1Dway].address = addr;

			//updating L2
			L2way = L2Cache[node]->checkTag(L2index, tag);
			if (L2way < L2Cache[node]->WAYS)
			{
			}
			else
			{
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
				L2way = L2Cache[node]->checkLRU(L2index);
     		L2Cache[node]->Cache[L2index][L2way].tag = tag;
			}
//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);
	    L2Cache[node]->updateLRU(L2index, L2way);
		  L2Cache[node]->Cache[L2index][L2way].address = addr;
	   	L2Cache[node]->Cache[L2index][L2way].MESIbits = E;

			break;

		case 6:
		case 7:
		case 8:
			break;

	}



//	printf("AMIN TEST: %s : %d\n", __func__, __LINE__);

				// TODO: the code for updating source cache
				/*
				L1Iway = L1ICache->checkLRU(L1Iindex);
        L1ICache->updateLRU(L1Iindex, L1Iway);
        L1ICache->Cache[L1Iindex][L1Iway].tag = tag;
      	L1ICache->Cache[L1Iindex][L1Iway].address = addr;
			  L1ICache->Cache[L1Iindex][L1Iway].MESIbits = E;
				*/

	//TODO: create packet codes
	/*
	packetNoC.src_id = ((int)pkt->req->masterId()) % 6;
	packetNoC.dst_id = (pkt->getAddr()) >> 20;
	packetNoC.time = (int) pkt->req->time();
	packetNoC.data = *(pkt->getPtr<int>());
	*/

}


bool TrafficManager::load_traffic(PacketNoC pk, int src, int type)
{
//	printf("AMIN TEST: %s: src_id is %d, pkt src_id is: %d\n", __func__, src, pk.src_id);
	

	packet_table[src][type].push(pk);	
		
		
	//cout  << "traffic is loaded" << endl;
	return  true;
}

bool TrafficManager::ReadPacket(PacketNoC & pk, int time , int src, int type)
{

	//Packet 
	if (packet_table[src][type].empty() == false)
	{
		pk = packet_table[src][type].front();


		if (time >= pk.time  && pk.src_id == src)
			packet_table[src][type].pop();
		else
			return false;


		int size_of_packet;

		if (pk.data.length() == 1)
			size_of_packet = 2;//DEFAULT_MIN_PACKET_SIZE;
		else
			size_of_packet = 18;//DEFAULT_MAX_PACKET_SIZE;

		pk.size = pk.flit_left = size_of_packet;

		return true;
	}
	else
		return false;
}


Flit*  TrafficManager::nextFlit(int src, int global_clk, int type)
{
    
    Flit * flit  = Flit::New();
    
    static int seq =0;
    
    PacketNoC packet = packet_queue[src][type].front();

    flit->src_id      = packet.src_id;
    flit->dst_id      = packet.dst_id;


//masoud

	flit->flit_is_inverted = false;

	flit->flipNumber = 0;  				//set flip number of a flit to zero
	std::stringstream str;

	string s1  = packet.data;
	string s2;

	int flit_size  ; 
	int flit_left  = packet.flit_left;


    if (packet.size == packet.flit_left)
    {
	flit->flit_type = FLIT_TYPE_HEAD;
        seq = 0;
    }
    else if (packet.flit_left == 1)
    {
	flit->flit_type = FLIT_TYPE_TAIL;
        seq++;
    }
    else
    {
	flit->flit_type = FLIT_TYPE_BODY;
        seq++;


			flit_size = 18 - flit_left -1; //DEFAULT_MAX_PACKET_SIZE
			for (int j = 0; j <  8; j++)
			{
			 s2.push_back(s1[flit_size*8+j]);

			}
			uint8_t  output[4];

			for (size_t i = 0; i < 4; i++)
			{
			  output[i] = 16 * toInt(s2[2*i]) + toInt(s2[2*i+1]);
			
			}


              uint32_t u;
                u = output[0];
                u = (u  << 8) + output[1];
                u = (u  << 8) + output[2];
                u = (u  << 8) + output[3];
			
			flit->payload = u;
            
            
			s2.clear();
			str.clear();



    }


    flit->sequence_no = seq;
    // AMIN_ST 
    flit->timestamp = global_clk;
    //flit->timestamp = DEFAULT_GLOBAL_CLK;
    packet_queue[src][type].front().flit_left--;
    if (packet_queue[src][type].front().flit_left == 0){
	packet_queue[src][type].pop();
	packet_queue_length--;
	}
    return flit;
}


void TrafficManager::clk( int global_clk)
{
    PacketNoC  pk;
    //Flit  *  f;
	int _num_of_type  = 3;

	_net->clk(global_clk);
    
	//Send and Recieve Packets
    for(int nodes = 0; nodes < _nodes; nodes++)
    {

    	for(int types = 0; types < _num_of_type; types++)
    	{
		       bool readnewpk = ReadPacket(pk, global_clk, nodes, types);
		       
		       if (readnewpk == true)
		            packet_queue[nodes][types].push( pk );
		       
		      if(packet_queue[nodes][types].empty() == false){

		             SendFlit( nodes,  global_clk , types);
		                               
		       }

          
    	}
    }

}


bool TrafficManager::IsNextFlitHead(int nodes, int type)
{
	PacketNoC packet = packet_queue[nodes][type].front();
	if (packet.size == packet.flit_left)
		return true;
	else
		return false;
}
void TrafficManager::SetIdleStateForVC(int nodes, int vc_number, VirtualChannel::VCState s)
{
		vector<Router *> _ru = _net->GetRouters();//;
		_ru[nodes]->_buf[4]->SetState( vc_number, s );
}
bool TrafficManager::FindFreeSpace(int nodes, int vc_number)
{
	if( vc_number != -1)
	{
		vector<Router *> _ru = _net->GetRouters();//;
		int size = _ru[nodes]->_buf[4]->GetOccupancy( vc_number );
		
		//if(size < GlobalParams::vcs_size)
		if(size < DEFAULT_VCS_SIZE)
			return true;
		else
			return false;
	}
	else
		return false;
	
}
int TrafficManager::FindAvailableVC(int nodes)
{
	vector<Router *> _ru = _net->GetRouters();//;
	int out_vc = -1;
	int occup_tmp = _ru[nodes]->_buf[4]->GetOccupancy(0);
	int occup;
	for ( int vc_number = 1; vc_number < _vcs; ++vc_number ){
		occup = _ru[nodes]->_buf[4]->GetOccupancy( vc_number );
		if (occup <= occup_tmp)
		{
			out_vc = vc_number;
			occup_tmp = occup;
		}
	}
		return out_vc;
					
}

int TrafficManager::FindIdleVC(int nodes)
{
	vector<Router *> _ru = _net->GetRouters();//;
	 
	VirtualChannel::VCState state ;
	 
	for ( int vc_number = 2; vc_number < _vcs; ++vc_number ){
		state = _ru[nodes]->_buf[4]->GetState( vc_number );
		
			if (state == VirtualChannel::idle){
				_ru[nodes]->_buf[4]->SetState( vc_number, VirtualChannel::routing );
				return vc_number;
			}
		}
		return -1;
}

void TrafficManager::SendFlit(int nodes, int global_clk, int type)
{

	Flit  *  f;
    if(IsNextFlitHead(nodes,type) == true){
            int out_vc = -1;

            switch(type)
            {
            	case 0:
            		out_vc = 0;
            	break;

            	case 1:
            		out_vc = 1;
            	break;
            		 
            	case 2:
            		out_vc = FindIdleVC(nodes);
            	break;

            }


            if(out_vc != -1)
            {
                    f = nextFlit(nodes, global_clk, type);
                    f->vc = out_vc;
                    _net->WriteFlit(f, nodes);
                    in_use_vc[nodes][type] = out_vc;                                
                    int vc_number = out_vc;
                    SetIdleStateForVC( nodes,  vc_number, VirtualChannel::routing);
                    
            }
    }
    else
    {
            if( FindFreeSpace(nodes, in_use_vc[nodes][type]) == true )
            {
                
                    f = nextFlit(nodes, global_clk, type);
                    f->vc = in_use_vc[nodes][type];
                    if(f->flit_type == FLIT_TYPE_TAIL){
                            in_use_vc[nodes][type] = -1;
                    }
                    _net->WriteFlit(f, nodes);
            }
            
    }   

}
