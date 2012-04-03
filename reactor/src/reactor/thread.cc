#include <boost/foreach.hpp>

#include <reactor/debug.hh>
#include <reactor/scheduler.hh>
#include <reactor/signal.hh>
#include <reactor/sleep.hh>
#include <reactor/thread.hh>

namespace reactor
{
  /*-------------.
  | Construction |
  `-------------*/

  Thread::Thread(Scheduler& scheduler,
                 const std::string& name,
                 const Action& action)
    : _state(state::running)
    , _injection()
    , _exception(0)
    , _waited()
    , _timeout(false)
    , _thread(scheduler._manager, name, action)
    , _scheduler(scheduler)
  {
    _scheduler._thread_register(*this);
  }

  /*------.
  | State |
  `------*/

  Thread::State
  Thread::state() const
  {
    return _state;
  }

  bool
  Thread::done() const
  {
    return state() == state::done;
  }

  std::string
  Thread::name() const
  {
    return _thread.name();
  }

  /*----.
  | Run |
  `----*/

  void
  Thread::_step()
  {
    _thread.step();
    if (_thread.status() == backend::status::done)
    {
      _state = Thread::state::done;
      _signal();
    }
  }

  void
  Thread::yield()
  {
    _thread.yield();
    if (_injection)
    {
      Injection i(_injection);
      _injection = Injection();
      i();
    }
    if (_exception)
    {
      Exception* e = _exception;
      _exception = 0;
      e->raise_and_delete();
    }
  }

  void
  Thread::inject(const Injection& injection)
  {
    _injection = injection;
  }

  void
  Thread::sleep(Duration d)
  {
    Sleep sleep(_scheduler, d);
    sleep.run();
  }

  void
  Thread::raise(Exception* e)
  {
    _exception = e;
  }

  /*----------------.
  | Synchronization |
  `----------------*/

  bool
  Thread::wait(Waitable& s, boost::optional<Duration> timeout)
  {
    Waitables waitables(1, &s);
    return wait(waitables, timeout);
  }

  bool
  Thread::wait(Waitables& waitables, boost::optional<Duration> timeout)
  {
    assert(_state == state::running);
    assert(_waited.empty());
    bool freeze = false;
    BOOST_FOREACH (Waitable* s, waitables)
      if (s->_wait(this))
      {
        freeze = true;
        _waited.insert(s);
      }
    if (freeze)
    {
      INFINIT_REACTOR_DEBUG(*this << ": freeze");
      if (timeout)
      {
        boost::asio::deadline_timer _timer(_scheduler.io_service(),
                                           timeout.get());
        _timeout = false;
        _timer.async_wait(boost::bind(&Thread::_wait_timeout, this, _1));
        _freeze();
        if (_timeout)
          return false;
        else
        {
          _timer.cancel();
          return true;
        }
      }
      else
      {
        _freeze();
        return true;
      }
    }
    else
      return true;
  }

  bool
  Thread::_wait(Thread* thread)
  {
    if (_state == state::done)
      return false;
    else
      return Waitable::_wait(thread);
  }

  void
  Thread::_wait_timeout(const boost::system::error_code& e)
  {
    if (e == boost::system::errc::operation_canceled)
      return;
    assert(state() == state::frozen);
    INFINIT_REACTOR_DEBUG(*this << ": timed out");
    BOOST_FOREACH (Waitable* waitable, _waited)
      waitable->_unwait(this);
    _waited.clear();
    _timeout = true;
    _scheduler._unfreeze(*this);
    _state = Thread::state::running;
  }

  void
  Thread::_freeze()
  {
    _scheduler._freeze(*this);
    _state = Thread::state::frozen;
    yield();
  }

  void
  Thread::_wake(Waitable* waitable)
  {
    INFINIT_REACTOR_DEBUG(*this << ": wake one");
    if (waitable->_exception && !_exception)
      _exception = waitable->_exception;
    _waited.erase(waitable);
    if (_waited.empty())
    {
      _scheduler._unfreeze(*this);
      _state = Thread::state::running;
    }
  }

  std::ostream& operator << (std::ostream& s, const Thread& t)
  {
    s << "thread " << t.name();
    return s;
  }
}
