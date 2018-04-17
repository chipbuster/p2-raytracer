#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <iostream>

class Config
{
private:
    double radius;
    double samples;
    
public:
    Config() {}

    Config(double r, double s) :
        radius(r), samples(s) {}

    ~Config() {}

    double getRadius() { return radius; }
    double getSamples() { return samples; }
};

#endif // __CONFIG_H__
