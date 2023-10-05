install JUCE dependencies: https://github.com/juce-framework/JUCE/blob/develop/docs/Linux%20Dependencies.md
(though `ladspa-sdk libcurl4-openssl-dev libwebkit2gtk-4.0-dev` aren't needed here)

clone the repo:  
`git clone -b linux-fixes https://github.com/KottV/Vaporizer2`  
`cd Vaporizer2`

build Projucer:  
`CONFIG=Release make -j8 -C ./JUCE/extras/Projucer/Builds/LinuxMakefile/`

export Makefile:  
`./JUCE/extras/Projucer/Builds/LinuxMakefile/build/Projucer --resave ./VASTvaporizer/VASTvaporizer.jucer`

build the synth:  
`CONFIG=Release make -j8 -C ./VASTvaporizer/Builds/LinuxMakefile/`

"8" - is the number of CPU cores here, replace it with yours

binaries shall be in: `./VASTvaporizer/Builds/LinuxMakefile/build/`

1. At the first run check "disable GFX" in Preset->Settings tab, there is a GUI performance issue ATM.
2. Vaporizer2's data must be placed somewhere, by now it searches inside `/tmp/` at first run, so place/symlink `Presets` `Noises` and `Tables` subfolders there, or wherever you want and set paths in the Preset->Settings tab.
