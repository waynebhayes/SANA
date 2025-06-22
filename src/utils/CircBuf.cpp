/* From "libwayne"
** Wayne Hayes, wayne@csri.utoronto.ca (preffered), or wayne@csri.toronto.edu
**
** Modified (and indicated as such with M:) to add STAT tools
** Marcus Longo, mlongo2021@gmail.com
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "Misc.hpp"
#include "Stats.hpp" // M: We want this for the confidence interval calculation
#include "CircBuf.hpp"
#include <math.h> // M: ditto

CIRC_STAT *CircStatAlloc(int len) {
    CIRC_STAT *c = (CIRC_STAT*)Calloc(sizeof(CIRC_STAT),1); // M: Added cast
    c->len = len;
    c->buf = (double*)Calloc(len, sizeof(double));
    c->sum = c->sum2 = 0.0; // M: Sum of squares added
    c->in = c->n = 0;
    return c;
}

void CircStatReset(CIRC_STAT *c) {
    c->sum = c->sum2 = 0.0; c->n = c->in = 0; // M: Now resets sum of squares
}

// returns the mean
double CircStatAdd(CIRC_STAT *c, double val) {
    int next = c->in == c->len - 1 ? 0 : c->in + 1;
    if (c->n == c->len) {
        c->sum -= c->buf[next];
        c->sum2 -= c->buf[next] * c->buf[next];
    }
    else c->n++;
    c->in = next;
    c->buf[next] = val;
    c->sum += val;
    c->sum2 += val * val;
    return c->sum / c->n;
}

double CircStatMean(CIRC_STAT *c) {
    assert(c->n > 0);
    return c->sum / c->n;
}

// M: Taken from stat.c in libwayne
double CircStatVariance(CIRC_STAT *c) {
    assert(c->n > 0);
  return fabs(c->sum2 - c->sum*c->sum / c->n) / (c->n - 1);
}
// M: Taken from stat.c in libwayne
double CircStatConfInterval (CIRC_STAT *c, double confidence) {
    return StatTDistP2Z((1-confidence)/2, c->n - 1) * sqrt(CircStatVariance(c) / c->n);
}

#ifdef __cplusplus
    }
#endif