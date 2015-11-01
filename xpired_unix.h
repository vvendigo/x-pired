/* 
 * Several additional includes and defines which we need only for the linux
 * build
 */

#ifndef XPIRED_H
#define XPIRED_H
#endif

#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>

#ifndef PREFIX
#define PREFIX "/usr/local"
#endif

#ifndef SHARE_PREFIX
#define SHARE_PREFIX "/usr/local/share/xpired"
#endif

