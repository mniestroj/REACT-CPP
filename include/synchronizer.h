/**
 *  Synchronizer.h
 *
 *  Object that can be used to synchroninze an event loop.
 *
 *  @copyright 2014 Copernica BV
 */

/**
 *  Set up namespace
 */
namespace React {

/**
 *  Class definition
 */
class Synchronizer : private Watcher
{
private:
    /**
     *  Pointer to the loop
     *  @var    Loop
     */
    Loop *_loop;

    /**
     *  IO resource
     *  @var    ev_async
     */
    struct ev_async _watcher;

    /**
     *  Callback function
     *  @var    SynchroninzeCallback
     */
    SynchronizeCallback _callback;

    /**
     *  Initialize the object
     */
    void initialize();

    /**
     *  Invoke the callback
     */
    virtual void invoke() override
    {
        _callback(this);
    }

public:
    /**
     *  Constructor
     *  @param  loop        Event loop
     *  @param  callback    Function that is called when synchronizer is activated
     */
    Synchronizer(Loop *loop, const SynchronizeCallback &callback) :
        _loop(loop), _callback(callback)
    {
        // store pointer to current object
        _watcher.data = this;

        // initialize the watcher
        initialize();
        
        // start right away
        ev_async_start(*_loop, &_watcher);
    }
    
    /**
     *  No copying or moving allowed
     *  @param  that
     */
    Synchronizer(const Synchronizer &that) = delete;
    Synchronizer(Synchronizer &&that) = delete;
    
    /**
     *  Destructor
     */
    virtual ~Synchronizer() 
    {
        // destructor
        ev_async_stop(*_loop, &_watcher);
    }

    /**
     *  No copying or moving
     *  @param  that
     */
    Synchronizer &operator=(const Synchronizer &that) = delete;
    Synchronizer &operator=(Synchronizer &&that) = delete;
    
    /**
     *  Synchronize with the event loop
     * 
     *  This is a thread safe method, that is normally called from an other thread.
     *  
     *  After you've called synchronize, a call to the registered callback will
     *  be soon executed in the event loop in which the synchronizer was created.
     * 
     *  @return bool 
     */
    bool synchronize()
    {
        ev_async_send(*_loop, &_watcher);
        return true;
    }
    
    /**
     *  Invoke operator does the same as the synchronize() method
     * 
     *  @return bool
     */
    bool operator () ()
    {
        return synchronize();
    }
}; 
 
/**
 *  End namespace
 */
}
