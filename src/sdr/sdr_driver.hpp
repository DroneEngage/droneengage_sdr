#ifndef SDR_DRIVER_H_
#define SDR_DRIVER_H_
#include <SoapySDR/Version.hpp>
#include <SoapySDR/Modules.hpp>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.h>
#include <vector>
#include <string>
#include <liquid/liquid.h>
#include <fftw3.h>
#include <mutex>


namespace de
{
namespace sdr
{


    typedef struct Filters{
        int np;
        ampmodem demodAM;
        freqdem demod;
        msresamp_crcf iqSampler;
        msresamp_rrrf iqSampler2;
        nco_crcf fShift;
        int thread;	
        double amHistory;
    } DE_Filters;


    typedef struct rxStruct{
        
        
        
        
        int iprint;
        
        double dummy;
        int last;
        std::mutex mutexo;
        std::mutex mutex1;
        std::mutex mutexa;

    } DE_SDR_RX;

    enum class ENUM_STATE
	{
		NOT_CONNECTED       = 0,
        CONNECTED           = 1,
		STREAMING_ONCE      = 2,
        STREAMING_INTERVALS  = 3,
        ERROR               = 999
	};

    enum class ENUM_DECODING_MODE
	{
		MODE_RAW        = 0,
        MODE_FM         = 1,
		MODE_NBFM       = 2,
		MODE_AM         = 3,
		MODE_NAM        = 4,
		MODE_USB        = 5,
		MODE_LSB        = 6,
        MODE_CW         = 7,
	};






#define NUM_ABUFF 10

#define NUM_DATA_BUFF 10


    
    /**
     * @brief 
     * 
     */
    class CSDRDriver
    {
        public:

            static CSDRDriver& getInstance()
            {
                static CSDRDriver instance;

                return instance;
            }

            CSDRDriver(CSDRDriver const&)               = delete;
            void operator=(CSDRDriver const&)           = delete;

        
        private:

            CSDRDriver() 
            {

            }

            
        public:
            
            ~CSDRDriver ()
            {

            }
        

        public:

            bool init();
            void listDevices();
            void startStreamingOnce();
            void startStreaming();
            void pauseStreaming();
            
            bool openSDR();
            bool closeSDR();
        
            
        public:

            inline ENUM_STATE getStatus() const
            {
                return m_status;
            }

            inline int getSDRDriverIndex() const
            {
                return m_sdr_index;
            }

            inline uint64_t getBars() const
            {
                return m_bars;
            }

            inline void setBars(const uint64_t bars)
            {
                m_bars = bars;
            }

            inline uint64_t getIntervals() const
            {
                return m_intervals;
            }

            inline void setIntervals(const uint64_t intervals)
            {
                m_intervals = intervals;
            }

            inline uint64_t getTriggerLevel() const
            {
                return m_trigger_level;
            }

            inline void setTriggerLevel(const uint64_t trigger_level)
            {
                m_trigger_level = trigger_level;
            }

            inline float getFrequencyCenter() const
            {
                return m_freq_center;
            }

            inline void setFrequencyCenter(const float frequency_center)
            {
                m_freq_center = frequency_center;
            }


            inline float getSampleRate() const
            {
                return m_sample_rate;
            }

            inline void setSampleRate(const float sample_rate)
            {
                m_sample_rate = sample_rate;
            }


            inline float getGain() const
            {
                return m_gain;
            }

            inline void setGain(const float gain)
            {
                m_gain = gain;
            }

            
            inline float getDecodeMode() const
            {
                return static_cast<float>(ENUM_DECODING_MODE::MODE_RAW);
            }

            inline std::vector<SoapySDR::Kwargs> get_device_agrs() const 
            {
                return m_device_args;
            }

        public:

            void setSDRDriverIndex(const int  sdr_driver_index);
            void setSDRDriverByName(const std::string sdr_driver);

        private:

            void setStatus(ENUM_STATE status);



        private:

            std::vector<SoapySDR::Kwargs> m_device_args;
            SoapySDR::Device *m_sdr = NULL;
            SoapySDR::Stream *m_rxStream = NULL;

            ENUM_STATE m_status;

            std::string m_sdr_driver;
            int m_sdr_index = 0; 
            std::vector<float>buff1;
            std::vector<float>buff2;
            

            fftwf_plan p1;
            fftwf_plan p2;
            size_t m_max_tx_unit;

            long m_size;
            uint64_t m_bars = 32;
            double m_freq_center;
            double m_gain;
            double m_sample_rate;
            uint64_t m_intervals = 0; // no interval
            uint64_t m_trigger_level = 0; // no trigger
            bool m_exit = false;

            std::mutex m_lock;
    };
}
}

#endif