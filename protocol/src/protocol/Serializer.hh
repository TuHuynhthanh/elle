#ifndef INFINIT_PROTOCOL_PACKET_SERIALIZER_HH
# define INFINIT_PROTOCOL_PACKET_SERIALIZER_HH

# include <iostream>

# include <reactor/mutex.hh>

# include <protocol/Stream.hh>
# include <elle/attribute.hh>

namespace infinit
{
  namespace protocol
  {
    class Serializer
      : public Stream
    {
    /*------.
    | Types |
    `------*/
    public:
      typedef Stream Super;
      typedef std::iostream Inner;

    /*-------------.
    | Construction |
    `-------------*/
    public:
      Serializer(std::iostream& stream,
                 elle::Version const& version = elle::Version(0, 1, 0),
                 bool checksum = true);

      Serializer(reactor::Scheduler& scheduler,
                 std::iostream& stream,
                 elle::Version const& version  = elle::Version(0, 1, 0),
                 bool checksum = true);

    public:
      ~Serializer();

    /*----------.
    | Receiving |
    `----------*/
    public:
      elle::Buffer
      read() override;

    /*--------.
    | Sending |
    `--------*/
    protected:
      void
      _write(elle::Buffer const& packet) override;

    /*----------.
    | Printable |
    `----------*/
    public:
      void
      print(std::ostream& stream) const override;

    /*--------.
    | Details |
    `--------*/
      ELLE_ATTRIBUTE_R(elle::Version, version);
      ELLE_ATTRIBUTE_R(elle::Buffer::Size, chunk_size);
      ELLE_ATTRIBUTE_R(bool, checksum);
    public:
      class pImpl;
    private:
      ELLE_ATTRIBUTE(std::unique_ptr<pImpl>, impl);
    };
  }
}

#endif
