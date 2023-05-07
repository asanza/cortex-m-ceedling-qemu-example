/*
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 * 
 * Copyright(c) 2023 Diego Asanza <f.asanza@gmail.com>
 */

#ifndef F505982C_3F8F_44F0_886B_950DD965EFD5
#define F505982C_3F8F_44F0_886B_950DD965EFD5

/**
 * @brief Second order IIR filter structure.
 */
struct iir2 {
    float a[2];
    float b[3];
};

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
iir2_apply(const struct iir2 *f, float *dl, float x);

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
ap1_apply(float b, float *dl, float x);

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
ap2_apply(float c3, float c2, float* dl, float x);

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
dc_block(float b, float* dl, float x);

#endif /* F505982C_3F8F_44F0_886B_950DD965EFD5 */
