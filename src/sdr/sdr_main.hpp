#ifndef SDR_MAIN_H_
#define SDR_MAIN_H_

#include <thread>         // std::thread
#include <mutex>          // std::mutex, std::unique_lock



#include "../de_common/messages.hpp"
#include "../de_common/de_module.hpp"
#include "../defines.hpp"

#include "sdr_facade.hpp"
#include "sdr_parser.hpp"


#include "../helpers/json_nlohmann.hpp"
using Json_de = nlohmann::json;


typedef struct {

    int32_t latitude = 0;
    int32_t longitude = 0;
    int32_t altitude = std::numeric_limits<int32_t>::min();  
    int32_t altitude_relative = std::numeric_limits<int32_t>::min(); 
    float   air_speed = 0.0f;
    float   ground_speed = 0.0f;
    uint64_t last_access_time = 0;
    uint32_t h_acc; /*< [mm] Position uncertainty.*/
    uint32_t v_acc; /*< [mm] Altitude uncertainty.*/
    uint32_t vel_acc; /*< [mm] Speed uncertainty.*/
    uint16_t yaw; /*< [cdeg] Yaw in earth frame from north. Use 0 if this GPS does not provide yaw. Use 65535 if this GPS is configured to provide yaw and is currently unable to provide it. Use 36000 for north.*/

    bool is_valid = false;
    bool is_new = false;   
} ANDRUAV_UNIT_LOCATION;

namespace de
{
namespace sdr
{

    /**
     * @brief 
     * 
     */
    class CSDRMain
    {
        public:

            static CSDRMain& getInstance()
            {
                static CSDRMain instance;

                return instance;
            }

            CSDRMain(CSDRMain const&)               = delete;
            void operator=(CSDRMain const&)         = delete;

        
        private:

            CSDRMain() 
            {

            }

            
        public:
            
            ~CSDRMain ()
            {

            }
        
        public:
            
            bool init ();
            bool uninit ();
            void loopScheduler();


        public:
            /**
             * @brief Set the PartyID & GroupID
             * 
             * @param party_id 
             * @param group_id 
             */
            void setPartyID (const std::string& party_id, const std::string& group_id) 
            {
                m_party_id = party_id;
                m_group_id = group_id;
            };


        public:


            inline ANDRUAV_UNIT_LOCATION& getUnitLocationInfo()
            {
                return m_unit_location_info;
            }


            const inline int32_t getLatitude() const 
            {
                return m_unit_location_info.latitude;
            }

            const inline int32_t getLongitude() const 
            {
                return m_unit_location_info.longitude;
            }

            const inline int32_t getRelativeAltitude() const 
            {
                return m_unit_location_info.altitude_relative;
            }

            const inline int32_t getAbsoluteAltitude() const 
            {
                return m_unit_location_info.altitude;
            }

            const  inline double getLocationReportStatus() const 
            {
                return m_report_static_location;
            }

            const inline bool getSendingLocation () const
            {
                return m_sending_location;
            }
            

            const inline void setSendingLocation (const bool enable)
            {
                m_sending_location = enable;
            }

        protected:
            void initSDRParameters();

        private:
            de::sdr::CSDR_Facade& m_sdr_facade = de::sdr::CSDR_Facade::getInstance();

        private:
            bool m_sending_location = true;
            bool m_exit_thread = true;
            u_int64_t m_counter =0;
            std::thread m_scheduler_thread;

            std::string m_party_id;
            std::string m_group_id;

            std::string m_driver;

            bool m_report_static_location = false;
            
            ANDRUAV_UNIT_LOCATION m_unit_location_info;
    };

}
}
#endif
