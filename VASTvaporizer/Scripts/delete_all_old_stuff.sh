#!/bin/bash

sudo rm -R /Library/Audio/Plug-Ins/Components/VAST*
sudo rm -R /Library/Audio/Plug-Ins/VST/VAST*
sudo rm -R /Library/Audio/Plug-Ins/VST3/VAST*
sudo rm -R "/Library/Application Support/Avid/Audio/Plug-Ins/"VAST*
sudo rm -R "/Users/setup/Library/Developer/VST/JUCE Projects/VASTvaporizer/Builds/MacOSX/build/Release 64bit/VASTvaporizer2_64.app"

sudo chmod 777 /Library/Audio/Plug-Ins/VST
sudo chmod 777 /Library/Audio/Plug-Ins/VST3
sudo chmod 777 /Library/Audio/Plug-Ins/Components
