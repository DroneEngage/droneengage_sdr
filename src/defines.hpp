#ifndef DEFINES_H_
#define DEFINES_H_

#include <limits> 
#include <string>


enum ANDRUAV_UNIT_P2P_TYPE 
{   
    unknown         = 0,
    esp32_mesh      = 1
     
};

typedef struct DE_UNIT_P2P_INFO{
    
    // connection status
    std::string  party_id="";
    std::string  group_id="";
    std::string  unit_name="";

    ANDRUAV_UNIT_P2P_TYPE p2p_connection_type = ANDRUAV_UNIT_P2P_TYPE::unknown;
    std::string address_1="";
    std::string address_ap="";
    
    std::uint8_t wifi_channel=0;
    std::string wifi_password="";

    std::string firmware_version = "na";
    
    uint16_t network_layer = std::numeric_limits<uint16_t>::max() ;  // 0 if root or direct p2p with no mesh.

    std::string parent_address      = "";
    bool parent_connection_status   = false;
    bool driver_connected           = false;
    bool is_p2p_connected           = false;
    bool is_p2p_disabled            = false;

    int swarm_leader_formation=-1;
    int swarm_follower_formation=-1;
    std::string swarm_leader_I_am_following="";

} DE_UNIT_P2P_INFO;


/**
 * @brief configuraytion file path & name
 * 
 */
static std::string configName = "de_sdr.config.module.json";
static std::string localConfigName = "de_sdr.local";



#endif