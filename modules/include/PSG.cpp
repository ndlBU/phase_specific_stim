/*
* Syn. conductance
* intended to be included into many RTXI models--updated June 15th, 2007
*/

#include <math.h>
#include "PSG.h"

PSG::PSG(double d) {

    exp1 = 0;
    exp2 = 0;

    PSGrise = 1.68; 
    PSGfall = 6.21;
    
    state = 0;
    dt = d;

    resetTimeConst();
}

PSG::PSG(double rs, double fl, double d) {

    exp1 = 0;
    exp2 = 0;

 
    PSGrise = rs; 
    PSGfall = fl;

    state = 0;
    dt = d;

    resetTimeConst();
}
    

PSG::~PSG(void) {
}

/**
 * v - post synaptic membrane voltage in volts
 * dt - RTXI system period:  RT::System::getInstance()->getPeriod()*1e-6
 */
double PSG::update(double v, double d) {
    
    //This is what will be returned when update is called.
    double g;
    
    // when model period changes, update the synaptic waveform to compensate
    if (dt != d) {
        dt = d;
        resetTimeConst();
    }
   
    // detect spike
    if (isSpiking()) {
        exp1 += 1/peakval;
        exp2 += 1/peakval;
    }
    
    // compute exponentials
    exp1 = exp1 * fact1;
    exp2 = exp2 * fact2;
    g = exp2 - exp1;
    

    return g; 
}

/*
 * Called when dt changes
 */
void PSG::resetTimeConst() {

    double rs = PSGrise;
    double fl = PSGfall;

    fact1 = exp (-dt / fl);
    fact2 = exp (-dt / rs);
    peakval = exp((1/(1/fl-1/rs) * log(fl/rs))/rs) - exp ((1/(1/fl-1/rs)*log(fl/rs))/fl);
}


int PSG::isSpiking() {
    if (state == 1) {
        return 1; 
    } else {
        return 0;
    }
}

// set the state of the cell to determine if it is spiking
void PSG::setState(int s) {
    state = s;
}


int PSG::getState(void){
    return state;
}

void PSG::setDt(double dd) {
    dt = dd;
}
