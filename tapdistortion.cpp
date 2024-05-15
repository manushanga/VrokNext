#include "tapdistortion.h"

TapDistortion::TapDistortion() {
    is_active = false;
    srate = 0;
    meter = 0.f;
    rdrive = rbdr = kpa = kpb = kna = knb = ap = an = imr = kc = srct = sq = pwrq = prev_med = prev_out = 0.f;
    drive_old = blend_old = -1.f;
    over = 1;
}

TapDistortion::~TapDistortion() { }

void TapDistortion::activate() {
    is_active = true;
    set_params(0.f, 0.f);
}
void TapDistortion::deactivate() {
    is_active = false;
}

void TapDistortion::set_params(float blend, float drive) {
    // set distortion coeffs
    if ((drive_old != drive) || (blend_old != blend)) {
        rdrive = 12.0f / drive;
        rbdr = rdrive / (10.5f - blend) * 780.0f / 33.0f;
        kpa = D(2.0f * (rdrive * rdrive) - 1.0f) + 1.0f;
        kpb = (2.0f - kpa) / 2.0f;
        ap = ((rdrive * rdrive) - kpa + 1.0f) / 2.0f;
        kc = kpa / D(2.0f * D(2.0f * (rdrive * rdrive) - 1.0f) - 2.0f * rdrive * rdrive);

        srct = (0.1f * srate) / (0.1f * srate + 1.0f);
        sq = kc * kc + 1.0f;
        knb = -1.0f * rbdr / D(sq);
        kna = 2.0f * kc * rbdr / D(sq);
        an = rbdr * rbdr / sq;
        imr = 2.0f * knb + D(2.0f * kna + 4.0f * an - 1.0f);
        pwrq = 2.0f / (imr + 1.0f);

        drive_old = drive;
        blend_old = blend;
    }
}

void TapDistortion::set_sample_rate(uint32_t sr) {
    srate = sr;
    over = srate * 2 > 96000 ? 1 : 2;
    // resampler.set_params(srate, over, 2);
}
#include <iostream>
float TapDistortion::process(float in) {
    // double *samples = resampler.upsample((double)in);
    meter = 0.f;
    // for (int o = 0; o < over; o++) {
    float proc = in; // samples[o];
    float med;
    if (proc >= 0.0f) {
        med = (D(ap + proc * (kpa - proc)) + kpb) * pwrq;
    } else {
        med = (D(an - proc * (kna + proc)) + knb) * pwrq * -1.0f;
    }
    proc = srct * (med - prev_med + prev_out);
    prev_med = M(med);
    prev_out = M(proc);
    // samples[o] = proc;

    // std::cout<<med<<std::endl;
    meter = std::max(meter, proc);
    //}
    // float out = (float)resampler.downsample(samples);
    return proc; // out;
}

float TapDistortion::get_distortion_level() {
    return meter;
}
