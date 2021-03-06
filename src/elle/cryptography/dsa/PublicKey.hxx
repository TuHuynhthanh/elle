#ifndef ELLE_CRYPTOGRAPHY_DSA_PUBLICKEY_HXX
# define ELLE_CRYPTOGRAPHY_DSA_PUBLICKEY_HXX

//
// ---------- Hash ------------------------------------------------------------
//

# include <elle/serialization/binary.hh>

namespace std
{
  template <>
  struct hash<elle::cryptography::dsa::PublicKey>
  {
    size_t
    operator ()(elle::cryptography::dsa::PublicKey const& value) const
    {
      std::stringstream stream;
      {
        elle::serialization::binary::SerializerOut output(stream);
        output.serialize("value", value);
      }

      size_t result = std::hash<std::string>()(stream.str());

      return (result);
    }
  };
}

#endif
