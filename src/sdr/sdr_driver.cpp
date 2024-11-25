#include "../helpers/colors.hpp"
#include "../helpers/helpers.hpp"

#include "../de_common/configFile.hpp"
#include "../de_common/messages.hpp"
#include "sdr_facade.hpp"
#include "sdr_driver.hpp"

// #define DDEBUG 1

using namespace de::sdr;

CSDR_Facade &cSDR_Facade = CSDR_Facade::getInstance();


void CSDRDriver::setStatus(ENUM_STATE status)
{
    bool changed = (m_status != status);
    m_status = status;

    if (changed)
    {
        cSDR_Facade.API_SDRInfo(std::string(""));
    }
}

bool CSDRDriver::init()
{

    m_gain = 50.0;

    m_freq_center = 100.3e6;

    m_sample_rate = 1.3e6;

    return true;
}

void CSDRDriver::listDevices()
{
    SoapySDR::Kwargs device_arg;

    m_device_args.clear();

    m_device_args = SoapySDR::Device::enumerate();

    std::cout << "Number of SDR Devices Found: " << (long)m_device_args.size() << std::endl;

    if (m_device_args.size() < 1)
        return;

    for (unsigned int k = 0; k < m_device_args.size(); ++k)
    {
        fprintf(stderr, "SDR device =  %ld ", (long)k);
        device_arg = m_device_args[k];
        for (SoapySDR::Kwargs::const_iterator it = device_arg.begin(); it != device_arg.end(); ++it)
        {
            std::cout << it->first << " = " << it->second << std::endl;
        }
    }
}

/**
 * @brief select driver name by driver index.
 * index should be included in the connection string
 * as multiple devices with same brand can exist.
 *
 * @param sdr_driver_index
 */
void CSDRDriver::setSDRDriverIndex(const int sdr_driver_index)
{
    listDevices();

    if ((int)m_device_args.size() <= sdr_driver_index)
    {
        std::cout << "SDR Driver - Out of index." << std::endl;
        return;
    }

    m_sdr_index = sdr_driver_index;

    SoapySDR::Kwargs device_arg = m_device_args[m_sdr_index];

    for (SoapySDR::Kwargs::const_iterator it = device_arg.begin(); it != device_arg.end(); ++it)
    {
        std::cout << it->first << " = " << it->second << std::endl;
        if (it->first == "driver")
        {
            m_sdr_driver = it->second;
            return;
        }
    }
}

void CSDRDriver::setSDRDriverByName(const std::string sdr_driver)
{
    listDevices();

    for (int i = 0; i < (int)m_device_args.size(); ++i)
    {
        SoapySDR::Kwargs device_arg = m_device_args[i];

        for (SoapySDR::Kwargs::const_iterator it = device_arg.begin(); it != device_arg.end(); ++it)
        {
            std::cout << it->first << " = " << it->second << std::endl;
            if (it->first == "driver")
            {
                if (sdr_driver == it->second)
                {
                    m_sdr_index = i;
                    m_sdr_driver = it->second;
                }
                return;
            }
        }
    }
}

bool CSDRDriver::closeSDR()
{
    try
    {
        // deactivate drivers.
        if (m_sdr != NULL)
        {

            m_sdr->deactivateStream(m_rxStream, 0, 0);

            m_sdr->closeStream(m_rxStream);

            SoapySDR::Device::unmake(m_sdr);
        }

        // delete buffers

        buff1.clear();
        buff2.clear();

        buff1.shrink_to_fit();
        buff2.shrink_to_fit();

        return true;
    }
    catch (...)
    {
        return false;
    }
}


void CSDRDriver::pauseStreaming()
{
    m_exit = true;
}

void CSDRDriver::startStreaming()
{
    m_exit = false;

    do
    {
        startStreamingOnce();
        if (m_exit)
            break;
        wait_time_nsec(0, m_intervals * 1000000000l);
        if (m_exit)
            break;
    } while (m_intervals);
    
    m_intervals = 0;
    setStatus(ENUM_STATE::CONNECTED);
}


/**
 * @brief connect to SDR driver and activate stream.
 *
 * @return true
 * @return false
 */
bool CSDRDriver::openSDR()
{
    m_exit = true;
    std::lock_guard<std::mutex> lock(m_lock);

    if (!closeSDR())
    {
        const std::string error_msg = "Could not close SDR.";
        cSDR_Facade.sendErrorMessage(std::string(), 0, ERROR_TYPE_ERROR_P2P, NOTIFICATION_TYPE_WARNING, error_msg);
        std::cout << error_msg << std::endl;
        setStatus(ENUM_STATE::ERROR);
    }

    setStatus(ENUM_STATE::NOT_CONNECTED);
    setSDRDriverIndex(m_sdr_index);

    // Open SDR Device.
    std::cout << "m_sdr_driver: " << m_sdr_driver << " m_sdr_index: " << m_sdr_index << std::endl;
    std::string connection = m_sdr_driver + ",index=" + std::to_string(m_sdr_index);
    m_sdr = SoapySDR::Device::make(connection);

    if (m_sdr == NULL)
    {
        const std::string error_msg = "No SDR device found.";
        std::cout << error_msg << std::endl;
        cSDR_Facade.sendErrorMessage(std::string(), 0, ERROR_TYPE_ERROR_P2P, NOTIFICATION_TYPE_ERROR, error_msg);
        setStatus(ENUM_STATE::ERROR);
        return false;
    }

    setStatus(ENUM_STATE::CONNECTED);
    cSDR_Facade.sendErrorMessage(std::string(), 0, ERROR_TYPE_ERROR_P2P, NOTIFICATION_TYPE_INFO, "SDR is Active.");

    m_sdr->setFrequency(SOAPY_SDR_RX, 0, "RF", m_freq_center);

#ifdef DDEBUG
    std::cout << "m_freq_center:" << m_freq_center << std::endl;
#endif

    m_sdr->setSampleRate(SOAPY_SDR_RX, 0, m_sample_rate);

#ifdef DDEBUG
    std::cout << "m_sample_rate:" << m_sample_rate << std::endl;
#endif

    m_sdr->setGain(SOAPY_SDR_RX, 0, m_gain);

#ifdef DDEBUG
    std::cout << "m_gain:" << m_gain << std::endl;
#endif

    m_size = m_sample_rate / 20;

    m_rxStream = m_sdr->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32, {});

    if (m_sdr->activateStream(m_rxStream, 0, 0, 0) != 0)
    {
        std::cout << _ERROR_CONSOLE_BOLD_TEXT_ << "ERROR: " << _TEXT_BOLD_HIGHTLITED_ << "Could not activate stream." << _NORMAL_CONSOLE_TEXT_ << std::endl;
        setStatus(ENUM_STATE::ERROR);
        return false;
    }

    m_max_tx_unit = m_sdr->getStreamMTU(m_rxStream);

    std::cout << "MCU: " << m_max_tx_unit << std::endl;

    // Allocate buffers for FFT
    buff1 = std::vector<float> ((int)m_sample_rate * 2);// new float[(int)m_sample_rate * 2];
    buff2 = std::vector<float> ((int)m_sample_rate * 2);// new float[(int)m_sample_rate * 2];

    /**
     *      EXAMPLE: p1 = fftwf_plan_dft_1d(n, in, out, FFTW_ESTIMATE);
     * Here, n is the number of complex samples, in is the input array (time-domain signal),
     * out is the output array (frequency-domain signal), 
     * and FFTW_ESTIMATE is a flag that tells FFTW to use a simple, fast strategy for planning.
     */
    p1 = fftwf_plan_dft_1d(m_sample_rate, (fftwf_complex *)buff1.data(), (fftwf_complex *)buff2.data(), FFTW_FORWARD, FFTW_ESTIMATE);

    std::cout << _SUCCESS_CONSOLE_BOLD_TEXT_ << "Initialized SDR with sample rate: " << m_sample_rate << _NORMAL_CONSOLE_TEXT_ << std::endl;

    return true;
}



void CSDRDriver::startStreamingOnce()
{
    std::lock_guard<std::mutex> lock(m_lock);

    if ((m_status != ENUM_STATE::CONNECTED) && (m_status != ENUM_STATE::STREAMING_INTERVALS))
        return;
    try
    {
        // switch to streaming mode.
        setStatus(m_intervals!=0?ENUM_STATE::STREAMING_INTERVALS:ENUM_STATE::STREAMING_ONCE);


        void *buffs[1];
        float *out;
        int flags = 0;
        long long timeNs = 0;
        int rec = m_sample_rate; // Total number of samples to read
        uint numElems;
        out = buff1.data();

#ifdef DDEBUG
        std::cout << " ============= rec:" << rec << std::endl;
#endif

        while (rec > 0)
        {
            flags = 0;
            
            // Prepare to read samples into a buffer
            buffs[0] = out;
            numElems = std::min(static_cast<uint>(std::max(0, rec)), static_cast<uint>(m_max_tx_unit));  // Limit to max transmission unit

#ifdef DDDEBUG
            std::cout << "=====================numElems " << numElems << std::endl;
#endif

            // IMPORTANT: DO READING HERE
            int ret = m_sdr->readStream(m_rxStream, buffs, numElems, flags, timeNs, 100000L);

            if (ret < 0)
            {
                // If there's an error, skip this attempt
                continue;
            }

            // advance pointer to next location in buffer
            out += 2 * (long long)ret;

            // update remaining number of samples to read.
            rec -= ret;
        }

        // Execute FFT on the received samples
        // Before this line is executed, you typically create an FFT plan using a function like fftwf_plan_dft_1d. 
        // The plan specifies how to compute the FFT for a given set of input data.
        fftwf_execute(p1);

        

        const double frequency_min = m_freq_center - 0.5 * m_sample_rate;
        const uint64_t div = (int)m_sample_rate / m_bars;
        const float frquency_step = static_cast<float>(m_sample_rate) / m_bars;
        
        std::vector<float> output(m_bars);
        uint64_t number_of_data = 0;
        for (long int np = 0; np < m_bars; ++np)
        {
            double frequency_value_sum = 0.0;
            const long int start_index = np * div;
            const long int end_index = start_index + div - 1;

            for (long int i = start_index; i <= end_index; ++i)
            {
                const long int index = i * 2;
                const double frequency_value = hypot(buff2[index], buff2[index + 1]); // Magnitude
                frequency_value_sum += frequency_value;
                
            }

            // Calculate average frequency value
            double average_frequency_value = frequency_value_sum / static_cast<double>(div);
            output[number_of_data] = static_cast<float>(average_frequency_value);
            ++number_of_data;

            if ((m_trigger_level > 0) && (average_frequency_value > m_trigger_level))
            {
                CSDR_Facade::getInstance().sendSignalAlert("", frequency_min + ((start_index + end_index) / 2 )* frquency_step, average_frequency_value);
            }
        }

#ifdef DEBUG
        std::cout << "sendSpectrumResultInfo" << std::endl;
#endif

        CSDR_Facade::getInstance().sendSpectrumResultInfo("", frequency_min, frquency_step, output.size(), output.data());
        

        // switch back to connected state "READY"
        if (m_status != ENUM_STATE::STREAMING_INTERVALS)
        {   // avoid chaning status rapidly when we are still in streaming process. otherwise dont send update message.
            setStatus(ENUM_STATE::CONNECTED);
        }
    }
    catch (...)
    {
        std::cout << "ERROR:" << std::endl;
        setStatus(ENUM_STATE::CONNECTED);
    }
}