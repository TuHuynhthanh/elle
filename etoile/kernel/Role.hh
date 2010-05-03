//
// ---------- header ----------------------------------------------------------
//
// project       etoile
//
// license       infinit
//
// file          /home/mycure/infinit/etoile/kernel/Role.hh
//
// created       julien quintard   [sun apr  4 14:38:46 2010]
// updated       julien quintard   [mon may  3 16:58:29 2010]
//

#ifndef ETOILE_KERNEL_ROLE_HH
#define ETOILE_KERNEL_ROLE_HH

//
// ---------- includes --------------------------------------------------------
//

#include <elle/Elle.hh>

namespace etoile
{
  namespace kernel
  {

//
// ---------- types -----------------------------------------------------------
//

    ///
    /// this defines the type that is used to represent which role
    /// a user can play on an object.
    ///
    typedef elle::Natural8		Role;

//
// ---------- constants -------------------------------------------------------
//

    ///
    /// these are the role constants.
    ///
    const Role				RoleUnknown = 0x0;
    const Role				RoleOwner = 0x1;

    const Role				RoleDelegate = 0x4;
    const Role				RoleVassal = 0x8;

  }
}

#endif
