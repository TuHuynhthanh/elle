//
// ---------- header ----------------------------------------------------------
//
// project       etoile
//
// license       infinit
//
// file          /home/mycure/infinit/etoile/context/Directory.hh
//
// created       julien quintard   [fri aug 14 23:13:51 2009]
// updated       julien quintard   [mon may  3 17:33:10 2010]
//

#ifndef ETOILE_CONTEXT_DIRECTORY_HH
#define ETOILE_CONTEXT_DIRECTORY_HH

//
// ---------- includes --------------------------------------------------------
//

#include <etoile/context/Object.hh>

#include <etoile/kernel/Contents.hh>
#include <etoile/kernel/Catalog.hh>

namespace etoile
{
  namespace context
  {

//
// ---------- classes ---------------------------------------------------------
//

    ///
    /// this context represents a directory object as it embeds
    /// a catalog contents along with inherited object-related stuff.
    ///
    class Directory:
      public Object
    {
    public:
      //
      // types
      //
      typedef kernel::Catalog		Content;

      //
      // constructors & destructors
      //
      Directory();
      ~Directory();

      //
      // interfaces
      //

      // dumpable
      elle::Status		Dump(const elle::Natural32 = 0) const;

      //
      // attributes
      //
      kernel::Contents<Content>*	contents;
    };

  }
}

#endif
