//
// ---------- header ----------------------------------------------------------
//
// project       etoile
//
// license       infinit
//
// file          /home/mycure/infinit/etoile/kernel/Contents.hh
//
// created       julien quintard   [mon aug 10 12:07:15 2009]
// updated       julien quintard   [mon may  3 22:57:45 2010]
//

#ifndef ETOILE_KERNEL_CONTENTS_HH
#define ETOILE_KERNEL_CONTENTS_HH

//
// ---------- includes --------------------------------------------------------
//

#include <etoile/kernel/ContentHashBlock.hh>
#include <etoile/kernel/Size.hh>

namespace etoile
{
  namespace kernel
  {

//
// ---------- classes ---------------------------------------------------------
//

    ///
    /// this class abstracts the data, catalog or reference by embedding
    /// it since the contents is always encrypted.
    ///
    /// therefore, once a contents is retrieved from the hole, the Extract()
    /// method is called which basically extracts an archive i.e the encrypted
    /// version of the embedded block. Then the Decrypt() method can be called
    /// in order to  (i) decrypt the embedded archive (ii) extract it (iii)
    /// build the embedded unencrypted object.
    ///
    /// the _cipher_ attribute contains the data once encrypted, often
    /// just before being stored in the hole since there is no benefit
    /// in encrypting the data for fun.
    ///
    template <typename T>
    class Contents:
      public ContentHashBlock
    {
    public:
      //
      // identifier
      //
      static const elle::String		Name;

      //
      // constructors & destructors
      //
      Contents();
      ~Contents();

      //
      // methods
      //
      elle::Status	Create();

      elle::Status	Encrypt(const elle::SecretKey&);
      elle::Status	Decrypt(const elle::SecretKey&);

      //
      // interfaces
      //

      // object
      declare(Contents<T>, _());

      // dumpable
      elle::Status	Dump(const elle::Natural32 = 0) const;

      // archivable
      elle::Status	Serialize(elle::Archive&) const;
      elle::Status	Extract(elle::Archive&);

      //
      // attributes
      //
      T*		content;

      elle::Cipher*	cipher;
    };

  }
}

//
// ---------- templates -------------------------------------------------------
//

#include <etoile/kernel/Contents.hxx>

#endif
