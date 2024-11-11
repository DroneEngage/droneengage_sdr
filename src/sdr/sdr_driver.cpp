#include "../helpers/colors.hpp"
#include "../helpers/helpers.hpp"

#include "../de_common/configFile.hpp"
#include "../de_common/messages.hpp"
#include "sdr_facade.hpp"
#include "sdr_driver.hpp"

// #define DDEBUG 1

using namespace de::sdr;

CSDR_Facade &cSDR_Facade = CSDR_Facade::getInstance();

int setFilters(DE_SDR_RX &rx, DE_Filters &f)
{

    float As = 60.0f;

    liquid_ampmodem_type mode = LIQUID_AMPMODEM_DSB;

    int iflag = 0;

    if (rx.decode_mode == (int)ENUM_DECODING_MODE::MODE_AM)
    {
        std::cout << "ENUM_DECODING_MODE::MODE_AM" << std::endl;
        rx.bw = 10000.0;
        mode = LIQUID_AMPMODEM_DSB;
        iflag = 0;
    }
    else if (rx.decode_mode == (int)ENUM_DECODING_MODE::MODE_NAM)
    {
        std::cout << "ENUM_DECODING_MODE::MODE_NAM" << std::endl;
        rx.bw = 5000.0;
        mode = LIQUID_AMPMODEM_DSB;
        iflag = 0;
    }
    else if (rx.decode_mode == (int)ENUM_DECODING_MODE::MODE_NBFM)
    {
        std::cout << "ENUM_DECODING_MODE::MODE_NBFM" << std::endl;
        rx.bw = 12500.0;
    }
    else if (rx.decode_mode == (int)ENUM_DECODING_MODE::MODE_FM)
    {
        std::cout << "ENUM_DECODING_MODE::MODE_FM" << std::endl;
        rx.bw = 200000.0;
    }
    else if (rx.decode_mode == (int)ENUM_DECODING_MODE::MODE_USB)
    { // Above 10 MHZ
        std::cout << "ENUM_DECODING_MODE::MODE_USB" << std::endl;
        rx.bw = 6000.0;
        mode = LIQUID_AMPMODEM_USB;
        iflag = 1;
    }
    else if (rx.decode_mode == (int)ENUM_DECODING_MODE::MODE_LSB)
    { // Below 10 MHZ
        std::cout << "ENUM_DECODING_MODE::MODE_LSB" << std::endl;
        rx.bw = 6000.0;
        mode = LIQUID_AMPMODEM_LSB;
        iflag = 1;
    }
    else if (rx.decode_mode == (int)ENUM_DECODING_MODE::MODE_CW)
    {
        std::cout << "ENUM_DECODING_MODE::MODE_CW" << std::endl;
        rx.bw = 3000.0;
        mode = LIQUID_AMPMODEM_LSB;
        iflag = 1;
    }

    double Ratio1 = (float)(rx.bw / rx.sample_rate);

    double Ratio2 = (float)(rx.faudio / rx.bw);

    fprintf(stderr, "Ratio1 %g Ratio2 %g\n", Ratio1, Ratio2);

    f.demod = freqdem_create(0.5);

#ifdef LIQUID_VERSION_4
    f.demodAM = ampmodem_create(0.5, 0.0, mode, iflag);
#else
    f.demodAM = ampmodem_create(0.5, mode, iflag);
#endif

    f.iqSampler = msresamp_crcf_create(Ratio1, As);

    f.iqSampler2 = msresamp_rrrf_create(Ratio2, As);

    f.fShift = nco_crcf_create(LIQUID_NCO);

    f.amHistory = 0;

    return 0;
}

cStack::cStack(DE_SDR_RX *rxi)
{
    rx = rxi;
    bufftopa = 0;
    bufftop = 0;

    for (int k = 0; k < NUM_ABUFF; ++k)
    {
        buffStacka[k] = 0;
        buffa[k] = NULL;
    }
}
cStack::~cStack()
{

    for (int k = 0; k < NUM_ABUFF; ++k)
    {
        if (buffa[k])
            free((char *)buffa[k]);
        buffa[k] = NULL;
    }
}

int cStack::setBuff(DE_SDR_RX *rx)
{

    for (int k = 0; k < NUM_ABUFF; ++k)
    {
        if (buffa[k])
            free((char *)buffa[k]);
        buffa[k] = (float *)malloc((size_t)(2 * rx->faudio * 4 * sizeof(float)));
        if (!buffa[k])
        {
            fprintf(stderr, "malloc Error %ld\n", (long)(2 * rx->faudio * 4));
            return 1;
        }
        memset(buffa[k], 0, 2 * rx->faudio * 4 * sizeof(float));
        buffStacka[k] = -1;
    }

    return 0;
}

int cStack::pushBuffa(int nbuffer, DE_SDR_RX *rx)
{

    rx->mutexa.lock();

    if (bufftopa >= NUM_ABUFF)
    {
        bufftopa = NUM_ABUFF;
        int small2, ks;
        small2 = 1000000000;
        ks = -1;
        for (int k = 0; k < NUM_ABUFF; ++k)
        {
            if (buffStacka[k] < small2)
            {
                small2 = buffStacka[k];
                ks = k;
            }
        }

        if (ks >= 0)
        {
            buffStacka[ks] = nbuffer;
        }
    }
    else
    {
        buffStacka[bufftopa++] = nbuffer;
    }

    rx->mutexa.unlock();

    return 0;
}

int cStack::popBuffa(DE_SDR_RX *rx)
{
    int ret;

    rx->mutexa.lock();

    ret = -1;

    if (bufftopa < 1)
        goto Out;

    if (bufftopa == 1)
    {
        ret = buffStacka[0];
        bufftopa = 0;
        goto Out;
    }

    int small2, ks;
    small2 = 1000000000;
    ks = -1;
    for (int k = 0; k < bufftopa; ++k)
    {
        if (buffStacka[k] < small2)
        {
            small2 = buffStacka[k];
            ks = k;
        }
    }

    if (ks >= 0)
    {
        ret = buffStacka[ks];
        int kk;
        kk = 0;
        for (int k = 0; k < bufftopa; ++k)
        {
            if (k == ks)
                continue;
            buffStacka[kk++] = buffStacka[k];
        }
        bufftopa--;
    }

Out:
    rx->mutexa.unlock();

    return ret;
}

int CSDRDriver::center_fft(fftw_complex *out, int N) const
{
    int i;
    double work;

    // centering the fourier transform
    for (i = 0; i < N / 2; i++)
    {
        work = out[N / 2 + i][0];
        out[N / 2 + i][0] = out[i][0];
        out[i][0] = work;

        work = out[N / 2 + i][1];
        out[N / 2 + i][1] = out[i][1];
        out[i][1] = work;
    }
    return 0;
}

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

    // m_sdr_rx.cs=new cStack(rx);

    m_sdr_rx.decode_mode = (int)ENUM_DECODING_MODE::MODE_NBFM;

    m_sdr_rx.gain = 50.0;

    m_sdr_rx.fc = 100.3e6;

    m_sdr_rx.f = 100.6e6;

    m_sdr_rx.sample_rate = 1.3e6;

    m_sdr_rx.faudio = 48000;

    return true;
}

void CSDRDriver::listDevices()
{
    SoapySDR::Kwargs device_arg;

    m_device_args.clear();

    m_device_args = SoapySDR::Device::enumerate();

    fprintf(stderr, "Number of SDR Devices Found: %ld\n", (long)m_device_args.size());

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

        if (buff1)
            delete[] buff1;
        buff1 = NULL;

        if (buff2)
            delete[] buff2;
        buff2 = NULL;

        if (buff3)
            delete[] buff3;
        buff3 = NULL;

        return true;
    }
    catch (...)
    {
        return false;
    }
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
        const std::string error_msg = "No SDR device! Found";
        std::cout << error_msg << std::endl;
        cSDR_Facade.sendErrorMessage(std::string(), 0, ERROR_TYPE_ERROR_P2P, NOTIFICATION_TYPE_ERROR, error_msg);
        setStatus(ENUM_STATE::ERROR);
        return false;
    }

    setStatus(ENUM_STATE::CONNECTED);
    cSDR_Facade.sendErrorMessage(std::string(), 0, ERROR_TYPE_ERROR_P2P, NOTIFICATION_TYPE_INFO, "SDR is Active.");

    m_sdr->setFrequency(SOAPY_SDR_RX, 0, "RF", m_sdr_rx.fc);

#ifdef DDEBUG
    std::cout << "m_sdr_rx.fc:" << m_sdr_rx.fc << std::endl;
#endif

    m_sdr->setSampleRate(SOAPY_SDR_RX, 0, m_sdr_rx.sample_rate);

#ifdef DDEBUG
    std::cout << "m_sdr_rx.sample_rate:" << m_sdr_rx.sample_rate << std::endl;
#endif

    m_sdr->setGain(SOAPY_SDR_RX, 0, m_sdr_rx.gain);

#ifdef DDEBUG
    std::cout << "m_sdr_rx.gain:" << m_sdr_rx.gain << std::endl;
#endif

    m_sdr_rx.size = m_sdr_rx.sample_rate / 20;

    m_rxStream = m_sdr->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32, (const std::vector<size_t>)0);

    const int ret = m_sdr->activateStream(m_rxStream, 0, 0, 0);
    if (ret != 0)
    {
        std::cout << _ERROR_CONSOLE_BOLD_TEXT_ << "ERROR: " << _TEXT_BOLD_HIGHTLITED_ << "Could not activate stream." << _NORMAL_CONSOLE_TEXT_ << std::endl;
        setStatus(ENUM_STATE::ERROR);
        return false;
    }

    m_max_tx_unit = m_sdr->getStreamMTU(m_rxStream);

    std::cout << "MCU: " << m_max_tx_unit << std::endl;

    setFilters(m_sdr_rx, m_sdr_rx.fs);

    float shift = m_sdr_rx.fc - (m_sdr_rx.fc * 0.971698133); // estimation for m_sdr_rx.f;

    nco_crcf_set_frequency(m_sdr_rx.fs.fShift, (float)((2.0 * M_PI) * (((double)abs(shift)) / ((double)m_sdr_rx.sample_rate))));

    // create a re-usable buffer for rx samples
    //  The FFTW library expects the input and output buffers to be of a specific size and layout.
    //  In this case, the input buffer buff1 is expected to be of size m_sdr_rx.sample_rate * 2,
    //  as it is treated as a complex buffer (interleaved real and imaginary parts).
    buff1 = new float[(int)m_sdr_rx.sample_rate * 2];
    buff2 = new float[(int)m_sdr_rx.sample_rate * 2];
    buff3 = new float[(int)m_sdr_rx.sample_rate * 2];

#ifdef DDEBUG
    std::cout << "=====================m_sdr_rx.sample_rate " << m_sdr_rx.sample_rate << std::endl;
#endif

    p1 = fftwf_plan_dft_1d(m_sdr_rx.sample_rate, (fftwf_complex *)buff1, (fftwf_complex *)buff2, FFTW_FORWARD, FFTW_ESTIMATE);

    p2 = fftwf_plan_dft_1d((int)m_sdr_rx.bw, (fftwf_complex *)buff3, (fftwf_complex *)buff1, FFTW_BACKWARD, FFTW_ESTIMATE);

    std::cout << _SUCCESS_CONSOLE_BOLD_TEXT_ << "Read  m_sdr_rx.bw " << std::to_string((int)m_sdr_rx.bw) << _NORMAL_CONSOLE_TEXT_ << std::endl;

    return true;
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
        int rec = m_sdr_rx.sample_rate;
        uint numElems;
        out = buff1;

#ifdef DDEBUG
        std::cout << " ============= rec:" << rec << std::endl;
#endif

        while (rec > 0)
        {
            flags = 0;
            buffs[0] = out;
            numElems = rec;
            if (numElems > (uint)m_max_tx_unit)
                numElems = (uint)m_max_tx_unit;

#ifdef DDDEBUG
            std::cout << "=====================numElems " << numElems << std::endl;
#endif

            // IMPORTANT: DO READING HERE
            int ret = m_sdr->readStream(m_rxStream, buffs, numElems, flags, timeNs, 100000L);

            if (ret < 0)
            {
                // skip this attempt
                continue;
            }

            // advance pointer to next location in buffer
            out += 2 * (long long)ret;

            // update remaining number of samples to read.
            rec -= ret;
        }

        const long int sr = m_sdr_rx.sample_rate;
        for (long int n = 0; n < (long int)(sr); ++n)
        {
            buff1[n * 2] *= pow(-1.0, n);
            buff1[n * 2 + 1] *= pow(-1.0, n);
        }

        fftwf_execute(p1);

        if (true)
        {

            const double frequency_min = m_sdr_rx.fc - 0.5 * m_sdr_rx.sample_rate;
#ifdef DEBUG
            printf("plot %d sinwave1\n", (int)m_sdr_rx.sample_rate);
#endif

            const uint64_t div = (int)m_sdr_rx.sample_rate / m_bars;
            const float frquency_step = static_cast<float>(m_sdr_rx.sample_rate) / m_bars;
            const long int num_output_points = m_sdr_rx.sample_rate / div;

            std::vector<float> output(num_output_points);
            uint64_t number_of_data = 0;
            for (long int np = 0; np < num_output_points; ++np)
            {
                double frequency_sum = 0.0;
                double frequency_value_sum = 0.0;
                long int start_index = np * div;
                long int end_index = (np + 1) * div - 1;

                for (long int i = start_index; i <= end_index; ++i)
                {
                    double frequency_value = hypot(buff2[i * 2], buff2[i * 2 + 1]);
                    frequency_value_sum += frequency_value;
                    double frequency = frequency_min + i;
                    frequency_sum += frequency;
                }

                double average_frequency_value = frequency_value_sum / frquency_step;
                output[number_of_data] = static_cast<float>(average_frequency_value);
                ++number_of_data;
#ifdef DDEBUG
                double average_frequency = frequency_sum / frquency_step;
                printf("%f %f\n", average_frequency, average_frequency_value);
#endif
            }

#ifdef DEBUG
            std::cout << "sendSpectrumResultInfo" << std::endl;
#endif

            CSDR_Facade::getInstance().sendSpectrumResultInfo("", frequency_min, frquency_step, output.size(), output.data());
        }

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