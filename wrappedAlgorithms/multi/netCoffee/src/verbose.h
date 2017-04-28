/* verbose.h
Author: Jialu Hu
Date: 06.07.2012*/

#ifndef VERBOSE_H_
#define VERBOSE_H_

typedef enum {
               VERBOSE_NONE = 0, 
               VERBOSE_ESSENTIAL, 
               VERBOSE_NON_ESSENTIAL, 
               VERBOSE_DEBUG
             } VerbosityLevel;

extern VerbosityLevel g_verbosity;

#endif /* VERBOSE_H_ */
