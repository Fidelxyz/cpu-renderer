#pragma once
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

__attribute_used__ static float smoothstep(const float x, const float t1, const float t2) {
    if (x <= t1) return 0.f;
    if (x >= t2) return 1.f;
    float k = (x - t1) / (t2 - t1);
    return k * k * (3.f - 2.f * k);
}

#endif