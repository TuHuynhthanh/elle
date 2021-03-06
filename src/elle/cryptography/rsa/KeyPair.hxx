#ifndef ELLE_CRYPTOGRAPHY_RSA_KEYPAIR_HXX
# define ELLE_CRYPTOGRAPHY_RSA_KEYPAIR_HXX

# include <elle/cryptography/rsa/PublicKey.hh>

namespace std
{
  template <>
  struct hash<elle::cryptography::rsa::KeyPair>
  {
    size_t
    operator ()(elle::cryptography::rsa::KeyPair const& value) const
    {
      return (std::hash<elle::cryptography::rsa::PublicKey>()(value.K()));
    }
  };
}

#endif
