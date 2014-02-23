REACT-CPP
=========

REACT-CPP is an event loop library that utilizes the new C++11 lambda functions 
to notify you when there is activity on a filedescriptor or on a timer. 
Internally, it is a wrapper around the libev library, and does therefore also
depend on that library.


EVENT LOOP
==========

The React::Loop and the React::MainLoop classes are the central classes
of this library. These classes have methods to set timers and to register 
callback functions that will be called when a filedescriptor becomes readable 
or writable.

In a typical application you create an instance of the mainloop class, and
then you register filedescriptors that you'd like to watch for readability,
register event handlers and timers:

````c++
#include <reactcpp.h>
#include <unistd.h>
#include <iostream>

/**
 *  Main application procedure
 *  @return int
 */
int main()
{
    // create an event loop
    React::MainLoop loop;

    // set a timer to stop the application after five seconds
    loop.onTimeout(5.0, []() {
    
        // report that the timer expired
        std::cout << "timer expired" << std::endl;
    
        // stop the application
        exit(0);
    });
    
    // we'd like to be notified when input is available on stdin
    loop.onReadable(STDIN_FILENO, []() {
    
        // read input
        std::string buffer;
        std::cin >> buffer;
    
        // show what we read
        std::cout << buffer << std::endl;
    });

    // handler when control+c is pressed
    loop.onSignal(SIGINT, []() {
        
        // report that we got a signal
        std::cout << "control+c detected" << std::endl;
        
        // stop the application
        exit(0);
    });

    // run the event loop
    loop.run();

    // done
    return 0;
}
````

The above example contains a very simple echo application. Everything that
the application reads from stdin is directly echo'd back to stdout. After five
seconds the application automatically stops, and when the SIGINT signal is 
caught, the application also exits.

There is a subtle difference between the React::MainLoop and React::Loop
classes. The React::MainLoop is supposed to run the main event loop for the 
entire application, while the React::Loop classes are additional event loops
that you can (for example) use in additional threads. In normal circumstances, 
you will never have to instantiate more than once instance of the React::MainLoop 
class, while it is perfectly legal to create as many React::Loop objects
as you wish.

Because the React::MainLoop class is intended to control the entire application,
it has some additional methods to register signal handlers. Such methods are
not available in the regular React::Loop class.


DIFFERENT TYPES OF CALLBACKS
============================

In the first example we showed how to install handlers on the loop object.
However, once such a handler is set, the loop will keep calling it every time
a filedescriptor becomes active. But what if you no longer are interested in
these events? In that case you have a number of options to stop a callback 
from being called.

The first one is by using a different type of callback. In the examples above,
we had registered straightforward callbacks that did not take any parameters. 
However, a different type of callback can be registered too: one that takes
a parameter which you can modify to stop subsequent calls to the callback.

````c++
#include <reactcpp.h>
#include <unistd.h>
#include <iostream>

int main()
{
    // create the event loop
    React::MainLoop loop;
    
    // we'd like to be notified when input is available on stdin
    loop.onReadable(STDIN_FILENO, [](React::Reader *reader) {
    
        // read input
        std::string buffer;
        std::cin >> buffer;
    
        // show what we read
        std::cout << buffer << std::endl;
        
        // from this moment on, we no longer want to receive updates
        reader->cancel();
    });
    
    // run the event loop
    loop.run();
    
    // done
    return 0;
}
````

The program above is only interested in read events until the first line
from stdin has been read. After that it calls the cancel() method on the
React::Reader object, to inform the event loop that it is no longer interested
in read events.

In the above example, this also means that the program automatically exits after
the first line has been read. The reason for this is that the run() method of 
the React::Loop and React::MainLoop classes automatically stop running when 
there are no more callback functions active. By calling the Reader::cancel()
method, the last and only registered callback function is cancelled, and the
event loop has nothing left to monitor.

The type of the parameter that is passed to your callback function depends on
the type of callback function that you have registered.

````c++
loop.onReadable(fd, [](React::Reader *reader) { ... });
loop.onWritable(fd, [](React::Writer *writer) { ... });
loop.onTimeout(time, [](React::Timer *timer) { ... });
loop.onInterval(time, [](React::Interval *interval) { ... });
loop.onSignal(signum, [](React::Signal *signal) { ... });
````

The objects that are passed to your callback function all have in common that
they have a cancel() method that you can call to stop further events from
being received. Next to the cancel() method, additional methods are available
to deal with the specific behavior of the item being watched.


RETURN VALUE OF LOOP METHODS
============================

The objects that are passed to the callback functions (React::Reader, 
React::writer, etc) are also returned from the event-registering functions.
This means that it is possible to store the return value of a call to 
Loop::onReadable() in a variable, and that you will not have to wait for the
callback to be called to cancel further calls to it.

All Loop::onSomething() methods return a shared_ptr to the watcher object. 
You may store this shared_ptr if you'd like to use it in the future, but you do 
not have to. Internally, the library
also keeps a pointer to the object, and will pass on that pointer to your callback 
every time it is called. So even if you decide to discard the return value, the 
object will live on. The only way to stop the callback from being active is by 
calling the cancel() method on the returned object, or on the same object
inside your callback function.

With this knowledge we are going to modify our earlier example. The echo 
application that we showed before is updated to set the timer back to five
seconds every time that some input is read, so that the application will now
only stop after no input was detected for five seconds. We also change the
signal watcher: the moment CTRL+C is pressed, the application will stop 
responding, but it will only exit one second later.

````c++
#include <reactcpp.h>
#include <unistd.h>
#include <iostream>

/**
 *  Main application procedure
 *  @return int
 */
int main()
{
    // create an event loop
    React::MainLoop loop;

    // set a timer to stop the application if it is idle for five seconds
    // note that the type of 'timer' is std::shared_ptr<React::Timer>
    auto timer = loop.onTimeout(5.0, []() {
    
        // report that the timer expired
        std::cout << "timer expired" << std::endl;
    
        // stop the application
        exit(0);
    });
    
    // we'd like to be notified when input is available on stdin
    // the type of 'reader' is std::shared_ptr<React::Reader>
    auto reader = loop.onReadable(STDIN_FILENO, [timer]() {
    
        // read input
        std::string buffer;
        std::cin >> buffer;
    
        // show what we read
        std::cout << buffer << std::endl;
        
        // set the timer back to five seconds
        timer->set(5.0);
    });

    // handler when control+c is pressed
    loop.onSignal(SIGINT, [&loop, timer, reader]() {
        
        // report that we got a signal
        std::cout << "control+c detected" << std::endl;
        
        // both the timer, and the input checker can be cancelled now
        timer->cancel();
        reader->cancel();
        
        // stop the application in one second
        loop.onTimeout(1.0, [](Timer *timer) {
        
            // exit the application
            exit(0);
        });
    });

    // run the event loop
    loop.run();

    // done
    return 0;
}
````

CONSTRUCT WATCHER OBJECTS
=========================

Up to now we have registered callback methods via the Loop::onSomething()
methods. These methods return a shared pointer to an object that keeps the
watcher state. It is also possible to create such objects directly, without 
calling a Loop::onSomething method(). This can be very convenient, because
you will have ownership of the object (instead of the event loop) and you can 
unregister your handler function by just destructing the object.

````c++
#include <reactcpp.h>
#include <unistd.h>
#include <iostream>

/**
 *  Main application procedure
 *  @return int
 */
int main()
{
    // create an event loop
    React::MainLoop loop;

    // we'd like to be notified when input is available on stdin
    React::Reader reader(loop, STDIN_FILENO, [timer]() {
    
        // read input
        std::string buffer;
        std::cin >> buffer;
    
        // show what we read
        std::cout << buffer << std::endl;
    });

    // run the event loop
    loop.run();

    // done
    return 0;
}
````

Conceptually, there is not a big difference between calling Loop::onReadable()
to register a callback function, or by instantiating a React::Reader object
yourself. In my opinition, the code that utilizes a call to Loop::onReadable() 
is easier to understand and maintain, but by creating a Reader class yourself,
you have full ownership of the class and can destruct it whenever you like -
which can be useful too.


FILEDESCRIPTORS
===============

Filedescriptors can be checked for activity by registering callbacks for 
readability and writability. The loop object has two methods for that:

````c++
std::shared_ptr<Reader> Loop::onReadable(int fd, const ReadCallback &callback);
std::shared_ptr<Writer> Loop::onWritable(int fd, const WriteCallback &callback);
````

Two possible callback signatures are accepted, one that takes a pointer to a
watcher object (React::Reader* for the callback to onReadable(), and 
React::Writer* for the callback to onWritable()):

````c++
loop.onReadable(fd, [](Reader *reader) { ... });
loop.onReadable(fd, []() { ... });
loop.onWritable(fd, [](Writer *writer) { ... });
loop.onWritable(fd, []() { ... });
````

You can also create a Reader or Writer object yourself. In that case you will
not have to use the Loop::onReadable() or Loop::onWritable() methods:

````c++
Reader reader(&loop, fd, [](Reader *reader) { ... });
Reader reader(&loop, fd, []() { ... });
Writer writer(&loop, fd, [](Writer *writer) { ... });
Writer writer(&loop, fd, []() { ... });
````

TIMERS AND INTERVALS
====================

The React library supports both intervals and timers. A timer is triggered
only once, an interval on the other hand calls the registered callback method 
every time the interval time has expired.

When you create an interval, you can specify both the initial expire time as
well as the interval between all subsequent calls. If you ommit the initial time,
the callback will be first called after the first interval has passed.

````c++
std::shared_ptr<Timer> Loop::onTimeout(Timestamp seconds, const TimerCallback &callback);
std::shared_ptr<Interval> Loop::onInterval(Timestamp interval, const IntervalCallback &callback);
std::shared_ptr<Interval> Loop::onInterval(Timestamp initial, Timestamp interval, const IntervalCallback &callback);
````

Just like all other callbacks, the timer and interval callbacks also come in
two forms: with and without a parameter.

````c++
loop.onTimeout(3.0, [](Timer *timer) { ... });
loop.onTimeout(3.0, []() { ... });
loop.onInterval(5.0, [](Interval *interval) { ... });
loop.onInterval(5.0, []() { ... });
loop.onInterval(0.0, 5.0, [](Interval *interval) { ... });
loop.onInterval(0.0, 5.0, []() { ... });
````

And you can of course also instantiate React::Timer and React::Interval objects
directly:

````c++
Timer timer(&loop, 3.0, [](Timer *timer) { ... });
Timer timer(&loop, 3.0, []() { ... });
Interval(&loop, 5.0, [](Interval *interval) { ... });
Interval(&loop, 5.0, []() { ... });
Interval(&loop, 2.0, 5.0, [](Interval *interval) { ... });
Interval(&loop, 2.0, 5.0, []() { ... });
````
