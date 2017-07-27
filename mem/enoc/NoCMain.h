
#ifndef __NOCMMAIN_H__
#define __NOCMMAIN_H__

#include <string>
#include <string.h>
#include <iostream>
#include <stdint.h>
using namespace std;


#define NUMBER_OF_NODES	64
#define DIRECTORY_ADDRESS_0 8
#define DIRECTORY_ADDRESS_1 24
#define DIRECTORY_ADDRESS_2 40
#define DIRECTORY_ADDRESS_3 56

#define MAX_STATIC_DIM				 10
#define DIRECTION_NORTH              0
#define DIRECTION_EAST               1
#define DIRECTION_SOUTH              2
#define DIRECTION_WEST               3



#define DIRECTIONS                   5
#define __CACHE_LINES_BYTE          64
//#define MAX_STATIC_DIM               20

#define	_total_sims_def			78000


#define DEFAULT_TRAFFIC_TABLE_FILENAME           ""
#define DEFAULT_OUTPUT_FILENAME           ""
#define DEFAULT_CONFIG_FILENAME           "config.txt"

#define DEFAULT_GLOBAL_CLK			 0
#define DEFAULT_MESH_DIM_X                       8
//#define DEFAULT_MESH_DIM_Y                       8

#define DEFAULT_NUM_VCS		 		8
#define DEFAULT_VCS_SIZE	 		8
#define DEFAULT_DETAILED                        false

#define CYCLE_TIME                      0.25*1e-9  // 4GHz

    

#define BIT_TRANSITION_DYNAMIC_ENERGY                 1e-12 //15fj

#define SWITCHING_ACTIVITY                            0.25  

//#define SCALING_FACTOR_45n_TO_32nm_LATENCY            0.91  
//#define SCALING_FACTOR_45n_TO_32nm_ENERGY             0.66  
//#define SCALING_FACTOR_45n_TO_32nm_POWER              0.66


#define SIGNALING_LEVEL                            0
#define SIGNALING_TRANSITION                       1 

#define ENCODING_BINARY                            0
#define ENCODING_BUS_INVERT                        1 

//typedef char uint8_t;


struct GlobalParams {
	//static int                          mesh_dim_x;
	//static int                          mesh_dim_y;
//	static int                          mesh_dim_z;
	static char                         traffic_table_filename[128];
    static char                         output_filename[128];
    static char                         config_filename[128];

	static int			    global_clk;
	//static int			    num_vcs;	
	//static int			    vcs_size;
    static bool                         detailed;
    //static int                          signaling;
    //static int                          encoding;        
};

  enum FlitType { FLIT_TYPE_HEAD, FLIT_TYPE_BODY, FLIT_TYPE_TAIL };

// Packet definition
struct PacketNoC {

    int    src_id;
    int    dst_id;

    int    size;
    int    flit_left;		// Number of remaining flits inside the packet

    int    packet_type;
    
    string data;
    int    time;

};

int GetSimTime();


//long   ran_next( );

// Returns a random integer in the range [0,max]
inline int RandomInt( int max ) {
  //return ( ran_next( ) % (max+1) );
    return ( (max+1) );
}

/*
int        global_clk 				   = DEFAULT_GLOBAL_CLK;
int        mesh_dim_x 				   = DEFAULT_MESH_DIM_X;
int        mesh_dim_y 				   = DEFAULT_MESH_DIM_Y;
int        num_vcs 				   = DEFAULT_NUM_VCS;
int        vcs_size 				   = DEFAULT_VCS_SIZE;
bool       detailed                                = DEFAULT_DETAILED;
int        NoCsignaling				   = SIGNALING_LEVEL;
int        NoCencoding 				   = ENCODING_BINARY;
*/


#endif
