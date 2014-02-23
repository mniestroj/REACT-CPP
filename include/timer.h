/**
 *  Timer.h
 *
 *  Timer that fires once
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
class Timer : private Watcher
{
private:
    /**
     *  Pointer to the loop
     *  @var    Loop
     */
    Loop *_loop;

    /**
     *  IO resource
     *  @var    ev_timer
     */
    struct ev_timer _watcher;

    /**
     *  Callback function
     *  @var    TimerCallback
     */
    TimerCallback _callback;

    /**
     *  Is the timer active?
     *  @var    bool
     */
    bool _active = false;

    /**
     *  When should it expired?
     *  @var    Timestamp
     */
    Timestamp _expire;

    /**
     *  Initialize the object
     *  @param  timeout
     */
    void initialize(Timestamp timeout);

protected:
    /**
     *  Invoke the callback
     */
    virtual void invoke() override
    {
        // is this indeed the expiration time?
        if (_expire <= _loop->now())
        {
            // timer is no longer active
            _active = false;
            
            // notify parent
            _callback(this);
        }
        else
        {
            // we set a new timer
            set(_expire - _loop->now());
        }
    }

public:
    /**
     *  Constructor
     *  @param  loop        Event loop
     *  @param  timeout     Timeout period
     *  @param  callback    Function that is called when timer is expired
     */
    Timer(Loop *loop, Timestamp timeout, const TimerCallback &callback) : _loop(loop), _callback(callback)
    {
        // store pointer to current object
        _watcher.data = this;

        // initialize the watcher
        initialize(timeout);

        // start the timer
        start();
    }

    /**
     *  No copying or moving allowed
     *  @param  that
     */
    Timer(const Timer &that) = delete;
    Timer(Timer &&that) = delete;

    /**
     *  Destructor
     */
    virtual ~Timer() 
    {
        // cancel the timer
        cancel();
    }

    /**
     *  No copying or moving
     *  @param  that
     */
    Timer &operator=(const Timer &that) = delete;
    Timer &operator=(Timer &&that) = delete;
    
    /**
     *  Start the timer
     *  @return bool
     */
    virtual bool start()
    {
        // skip if already running
        if (_active) return false;
        
        // start now
        ev_timer_start(*_loop, &_watcher);
        
        // remember that it is active
        return _active = true;
    }
    
    /**
     *  Cancel the timer
     *  @return bool
     */
    virtual bool cancel()
    {
        // skip if not running
        if (!_active) return false;
        
        // stop now
        ev_timer_stop(*_loop, &_watcher);
        
        // remember that it no longer is active
        _active = false;
        
        // done
        return true;
    }

    /**
     *  Set the timer to a new time
     *  @param  timeout
     *  @return bool
     */
    bool set(Timestamp timeout)
    {
        // is the timer still active, and is the new timeout after the current timeout?
        // in that case we won't reset the timer to not interfere with the event loop,
        // but only update the expire time, when the timer then expires, the callback
        // will not be called, but a new timer is going to be set instead
        if (_active && _loop->now() + timeout < _expire)
        {
            // set the new expire time
            _expire = _loop->now() + timeout;
            
            // done
            return true;
        }
        else
        {
            // cancel the current time
            cancel();
            
            // set a new timer
            ev_timer_set(&_watcher, timeout, 0.0);
            
            // remember expire time
            _expire = _loop->now() + timeout;
            
            // start the timer
            return start();
        }
    }
}; 
 
/**
 *  End namespace
 */
}
