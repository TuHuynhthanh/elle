#ifndef ETOILE_NEST_POD_HH
#define ETOILE_NEST_POD_HH

#include <elle/types.hh>
#include <nucleus/nucleus.hh> // XXX fwd.hh
#include <XXX/Placement.hh> // XXX fwd.hh
#include <XXX/Handle.hh> // XXX fwd.hh

namespace etoile
{
  namespace nest
  {

    ///
    /// XXX
    ///
    class Pod:
      public elle::radix::Object
    {
    public:
      //
      // enumerations
      //
      enum Nature
        {
          NatureUnknown,
          NatureVolatile,
          NaturePersistent,
          NatureOrphan
        };

      enum State
        {
          StateUnloaded,
          StateLoaded
        };

      //
      // constructors & destructors
      //
      Pod();
      Pod(const nucleus::Placement&,
          nucleus::Block*);
      Pod(const nucleus::Placement&,
          const nucleus::Address&);
      Pod(const Pod&);
      ~Pod();

      //
      // methods
      //

      elle::Status              Load(nucleus::Handle&);
      elle::Status              Unload(nucleus::Handle&);

      //
      // interfaces
      //

      // object
      declare(Pod);

      // dumpable
      elle::Status              Dump(const elle::Natural32 = 0) const;

      //
      // attributes
      //
      Nature                    nature;
      State                     state;

      nucleus::Placement        placement;
      nucleus::Address          address;

      nucleus::Block*           block;

      elle::Natural32           counter;
    };

  }
}

#endif
