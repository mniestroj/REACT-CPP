/**
 *  Reader.cpp
 *
 *  @copyright 2014 Copernica BV
 */
#include "includes.h"

/**
 *  Set up namespace
 */
namespace React {

/**
 *  Function that gets called when the filedescriptor is active
 *  @param  loop
 *  @param  watcher
 *  @param  events
 */
static void onActive(struct ev_loop *loop, ev_io *watcher, int events)
{
    // retrieve the reader
    Watcher *object = (Watcher *)watcher->data;

    // call it
    object->invoke();
}

/**
 *  Initialize
 *  @param  fd
 *  @param  flags
 */
void Reader::initialize(int fd)
{
    // initialize the watcher
    ev_io_init(&_watcher, onActive, fd, EV_READ);
}

/**
 *  End namespace
 */    
}
