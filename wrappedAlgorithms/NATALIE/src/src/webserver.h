/*
 * webserver.h
 *
 *  Created on: 16-apr-2013
 *      Author: M. El-Kebir
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

namespace nina {
namespace gna {

typedef enum {
               WEBSERVER_NONE,
               WEBSERVER_NATALIE,
               WEBSERVER_AMC
             } WebserverType;

extern WebserverType g_webserver;

} // namespace gna
} // namespace nina

#endif // WEBSERVER_H
