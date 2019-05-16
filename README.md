# ESP01-plant-watering
A plant watering system using the ESP01 micro controller and MQTT / Home Assistant for controlling.

This repository contains the code used for programming the ESP01 using platformIO as well as well as the corresponding electric plan and the configuration used for Home Assistant integration. As MQTT is used for communication with Home Assistant, you need to set up a MQTT broker of your choice in advance.

## Part list
* ESP01
* 12V (or something else) water pump
* Flow meter (I used DIGMESA FHKSC)
* Water proof housing
* Two MOSFETs (for example 2N7000A and IRL520NPBF)
* 3.3 V voltage regulator (for example LD33V)
* Some cables and soldering material

## Electric Layout
The electric layout is pretty simple for this project.
You can find the electric plan in the following image.
![Electric Plan](electric_diagram.png?raw=true "Electric Plan")

## Flashing the ESP01
Instructions on how to flash the ESP01 as well as how to install and use platformio are available on the internet, therefore this will be neglected here.
In order to get the code to work, you need to modify the file [```src/config.h```](https://github.com/LukasK13/ESP01-plant-watering/blob/master/src/config.h). In this file you need to go through all definitions and adapt them to your needs.
Afterwards, you can compile and flash the software to the ESP01.

## Home Assistant Integration
The necessary configuration files for integrating the plant watering system in Home Assistant can be found in the folder: [```home-assistant```](https://github.com/LukasK13/ESP01-plant-watering/tree/master/home-assistant). I prefer to separate the different component types in my Home Assistant configuration. Therefore, you will find one file for each component used. Additionally, I added the necessary parts of my [```configuration.yaml```](https://github.com/LukasK13/ESP01-plant-watering/blob/master/home-assistant/configuraiton.yaml) file. The result of this integration is shown in the following image.
![Home Assistant Integration](home-assistant/home-assistant.png?raw=true "Home Assistant Integration")
