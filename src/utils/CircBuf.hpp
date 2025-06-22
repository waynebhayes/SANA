/* From "libwayne"
** Wayne Hayes, wayne@csri.utoronto.ca (preffered), or wayne@csri.toronto.edu
**
** Modified (and indicated as such) to add certaiun STAT tools. To prevent confusion, data stucture
** has been renamed to "CIRC_STAT"
** Marcus Longo, mlongo2021@gmail.com
*/

#ifdef __cplusplus
extern "C" {
#endif

// Circular buffer of DOUBLE values (which of course can be ints as well) for the purpose of computing a running mean.
// Note this does NOT include the functionality of "get next element", and overflow just overwrites the oldest element.
typedef struct _circ_stat {
    uint len, in, out, n;
    double *buf, sum, sum2;
} CIRC_STAT;

CIRC_STAT *CircStatAlloc(uint len); // len = physical size of the buffer //M: made argument unsigned to conform with style
void CircStatReset(CIRC_STAT *c);
double CircStatAdd(CIRC_STAT *c, double val); // returns the mean
double CircStatMean(CIRC_STAT *c);
double CircStatVariance(CIRC_STAT *c);
double CircStatConfInterval (CIRC_STAT *c, double confidence)

#ifdef __cplusplus
    }
#endif