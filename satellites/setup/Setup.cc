#include <elle/Elle.hh>
#include <elle/io/Directory.hh>
#include <elle/utility/Parser.hh>

#include <satellites/setup/Setup.hh>

namespace satellite
{

//
// ---------- definitions -----------------------------------------------------
//

  ///
  /// this value defines the component's name.
  ///
  const elle::Character         Component[] = "8setup";

//
// ---------- methods ---------------------------------------------------------
//

  ///
  /// this method initializes the user's environment.
  ///
  elle::Status          Setup::Initialize()
  {
    elle::Path          path;
    lune::Configuration configuration;

    //
    // initialize the infinit path.
    //
    {
      // create the home path.
      if (path.Create(lune::Lune::Home) == elle::Status::Error)
        escape("unable to create the path");

      // if the user directory does not exist, create it.
      if (elle::io::Directory::Exist(path) == elle::Status::False)
        {
          // create the directory.
          if (elle::io::Directory::Create(path) == elle::Status::Error)
            escape("unable to create the directory");
        }
      else
        {
          // clear the directory.
          if (elle::io::Directory::Clear(path) == elle::Status::Error)
            escape("unable to clear the directory");
        }
    }

    //
    // create the configuration file.
    //
    {
      // pull the default parameters.
      if (configuration.Pull() == elle::Status::Error)
        escape("unable to pull the configuration parameters");

      // push the current parameters.
      if (configuration.Push() == elle::Status::Error)
        escape("unable to pus the parameters");

      // store the configuration.
      if (configuration.Store() == elle::Status::Error)
        escape("unable to store the configuration");
    }

    return elle::Status::Ok;
  }

  ///
  /// this method cleans the user's environment.
  ///
  elle::Status          Setup::Clean()
  {
    elle::Path          path;

    //
    // remove the configuration, if necessary.
    //
    {
      lune::Configuration       configuration;

      // if the configuration exists...
      if (configuration.Exist() == elle::Status::True)
        {
          // remove it.
          if (configuration.Erase() == elle::Status::Error)
            escape("unable to erase the configuration");
        }
    }

    //
    // reinitialize the infinit directory.
    //
    {
      // create the home path.
      if (path.Create(lune::Lune::Home) == elle::Status::Error)
        escape("unable to create the path");

      // if the user directory exists, clear it and remove it.
      if (elle::io::Directory::Exist(path) == elle::Status::True)
        {
          // clear the content.
          if (elle::io::Directory::Clear(path) == elle::Status::Error)
            escape("unable to clear the directory");

          // remove the directory.
          if (elle::io::Directory::Remove(path) == elle::Status::Error)
            escape("unable to remove the directory");
        }
    }

    return elle::Status::Ok;
  }

//
// ---------- functions -------------------------------------------------------
//

  ///
  /// the main function.
  ///
  elle::Status          Main(elle::Natural32                    argc,
                             elle::Character*                   argv[])
  {
    Setup::Operation    operation;

    // XXX Infinit::Parser is not deleted in case of errors

    // initialize the Elle library.
    if (elle::Elle::Initialize() == elle::Status::Error)
      escape("unable to initialize Elle");

    // set up the program.
    if (elle::concurrency::Program::Setup() == elle::Status::Error)
      escape("unable to set up the program");

    // initialize the nucleus library.
    if (nucleus::Nucleus::Initialize() == elle::Status::Error)
      escape("unable to initialize Nucleus");

    // initialize the Lune library.
    if (lune::Lune::Initialize() == elle::Status::Error)
      escape("unable to initialize Lune");

    // initialize Infinit.
    if (Infinit::Initialize() == elle::Status::Error)
      escape("unable to initialize Infinit");

    // initialize the Etoile library.
    if (etoile::Etoile::Initialize() == elle::Status::Error)
      escape("unable to initialize Etoile");

    // initialize the operation.
    operation = Setup::OperationUnknown;

    // allocate a new parser.
    Infinit::Parser = new elle::Parser(argc, argv);

    // specify a program description.
    if (Infinit::Parser->Description(Infinit::Copyright) == elle::Status::Error)
      escape("unable to set the description");

    // register the options.
    if (Infinit::Parser->Register(
          "Help",
          'h',
          "help",
          "display the help",
          elle::Parser::KindNone) == elle::Status::Error)
      escape("unable to register the option");

    // register the options.
    if (Infinit::Parser->Register(
          "Initialize",
          'i',
          "initialize",
          "initialize the user's infinit environment",
          elle::Parser::KindNone) == elle::Status::Error)
      escape("unable to register the option");

    // register the options.
    if (Infinit::Parser->Register(
          "Clean",
          'c',
          "clean",
          "clean the user's infinit environment",
          elle::Parser::KindNone) == elle::Status::Error)
      escape("unable to register the option");

    // parse.
    if (Infinit::Parser->Parse() == elle::Status::Error)
      escape("unable to parse the command line");

    // test the option.
    if (Infinit::Parser->Test("Help") == elle::Status::True)
      {
        // display the usage.
        Infinit::Parser->Usage();

        // quit.
        return elle::Status::Ok;
      }

    // check the mutually exclusive options.
    if ((Infinit::Parser->Test("Initialize") == elle::Status::True) &&
        (Infinit::Parser->Test("Clean") == elle::Status::True))
      {
        // display the usage.
        Infinit::Parser->Usage();

        escape("the initialize and clean options are mutually exclusive");
      }

    // test the option.
    if (Infinit::Parser->Test("Initialize") == elle::Status::True)
      operation = Setup::OperationInitialize;

    // test the option.
    if (Infinit::Parser->Test("Clean") == elle::Status::True)
      operation = Setup::OperationClean;

    // trigger the operation.
    switch (operation)
      {
      case Setup::OperationInitialize:
        {
          // initialize the environment.
          if (Setup::Initialize() == elle::Status::Error)
            escape("unable to initialize the environment");

          // display a message.
          std::cout << "The infinit environment has been "
                    << "initialized successfully!"
                    << std::endl;

          break;
        }
      case Setup::OperationClean:
        {
          // clean the environment.
          if (Setup::Clean() == elle::Status::Error)
            escape("unable to clean the environment");

          // display a message.
          std::cout << "The infinit environment has been "
                    << "cleaned successfully!"
                    << std::endl;

          break;
        }
      case Setup::OperationUnknown:
      default:
        {
          // display the usage.
          Infinit::Parser->Usage();

          escape("please specify an operation to perform");
        }
      }

    // delete the parser.
    delete Infinit::Parser;
    Infinit::Parser = nullptr;

    // clean the Etoile.
    if (etoile::Etoile::Clean() == elle::Status::Error)
      escape("unable to clean Etoile");

    // clean Infinit.
    if (Infinit::Clean() == elle::Status::Error)
      escape("unable to clean Infinit");

    // clean Lune
    if (lune::Lune::Clean() == elle::Status::Error)
      escape("unable to clean Lune");

    // clean the nucleus library.
    if (nucleus::Nucleus::Clean() == elle::Status::Error)
      escape("unable to clean Nucleus");

    // clean Elle.
    if (elle::Elle::Clean() == elle::Status::Error)
      escape("unable to clean Elle");

    return elle::Status::Ok;
  }

}

//
// ---------- main ------------------------------------------------------------
//

///
/// this is the program entry point.
///
int                     main(int                                argc,
                             char**                             argv)
{
  try
    {
      if (satellite::Main(argc, argv) == elle::Status::Error)
        {
          show();

          return (1);
        }
    }
  catch (std::exception& e)
    {
      std::cout << "The program has been terminated following "
                << "a fatal error (" << e.what() << ")." << std::endl;

      return (1);
    }

  return (0);
}
