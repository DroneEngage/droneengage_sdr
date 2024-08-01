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

        private:
            de::sdr::CSDR_Facade& m_sdr_facade = de::sdr::CSDR_Facade::getInstance();

        private:
            bool m_exit_thread = true;
            u_int64_t m_counter =0;
            std::thread m_scheduler_thread;

            std::string m_party_id;
            std::string m_group_id;

            
    };

}
}
#endif
