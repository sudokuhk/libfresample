/* Copyright 2012 Dietrich Epp <depp@zdome.net> */
#include "resample.h"

void
lfr_resample_s16n1s16_scalar(
    lfr_fixed_t *pos, lfr_fixed_t inv_ratio, unsigned *dither,
    void *out, int outlen, const void *in, int inlen,
    const struct lfr_filter *filter)
{
    int i, j, acc, f, log2nfilt, fn, ff0, ff1, off, flen, fidx0, fidx1;
    const short *fd, *inp = in;
    short *outp = out;
    lfr_fixed_t x;
    unsigned ds;

    fd = filter->data;
    flen = filter->nsamp;
    log2nfilt = filter->log2nfilt;
    x = *pos;
    ds = *dither;

    for (i = 0; i < outlen; ++i) {
        /* fn: filter number
           ff0: filter factor for filter fn
           ff1: filter factor for filter fn+1 */
        fn = (((unsigned) x >> 1) >> (31 - log2nfilt)) &
            ((1u << log2nfilt) - 1);
        ff1 = ((unsigned) x >> (32 - log2nfilt - INTERP_BITS)) &
            ((1u << INTERP_BITS) - 1);
        ff0 = (1u << INTERP_BITS) - ff1;

        /* off: offset in input corresponding to first sample in filter */
        off = (int) (x >> 32);
        /* fidx0, fidx1: start, end indexes in FIR data */
        fidx0 = -off;
        if (fidx0 < 0)
            fidx0 = 0;
        fidx1 = inlen - off;
        if (fidx1 > flen)
            fidx1 = flen;

        acc = 0;
        for (j = fidx0; j < fidx1; ++j) {
            f = (fd[(fn+0) * flen + j] * ff0 +
                 fd[(fn+1) * flen + j] * ff1) >> INTERP_BITS;
            acc += inp[j + off] * f;
        }
        acc += (int) (ds >> 17);
        acc >>= 15;
        if (acc > 0x7fff)
            acc = 0x7fff;
        else if (acc < -0x8000)
            acc = -0x8000;
        outp[i] = acc;

        x += inv_ratio;
        ds = LCG_A * ds + LCG_C;
    }

    *pos = x;
    *dither = ds;
}
