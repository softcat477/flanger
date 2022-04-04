MUMT 618 Homework3

Folder structure
- flanger.cpp
- stk/
- README

Compile:
    g++ -std=c++11 -o flanger -Istk/include/ -Lstk/src/ flanger.cpp -lstk
        or
    sh comp.sh

Usage:
    ./flanger <filename>.wav

Output:
    <filename>_flanger.wav

Parameters:
    Flanger equations:
        y[n] = x[n] + gx[n-M[n]]
        M[n] = M0(1 + Asin(2pifnT))
    float   depth = g,          Range:[0.0, 1.0]
    bool    phase_inversion ,   false: g=g; true:g=-g
    float   delay = M0/sr ,     The average delay in sec
    float   lfo_sweep = A,      The maximum delay swing
    float   lfo_speed = f,      The flanger rate(hz)