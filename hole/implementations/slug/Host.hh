#ifndef HOLE_IMPLEMENTATIONS_SLUG_HOST_HH
# define HOLE_IMPLEMENTATIONS_SLUG_HOST_HH

# include <elle/types.hh>

# include <elle/network/fwd.hh>
# include <elle/concurrency/fwd.hh>
# include <elle/concurrency/Signal.hh>
# include <elle/network/Locus.hh>


# include <hole/Label.hh>

namespace hole
{
  namespace implementations
  {
    namespace slug
    {

      ///
      /// XXX
      ///
      class Host:
        public elle::radix::Entity
      {
      public:
        //
        // enumerations
        //
        enum State
          {
            StateUnknown,
            StateConnected,
            StateAuthenticated,
            StateDead
          };

        //
        // constants
        //
        static const elle::Natural32            Timeout;

        //
        // constructors & destructors
        //
        Host(const elle::network::Locus& locus);
        Host(elle::network::TCPSocket* connection);
        ~Host();

        //
        // methods
        //
        elle::Status    Disconnect();

        elle::Status    Authenticated();

        //
        // callbacks
        //
        elle::Status    Connected();
        elle::Status    Disconnected();
        // XXX: const string&
        elle::Status    Error(elle::String);
        elle::Status    Abort();

        //
        // interfaces
        //

        // dumpable
        elle::Status    Dump(const elle::Natural32 = 0) const;

        //
        // signals
        //
        struct
        {
          elle::concurrency::Signal<
            elle::radix::Parameters<
              Host*
              >
            >                   dead;
        }                       signal;

        //
        // attributes
        //
        State                   state;

        elle::network::Locus             locus;
        elle::network::TCPSocket*        socket;
      };

    }
  }
}

#endif
