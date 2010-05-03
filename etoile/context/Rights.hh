//
// ---------- header ----------------------------------------------------------
//
// project       etoile
//
// license       infinit
//
// file          /home/mycure/infinit/etoile/context/Rights.hh
//
// created       julien quintard   [tue feb  2 16:59:43 2010]
// updated       julien quintard   [mon may  3 17:37:19 2010]
//

#ifndef ETOILE_CONTEXT_RIGHTS_HH
#define ETOILE_CONTEXT_RIGHTS_HH

//
// ---------- includes --------------------------------------------------------
//

#include <elle/Elle.hh>

#include <etoile/kernel/Role.hh>
#include <etoile/kernel/Record.hh>

namespace etoile
{
  namespace context
  {

//
// ---------- classes ---------------------------------------------------------
//

    ///
    /// this class represents the role, permissions and access key to the
    /// object and its contents.
    ///
    class Rights:
      public elle::Entity
    {
    public:
      //
      // constructors & destructors
      //
      Rights();

      //
      // interfaces
      //

      // dumpable
      elle::Status	Dump(const elle::Natural32 = 0) const;

      //
      // attributes
      //
      kernel::Role		role;
      kernel::Record		record;
      elle::SecretKey		key;
    };

  }
}

#endif
