#ifndef ONE_EURO_FILTER_H
#define ONE_EURO_FILTER_H

#include <time.h>
#include <math.h>

struct low_pass_filter {
    low_pass_filter() : hatxprev(0), hadprev(false) {}
    float operator()(float x, float alpha) {
        float hatx;
        if (hadprev) {
            hatx = alpha * x + (1 - alpha) * hatxprev;
        } else {
            hatx = x;
            hadprev = true;
        }
        hatxprev = hatx;
        return hatx;
    }
    float hatxprev;
    bool hadprev;
};

struct OneEuro {
    OneEuro(float _freq, float _mincutoff, float _beta, float _dcutoff);
    float operator()(float x, time_t t = -1);

    float mincutoff, beta, dcutoff;

private:
    float alpha(float cutoff);

    time_t last_time_;
    float dt;
    low_pass_filter xfilt_, dxfilt_;
};

#endif
