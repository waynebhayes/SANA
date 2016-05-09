/* 
 * verbose.h
 *
 *  Created on: 13-feb-2011
 *      Author: M. El-Kebir
 */

#ifndef VERBOSE_H_
#define VERBOSE_H_

namespace nina {

typedef enum {
               VERBOSE_NONE,
               VERBOSE_ESSENTIAL, 
               VERBOSE_NON_ESSENTIAL, 
               VERBOSE_DEBUG
             } VerbosityLevel;

extern VerbosityLevel g_verbosity;

} // namespace nina

#endif /* VERBOSE_H_ */
