/*
MUMT 618 Homework3
Author: Wan Yi Lin

Folder structure
- flanger.cpp
- README
- stk/

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
*/
#include "DelayL.h"
#include "SineWave.h"
#include "FileLoop.h"
#include "FileWvOut.h"
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <utility>
#include <algorithm>

using namespace stk;
using namespace std;

pair<StkFrames, float> flangerMono(int& channel, StkFrames& sine_out,
                 unsigned long& i_sp_points, 
                 int& lfo_avg_delay, float& lfo_sweep, float& lfo_speed,
                 float& depth,
                 FileWvIn& input
                 ){

    input.reset();
    // Create a delay line with linear interpolation
    DelayL delayline;

    StkFrames Y(i_sp_points, 1);
    cout << "Computing channel" << channel << endl;

    // Process
    StkFloat max_val = 0.0;
    for (auto i = 0; i < input.getSize(); i++){
        // Read
        StkFloat w_in = input.tick(channel);

        // Delay with linear interpolation
        float _delay = lfo_avg_delay * (1.0 + lfo_sweep*sine_out[i]);
        delayline.setDelay(_delay);
        StkFloat delay_out = delayline.tick(w_in);

        // Add
        StkFloat out = w_in + delay_out * depth;
        Y[i] = out;

        if (abs(out) > max_val){
            max_val = abs(out);
        }
    }
    return make_pair(Y, max_val+1e-7);
}

void usage() {
  std::cout << "Usage: ./flanger <input .wav>" << endl;
  exit( 0 );
}

int main(int argc, char* argv[]){
    if ( argc != 2 ) usage();

    // Read the input file.
    FileWvIn input;
    try{
        input.openFile(argv[1]);
    }
    catch (StkError& stk_e){
        cerr << stk_e.getMessage() << endl;
        exit(1);
    }

    auto i_channel = input.channelsOut();
    auto i_sp_points = input.getSize();
    auto i_sr = input.getFileRate();
    std::cout << "Read from " << argv[1] << std::endl;
    cout << "Channel/length/sr : " << i_channel << "/" << i_sp_points << "/" << i_sr << endl << endl;

    Stk::setSampleRate(i_sr);

    // Comb
    /*
    y[n] = x[n] + gx[n-M[n]]
    M[n] = M0(1 + Asin(2pifnT))
    */
    float depth = 1.0; // g, [0.0, 1.0]
    bool phase_inversion = false;
    depth = max(float(0.0), min(float(1.0), depth));
    depth = phase_inversion ? -depth : depth;
    cout << "gain " << depth << endl;

    // LFO 
    float   delay = 0.010; // average delay: 0.010 sec
    int     lfo_avg_delay = int(delay * i_sr); // M0, average delay length
    float   lfo_sweep = 0.5; // A, maximum delay swing
    float   lfo_speed = 0.5; // f, rate(Hz?)

    SineWave sine;
    sine.setFrequency(lfo_speed);
    StkFrames sine_out(i_sp_points, 1);
    sine.tick(sine_out);

    // Compute
    StkFrames Y(i_sp_points, i_channel);
    float max_val = 0.0;
    
    for (auto i = 0; i < i_channel; i++){
        auto tmp = flangerMono(i, sine_out, i_sp_points, lfo_avg_delay,
                                    lfo_sweep, lfo_speed, depth, input);
        Y.setChannel(i, tmp.first, 0);
        if (tmp.second > max_val){
            max_val = tmp.second;
        }
    }
    
    // Normalize
    for (auto i = 0; i < Y.size(); i++){
        Y[i] = Y[i] / max_val;
    }

    // Write to file
    string filename(argv[1]);
    filename.replace(filename.length()-4, 4, "_flanger.wav");
    FileWvOut output;
    try {
        output.openFile( filename, i_channel, FileWrite::FILE_WAV, Stk::STK_SINT16 );
    }
    catch ( StkError & stk_e) {
        cerr << stk_e.getMessage() << endl;
        exit(1);
    }

    output.tick(Y);

    cout << "END" << endl;
    return 0;
}