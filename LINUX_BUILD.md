install JUCE dependencies: https://github.com/juce-framework/JUCE/blob/develop/docs/Linux%20Dependencies.md

clone the repo:
`git clone -b linux-fixes https://github.com/KottV/Vaporizer2`

build Projucer
`CONFIG=Release make -j8 -C ./JUCE/extras/Projucer/Builds/LinuxMakefile/`

export Makefile
`./JUCE/extras/Projucer/Builds/LinuxMakefile/build/Projucer --resave ./VASTvaporizer/VASTvaporizer.jucer`

build the synth
`CONFIG=Release make -j8 -C ./VASTvaporizer/Builds/LinuxMakefile/`

binaries shall be in `./VASTvaporizer/Builds/LinuxMakefile/build/`

