#ifndef PSG_H
#define PSG_H

#include <iostream>

class PSG {
    
public:
    PSG(double d);
    PSG(double rs, double fl, double d);
    ~PSG(void);

    double update(double, double);
    int isSpiking();
    void setState(int);
    int getState();
    void resetTimeConst();

    void setDt(double);

private:

    double exp1;
    double exp2;

    double PSGrise;
    double PSGfall;

    double peakval;
    double fact1;
    double fact2;
    
    int state;
    double dt;

};

#endif //PSG_H
