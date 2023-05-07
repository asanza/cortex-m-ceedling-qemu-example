#include "iir.h"

/**
 * @brief Apply a second order filter to the input signal.
 *        the filter transfer function is:
 * 
 *                b[0] + b[1] * z^-1 + b[2] * z^-2
 *        H(z) = ----------------------------------
 *                 1   + a[0] * z^-1 + a[1] * z^-2
 * 
 * @param f  the filter coeficients
 * @param dl delay line (2 elements). Must be preallocated.
 * @param x  input sample (incomming signal)
 * @return float  output sample (filtered signal)
 */
float
iir2_apply(const struct iir2 *f, float *dl, float x)
{
    float y = f->b[0] * x + dl[0];
    dl[0] = f->b[1] * x - f->a[0] * y + dl[1];
    dl[1] = f->b[2] * x - f->a[1] * y;
    return y;
}
/**
 * @brief Apply a first order allpass filter to the input signal.
 * 
 *                  b - z^-1
 *        H(z) = ----------------
 *                  1 - b * z^-1
 * 
 * @param b  filter coeficient.
 * @param dl  delay line (1 element)
 * @param x  input sample
 * @return float  output sample
 */
float
ap1_apply(float b, float *dl, float x)
{
    float t = *dl;
    float y = (x - t) * b;
    *dl     = x + y;
    return y + t;
}

/**
 * @brief Apply a biquad allpass filter.
 *        The biquad allpass is as follows:
 * 
 *                c3 + c2 * z^-1 + z^-2
 *       H(z) = -----------------------------
 *                1 + c2 * z^-1  + c3 * z^-2 
 * @param c3 
 * @param c2 
 * @param dl 
 * @param x 
 * @return float 
 */
float 
ap2_apply(float c3, float c2, float* dl, float x) {
    float t = c3 * ( x + dl[1] ) + c2 * dl[0];
    float y = t + dl[1];
    dl[1] = dl[0];
    dl[0] = x - t;
    return y;
}

/**
 * @brief Apply a dc block filter 
 *             1  - z^-1
 *        y = ------------
 *             1 - b * z^-1
 * @param b   blocker coefficient 
 * @param dl  delay line, must have 2 elements
 * @param x   input sample
 * @return float 
 */
float
dc_block(float b, float* dl, float x) {
    float y = x - dl[0] + b * dl[1];
    dl[0] = x;
    dl[1] = y;
    return y;
}
