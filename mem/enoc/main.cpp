
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
//#include <vector>
#include "NoCMain.h"
#include "Network.h"
//#include "CmdLineParser.h"
#include "routefunc.h"
#include "trafficmanager.h"
#include "globalstats.h"

using namespace std;

char       GlobalParams::traffic_table_filename[128]   = DEFAULT_TRAFFIC_TABLE_FILENAME;

int        GlobalParams::global_clk 				   = DEFAULT_GLOBAL_CLK;
int        GlobalParams::mesh_dim_x 				   = DEFAULT_MESH_DIM_X;
int        GlobalParams::mesh_dim_y 				   = DEFAULT_MESH_DIM_Y;
int        GlobalParams::num_vcs 				   = DEFAULT_NUM_VCS;
int        GlobalParams::vcs_size 				   = DEFAULT_VCS_SIZE;
bool       GlobalParams::detailed                                  = DEFAULT_DETAILED;
int        GlobalParams::signaling 				   = SIGNALING_LEVEL;
int        GlobalParams::encoding 				   = ENCODING_BINARY;

TrafficManager * trafficManager = NULL;
//extern std::ostream * gWatchOut;

int GetSimTime() {
  return GlobalParams::global_clk;
}

bool parseCmdLine(int arg_num, char *arg_vet[])
{
    if (arg_num == 1)
		cout << "Running with default parameters (use '-help' option to see how to override them)" << endl;
    else 
    {
		for (int i = 1; i < arg_num; i++) 
		{
		    if (!strcmp(arg_vet[i], "-help")) 
		    {
				//	showHelp(arg_vet[0]);
				return false;
		    } 

		    else if (!strcmp(arg_vet[i], "-traffic"))
		    {

				    strcpy(GlobalParams::traffic_table_filename,  arg_vet[++i]);
				    //strcpy(GlobalParams::signaling,  arg_vet[i+2]);
				    //strcpy(GlobalParams::encoding,   arg_vet[i+3]);
				    return true;
		    } 

			else {
				cout << "Error: Invalid option: " << arg_vet[i] << endl;
				return false;
		    }
		
	    }
	}
}

int main( int argc, char **argv )
{
  
  //  InitializeRoutingMap( );
    
    
  if(parseCmdLine(argc, argv) == true)
  {
          ostringstream name;
          name << "network_0";
      	  Network * net = new Network(name.str());
          
          trafficManager = TrafficManager::New( net ) ;

	//  net->buildMesh();
	bool loaded_traffic =   trafficManager->load_traffic(GlobalParams::traffic_table_filename);

	  // warm-up ...

	//  GlobalParams::global_clk = 0;

	  for ( GlobalParams::global_clk = 0; GlobalParams::global_clk < _total_sims; GlobalParams::global_clk++ ) 
	  {
                trafficManager->clk(GlobalParams::global_clk);
	  	
		  
	  }
	  
	  GlobalStats gs(net);
          gs.showStats(std::cout );

  }

}