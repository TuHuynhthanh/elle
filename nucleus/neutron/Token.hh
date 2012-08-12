#ifndef NUCLEUS_NEUTRON_TOKEN_HH
# define NUCLEUS_NEUTRON_TOKEN_HH

# include <elle/types.hh>
# include <elle/radix/Object.hh>

# include <elle/cryptography/fwd.hh>

namespace nucleus
{
  namespace neutron
  {

    /// A token is a secret information enabling a user to access encrypted
    /// data. However, in order to allow only the proper user to
    /// use this secret information, it is encrypted with the user's public key.
    class Token:
      public elle::radix::Object
    {
      //
      // constants
      //
    public:
      /// Defines an empty/unused token.
      static const Token Null;

      //
      // constructors & destructors
      //
    public:
      Token();
      template <typename T>
      Token(elle::cryptography::PublicKey const& K,
            T const& secret);
      Token(const Token&);
      ~Token();

      //
      // methods
      //
    public:
      /// XXX[to rename]
      template <typename T>
      elle::Status
      Update(T const&,
             elle::cryptography::PublicKey const&);
      /// Extracts the secret information from the token by decrypting
      /// the code with the given private key which only the user has.
      template <typename T>
      elle::Status
      Extract(elle::cryptography::PrivateKey const&,
              T&) const;
      /// Returns the code pointer.
      elle::cryptography::Code const*
      code() const;

      //
      // interfaces
      //
    public:
      // object
#include <elle/idiom/Open.hh>
      declare(Token);
#include <elle/idiom/Close.hh>
      elle::Boolean     operator==(const Token&) const;

      // dumpable
      elle::Status      Dump(const elle::Natural32 = 0) const;

      // serializable
      ELLE_SERIALIZE_FRIEND_FOR(Token);

      //
      // attributes
      //
    public:
      elle::cryptography::Code* _code;
    };

  }
}

#include <nucleus/neutron/Token.hxx>

#endif
