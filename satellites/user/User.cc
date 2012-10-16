#include <satellites/user/User.hh>

#include <Infinit.hh>

#include <lune/Lune.hh>

#include <etoile/Etoile.hh>

#include <elle/cryptography/PublicKey.hh>
#include <elle/io/Console.hh>
#include <elle/io/Piece.hh>
#include <elle/io/Directory.hh>
#include <elle/utility/Parser.hh>
#include <elle/concurrency/Program.hh>

#include <lune/Identity.hh>
#include <elle/Authority.hh>
#include <lune/Dictionary.hh>

namespace satellite
{
  ///
  /// this method creates a new user by generating a new key pair and
  /// storing a user block.
  ///
  elle::Status          User::Create(elle::String const&        id,
                                     const elle::String&        name)
  {
    elle::String        prompt;
    elle::String        pass;
    elle::cryptography::KeyPair       pair;
    elle::Authority     authority;
    lune::Identity      identity;
    lune::Dictionary    dictionary;

    // check the argument.
    if (name.empty() == true)
      escape("unable to create a user without a user name");

    // check if the user already exists.
    if (lune::Identity::exists(name) == true)
      escape("this user seems to already exist");

    // check if the authority exists.
    if (elle::Authority::exists(elle::io::Path(lune::Lune::Authority)) == false)
      escape("unable to locate the authority file");

    // prompt the user for the passphrase.
    prompt = "Enter passphrase for the authority: ";

    if (elle::io::Console::Input(
          pass,
          prompt,
          elle::io::Console::OptionPassword) == elle::Status::Error)
      escape("unable to read the input");

    // load the authority.
    authority.load(elle::io::Path(lune::Lune::Authority));

    // decrypt the authority.
    if (authority.Decrypt(pass) == elle::Status::Error)
      escape("unable to decrypt the authority");

    // prompt the user for the passphrase.
    prompt = "Enter passphrase for keypair '" + name + "': ";

    if (elle::io::Console::Input(
          pass,
          prompt,
          elle::io::Console::OptionPassword) == elle::Status::Error)
      escape("unable to read the input");

    // generate a key pair.
    if (pair.Generate() == elle::Status::Error)
      escape("unable to generate the key pair");

    // create the identity.
    if (identity.Create(id, name, pair) == elle::Status::Error)
      escape("unable to create the identity");

    // encrypt the identity.
    if (identity.Encrypt(pass) == elle::Status::Error)
      escape("unable to encrypt the identity");

    // seal the identity.
    if (identity.Seal(authority) == elle::Status::Error)
      escape("unable to seal the identity");

    // store the identity.
    identity.store(name);

    // store an empty dictionary.
    dictionary.store(name);

    return elle::Status::Ok;
  }

  ///
  /// this method destroys an existing user.
  ///
  elle::Status          User::Destroy(const elle::String&       name)
  {
    //
    // remove the identity.
    //
    {
      lune::Identity    identity;

      // check the argument.
      if (name.empty() == true)
        escape("unable to destroy a user without a user name");

      // check if the user already exists.
      if (lune::Identity::exists(name) == false)
        escape("this user does not seem to exist");

      // destroy the identity.
      lune::Identity::erase(name);
    }

    //
    // remove the dictionary, if necessary.
    //
    {
      lune::Dictionary  dictionary;

      // if the dictionary exists...
      if (lune::Dictionary::exists(name) == true)
        lune::Dictionary::erase(name);
    }

    //
    // remove the user directory.
    //
    {
      elle::io::Path        path;

      // create the user path.
      if (path.Create(lune::Lune::User::Root) == elle::Status::Error)
        escape("unable to create the path");

      // complete the path with the user name.
      if (path.Complete(elle::io::Piece("%USER%", name)) == elle::Status::Error)
        escape("unable to complete the path");

      // clear the user directory content.
      if (elle::io::Directory::Clear(path) == elle::Status::Error)
        escape("unable to clear the directory");

      // remove the directory.
      if (elle::io::Directory::Remove(path) == elle::Status::Error)
        escape("unable to erase the directory");
    }

    return elle::Status::Ok;
  }

  ///
  /// this method displays information on the given user.
  ///
  elle::Status          User::Information(const elle::String&   name)
  {
    elle::String        prompt;
    elle::String        pass;
    lune::Identity      identity;
    elle::cryptography::PublicKey     K;
    elle::io::Unique        unique;

    // check the argument.
    if (name.empty() == true)
      escape("unable to create a user without a user name");

    // check if the user already exists.
    if (lune::Identity::exists(name) == false)
      escape("this user does not seem to exist");

    // prompt the user for the passphrase.
    prompt = "Enter passphrase for keypair '" + name + "': ";

    if (elle::io::Console::Input(
          pass,
          prompt,
          elle::io::Console::OptionPassword) == elle::Status::Error)
      escape("unable to read the input");

    // load the identity.
    identity.load(name);

    // verify the identity.
    if (identity.Validate(Infinit::Authority) == elle::Status::Error)
      escape("the identity seems to be invalid");

    // decrypt the identity.
    if (identity.Decrypt(pass) == elle::Status::Error)
      escape("unable to decrypt the identity");

    // dump the identity.
    if (identity.Dump() == elle::Status::Error)
      escape("unable to dump the identity");

    // retrieve the user's public key unique.
    if (identity.pair.K.Save(unique) == elle::Status::Error)
      escape("unable to save the public key's unique");

    // display the unique.
    std::cout << "[Unique] " << unique << std::endl;

    return elle::Status::Ok;
  }

  void
  User(elle::Natural32 argc, elle::Character* argv[])
  {
    User::Operation     operation;

    // set up the program.
    if (elle::concurrency::Program::Setup() == elle::Status::Error)
      throw std::runtime_error("unable to set up the program");

    // initialize the Lune library.
    if (lune::Lune::Initialize() == elle::Status::Error)
      throw std::runtime_error("unable to initialize Lune");

    // initialize Infinit.
    if (Infinit::Initialize() == elle::Status::Error)
      throw std::runtime_error("unable to initialize Infinit");

    // initialize the operation.
    operation = User::OperationUnknown;

    // allocate a new parser.
    Infinit::Parser = new elle::utility::Parser(argc, argv);

    // specify a program description.
    if (Infinit::Parser->Description(Infinit::Copyright) == elle::Status::Error)
      throw std::runtime_error("unable to set the description");

    // register the options.
    if (Infinit::Parser->Register(
          "Help",
          'h',
          "help",
          "display the help",
          elle::utility::Parser::KindNone) == elle::Status::Error)
      throw std::runtime_error("unable to register the option");

    // register the options.
    if (Infinit::Parser->Register(
          "Create",
          'c',
          "create",
          "create a user",
          elle::utility::Parser::KindNone) == elle::Status::Error)
      throw std::runtime_error("unable to register the option");

    // register the options.
    if (Infinit::Parser->Register(
          "Destroy",
          'd',
          "destroy",
          "destroy an existing network",
          elle::utility::Parser::KindNone) == elle::Status::Error)
      throw std::runtime_error("unable to register the option");

    // register the options.
    if (Infinit::Parser->Register(
          "Information",
          'x',
          "information",
          "display information regarding the user",
          elle::utility::Parser::KindNone) == elle::Status::Error)
      throw std::runtime_error("unable to register the option");

    // register the options.
    if (Infinit::Parser->Register(
          "Identifier",
          'i',
          "identifier",
          "specify the identity",
          elle::utility::Parser::KindOptional) == elle::Status::Error)
      throw std::runtime_error("unable to register the option");

    // register the options.
    if (Infinit::Parser->Register(
          "Name",
          'n',
          "name",
          "specify the user name",
          elle::utility::Parser::KindRequired) == elle::Status::Error)
      throw std::runtime_error("unable to register the option");

    // parse.
    if (Infinit::Parser->Parse() == elle::Status::Error)
      throw std::runtime_error("unable to parse the command line");

    // test the option.
    if (Infinit::Parser->Test("Help") == elle::Status::True)
      {
        // display the usage.
        Infinit::Parser->Usage();
        return;
      }

    // check the mutually exclusive options.
    if ((Infinit::Parser->Test("Create") == elle::Status::True) &&
        (Infinit::Parser->Test("Destroy") == elle::Status::True) &&
        (Infinit::Parser->Test("Information") == elle::Status::True))
      {
        // display the usage.
        Infinit::Parser->Usage();

        throw std::runtime_error("the create, destroy and information options are "
               "mutually exclusive");
      }

    // test the option.
    if (Infinit::Parser->Test("Create") == elle::Status::True)
      operation = User::OperationCreate;

    // test the option.
    if (Infinit::Parser->Test("Destroy") == elle::Status::True)
      operation = User::OperationDestroy;

    // test the option.
    if (Infinit::Parser->Test("Information") == elle::Status::True)
      operation = User::OperationInformation;

    // trigger the operation.
    switch (operation)
      {
      case User::OperationCreate:
        {
          elle::String          identifier;
          elle::String          name;

          // retrieve the name.
          if (Infinit::Parser->Value("Name", name) == elle::Status::Error)
            throw std::runtime_error("unable to retrieve the name value");

          // retrieve the identifier.
          if (Infinit::Parser->Value("Identifier", identifier, name) == elle::Status::Error)
            throw std::runtime_error("unable to retrieve the identifier");

          // create a user.
          if (User::Create(identifier, name) == elle::Status::Error)
            throw std::runtime_error("unable to create the user");

          // display a message.
          std::cout << "The user has been created successfully!"
                    << std::endl;

          break;
        }
      case User::OperationDestroy:
        {
          elle::String          name;

          // retrieve the name.
          if (Infinit::Parser->Value("Name", name) == elle::Status::Error)
            throw std::runtime_error("unable to retrieve the name value");

          // destroy a user.
          if (User::Destroy(name) == elle::Status::Error)
            throw std::runtime_error("unable to destroy the user");

          // display a message.
          std::cout << "The user has been destroyed successfully!"
                    << std::endl;

          break;
        }
      case User::OperationInformation:
        {
          elle::String          name;

          // retrieve the name.
          if (Infinit::Parser->Value("Name", name) == elle::Status::Error)
            throw std::runtime_error("unable to retrieve the name value");

          // display information.
          if (User::Information(name) == elle::Status::Error)
            throw std::runtime_error("unable to display information on the user");

          break;
        }
      case User::OperationUnknown:
      default:
        {
          // display the usage.
          Infinit::Parser->Usage();

          throw std::runtime_error("please specify an operation to perform");
        }
      }

    // delete the parser.
    delete Infinit::Parser;
    Infinit::Parser = nullptr;

    // clean Infinit.
    if (Infinit::Clean() == elle::Status::Error)
      throw std::runtime_error("unable to clean Infinit");

    // clean Lune
    if (lune::Lune::Clean() == elle::Status::Error)
      throw std::runtime_error("unable to clean Lune");
  }
}

int
main(int argc, char** argv)
{
  try
    {
      satellite::User(argc, argv);
    }
  catch (std::runtime_error const& e)
    {
      std::cerr << argv[0] << ": fatal error: " << e.what() << std::endl;
      elle::concurrency::scheduler().terminate();
      return 1;
    }
  return 0;
}
