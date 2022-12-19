q# Phase-Specific Stimulation
Real-time local field potential phase-specific stimulation

A set of RTXI modules written in C++ that allow stimulation at specific phases (peak or trough) of the local field potential (LFP) in real-time.
Theses modules were designed to extract the theta band from the LFP and stimulate in 1 of 4 available modes:
1. Peak
2. Trough
3. 20 Hz
4. 6 Hz


## Getting started

Prior to using the phase-specific stimulation modules, RTXI (Real-Time eXperiment Interface) should be installed and setup with all necessary hardware required to run the experiment. Detailed instructions for setting up RTXI can be found at http://rtxi.org/. 

### Overview

The package contains 2 modules:
1. Theta Filter
2. Phase Stimulator

The block diagram for connecting the modules to each other and the input and output of the DAC
<img src="https://github.com/ndlBU/phase_specific_stim/blob/master/Logo/block_diagram.png" width="600" align="available">



