#ifndef PARTICLE_H
#define PARTICLE_H

/*
*Particle object with x&y coordinate components
*/

class Particle  {
public:
    double x;
    double y;
    Particle(){};
    Particle(double a, double b){ x=a; y=b;}
    void pup(PUP::er &p){
    p|x;
    p|y;
    }

};

#endif
