
[![Ardupilot Cloud EcoSystem](https://cloud.ardupilot.org/_static/ardupilot_logo.png  "Ardupilot Cloud EcoSystem")](https://cloud.ardupilot.org  "Ardupilot Cloud EcoSystem") **Drone Engage** is part of Ardupilot Cloud Eco System

  

------------

  

![Drone Engage SDR Module](resources/de_logo_title.png)

  

# Drone Engage SDR Module

  

DroneEngage-SDR is a Drone Engage plugin module that enables DroneEngage to control SDR hardware for signal scanning and detection.

  
[![SDR Youtube](https://raw.githubusercontent.com/DroneEngage/droneengage_sdr/refs/heads/master/resources/youtube_sdr_plugin.png)](https://www.youtube.com/watch?v=_Ek24PA5dso)

The plugin is a proof-of-concept of how DroneEngage can be extended to inlcude varaities of sensors and devices and performs different functions.

  

[![WebClient-SDR](https://github.com/DroneEngage/droneengage_sdr/blob/master/resources/webclient_sdr.png?raw=true  "WebClient-SDR")](https://github.com/DroneEngage/droneengage_sdr/blob/master/resources/webclient_sdr.png?raw=true  "WebClient-SDR")


## How to Build


Install Libs

    $ sudo apt-get install libfftw3-dev
    $ sudo apt-get install libmirisdr-dev


Download & compile SoapySDR

    $ git clone https://github.com/HefnySco/SoapySDR.git --depth 1
    $ cd SoapySDR
    $ cmake ..
    $ make
    $ sudo make install 


Download & compile Liquid-dsp

    $ git clone https://github.com/HefnySco/dsp_liquid-dsp.git --depth 1
    $ cd dsp_liquid-dsp
    $./bootstrap.sh
    $ ./configure
    $ make
    $ sudo make install



Download & compile DroneEngage SDR

    $ git clone https://github.com/DroneEngage/droneengage_sdr.git
    $ cd droneengage_sdr
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make



