#include "../helpers/helpers.hpp"
#include "sdr_main.hpp"
#include "sdr_facade.hpp"




void de::sdr::CSDRMain::loopScheduler()
{
    while (!m_exit_thread)
    {
        try
        {
            // timer each 10m sec.
            wait_time_nsec (0,10000000);

            m_counter++;

            if (m_counter%1000 ==0)
            {   // each 10000 msec
                //m_cP2P.update();
                CSDR_Facade::getInstance().API_SDRInfo("");
            }
        }
        catch (int error)
        {

        }
    }

    return ;
}

bool de::sdr::CSDRMain::init()
{
    m_exit_thread = false; 

    
    m_scheduler_thread = std::thread{[&](){ loopScheduler(); }};

    return true;
}

bool de::sdr::CSDRMain::uninit()
{
    return true;
}