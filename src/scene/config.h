#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <iostream>

class Config
{
private:
    double samples;
    double lightRadius;
    double lightSamples;
    int mcDepth;
    
public:
    Config(double s=20.0, double lr=5.0, double ls=100.0) :
        samples(s), lightRadius(lr), lightSamples(ls),
        mcDepth(2) {}

    ~Config() {}

    double getSamples() { return samples; }
    double getLightRadius() { return lightRadius; }
    double getLightSamples() { return lightSamples; }
    double getMCDepth() { return mcDepth; }
};

#endif // __CONFIG_H__
