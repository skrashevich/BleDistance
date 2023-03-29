#include "one_euro_filter.h"

OneEuro::OneEuro(float _freq, float _mincutoff, float _beta, float _dcutoff)
    : mincutoff(_mincutoff), beta(_beta), dcutoff(_dcutoff), last_time_(-1)
{}

float OneEuro::operator()(float x, time_t t) {
    float dx = 0.0;

    if (last_time_ != -1 && t != -1 && t != last_time_) {
        dt = (t - last_time_) / 1e6;
    }
    last_time_ = t;

    if (xfilt_.hadprev)
        dx = (x - xfilt_.hatxprev) / dt;

    float edx = dxfilt_(dx, alpha(dcutoff));
    float cutoff = mincutoff + beta * abs(edx);
    return xfilt_(x, alpha(cutoff));
}

float OneEuro::alpha(float cutoff) {
    float tau = 1.0 / (2 * M_PI * cutoff);
    return 1.0 / (1.0 + tau / dt);
}
