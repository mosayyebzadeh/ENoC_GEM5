

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

	packet_table.resize(_nodes);
	packet_queue.resize(_nodes);
        in_use_vc.resize(_nodes);

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

bool TrafficManager::load_traffic(PacketNoC pk, int src)
{
//	printf("AMIN TEST: %s: src_id is %d, pkt src_id is: %d\n", __func__, src, pk.src_id);
	packet_table[src].push(pk);
		
	//cout  << "traffic is loaded" << endl;
	return  true;
}

bool TrafficManager::ReadPacket(PacketNoC & pk, int time , int src)
{

	//Packet 
	if (packet_table[src].empty() == false)
	{
		pk = packet_table[src].front();


		if (time >= pk.time  && pk.src_id == src)
			packet_table[src].pop();
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


Flit*  TrafficManager::nextFlit(int src, int global_clk)
{
    
    Flit * flit  = Flit::New();
    
    static int seq =0;
    
    PacketNoC packet = packet_queue[src].front();

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
    packet_queue[src].front().flit_left--;
    if (packet_queue[src].front().flit_left == 0){
	packet_queue[src].pop();
	packet_queue_length--;
	}
    return flit;
}


void TrafficManager::clk( int global_clk)
{
    PacketNoC  pk;
    Flit  *  f;
	

	_net->clk(global_clk);
    
	//Send and Recieve Packets
    for(int nodes = 0; nodes < _nodes; nodes++)
    {
       bool readnewpk = ReadPacket(pk, global_clk, nodes);
       
       if (readnewpk == true)
            packet_queue[nodes].push( pk );
       
      if(packet_queue[nodes].empty() == false){

                if(IsNextFlitHead(nodes) == true){
                        int out_vc = FindIdleVC(nodes);
                        
                        if(out_vc != -1)
                        {
                                f = nextFlit(nodes, global_clk);
                                f->vc = out_vc;
                                _net->WriteFlit(f, nodes);
                                in_use_vc[nodes] = out_vc;
                                
                                int vc_number = out_vc;
                                SetIdleStateForVC( nodes,  vc_number, VirtualChannel::routing);
                                
                        }
                }
                else
                {
                        if( FindFreeSpace(nodes, in_use_vc[nodes]) == true )
                        {
                            
                                f = nextFlit(nodes, global_clk);
                                f->vc = in_use_vc[nodes];
                                if(f->flit_type == FLIT_TYPE_TAIL){
                                        in_use_vc[nodes] = -1;
                                }
                                _net->WriteFlit(f, nodes);
                        }
                        
                }                
                               
       }

          
    }

}


bool TrafficManager::IsNextFlitHead(int nodes)
{
	PacketNoC packet = packet_queue[nodes].front();
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
	 
	for ( int vc_number = 0; vc_number < _vcs; ++vc_number ){
		state = _ru[nodes]->_buf[4]->GetState( vc_number );
		
			if (state == VirtualChannel::idle){
				_ru[nodes]->_buf[4]->SetState( vc_number, VirtualChannel::routing );
				return vc_number;
			}
		}
		return -1;
}
