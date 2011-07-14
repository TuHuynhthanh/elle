//
// ---------- header ----------------------------------------------------------
//
// project       hole
//
// license       infinit
//
// file          /home/mycure/infinit/hole/implementations/remote/Server.cc
//
// created       julien quintard   [thu may 26 09:58:52 2011]
// updated       julien quintard   [tue jul 12 14:57:45 2011]
//

//
// ---------- includes --------------------------------------------------------
//

#include <hole/implementations/remote/Server.hh>
#include <hole/implementations/remote/Manifest.hh>

namespace hole
{
  namespace implementations
  {
    namespace remote
    {

//
// ---------- constructors & destructors --------------------------------------
//

      ///
      /// default constructor.
      ///
      Server::Server(const nucleus::Network&			network,
		     const elle::Address&			host):
	Node(network, host),

	gate(NULL)
      {
      }

      ///
      /// destructor.
      ///
      Server::~Server()
      {
	// if present, delete the gate.
	if (this->gate != NULL)
	  delete this->gate;
      }

//
// ---------- node ------------------------------------------------------------
//

      ///
      /// initialize the server by waiting for incoming connections.
      ///
      elle::Status	Server::Initialize()
      {
	enter();

	//
	// register the messages.
	//
	{
	  elle::Callback<
	    elle::Parameters<
	      const nucleus::Address,
	      const nucleus::Derivable<nucleus::Block>
	      >
	    >					push(&Server::Push, this);
	  elle::Callback<
	    elle::Parameters<
	      const nucleus::Address,
	      const nucleus::Version
	      >
	    >					pull(&Server::Pull, this);
	  elle::Callback<
	    elle::Parameters<
	      const nucleus::Address
	      >
	    >					wipe(&Server::Wipe, this);

	  // register the push message.
	  if (elle::Network::Register<TagPush>(push) == elle::StatusError)
	    escape("unable to register the callback");

	  // register the pull message.
	  if (elle::Network::Register<TagPull>(pull) == elle::StatusError)
	    escape("unable to register the callback");

	  // register the wipe message.
	  if (elle::Network::Register<TagWipe>(wipe) == elle::StatusError)
	    escape("unable to register the callback");
	}

	//
	// create the connection.
	//
	{
	  elle::Callback<
	    elle::Parameters<
	      elle::Gate*
	    >
	  >				connection(&Server::Connection, this);

	  // listen for incoming connections.
	  if (elle::Bridge::Listen(this->host,
				   connection) == elle::StatusError)
	    escape("unable to listen for bridge connections");
	}

	leave();
      }

      ///
      /// clean the server.
      ///
      elle::Status	Server::Clean()
      {
	enter();

	// close the client connection, if necessary.
	if (this->gate != NULL)
	  {
	    // disconnect.
	    if (this->gate->Disconnect() == elle::StatusError)
	      escape("unable to disconnect the client gate");
	  }

	// block the incoming connections.
	if (elle::Bridge::Block(this->host) == elle::StatusError)
	  escape("unable to block bridge connections");

	leave();
      }

      ///
      /// this method stores an immutable block.
      ///
      elle::Status	Server::Put(const nucleus::Address&	address,
				    const nucleus::ImmutableBlock& block)
      {
	enter();

	// does the block already exist.
	if (block.Exist(this->network, address) == elle::StatusTrue)
	  escape("this immutable block seems to already exist");

	// store the block.
	if (block.Store(this->network, address) == elle::StatusError)
	  escape("unable to store the block");

	leave();
      }

      ///
      /// this method stores a mutable block.
      ///
      elle::Status	Server::Put(const nucleus::Address&	address,
				    const nucleus::MutableBlock& block)
      {
	enter();

	// does the block already exist.
	if (block.Exist(this->network,
			address,
			nucleus::Version::Last) == elle::StatusTrue)
	  {
	    nucleus::MutableBlock*	current;

	    // build a block according to the component.
	    if (nucleus::Nucleus::Factory.Build(address.component,
						current) == elle::StatusError)
	      escape("unable to build the block");

	    // load the latest version.
	    if (current->Load(this->network,
			      address,
			      nucleus::Version::Last) == elle::StatusError)
	      escape("unable to load the current version");

	    // does the given block derive the current version.
	    if (!(block.version > current->version))
	      escape("the block to store does not seem to derive the current "
		     "version");

	    // delete the current instance.
	    delete current;

	    // waive.
	    waive(current);

	    // release.
	    release();
	  }

	// store the block.
	if (block.Store(this->network, address) == elle::StatusError)
	  escape("unable to store the block");

	leave();
      }

      ///
      /// this method retrieves an immutable block.
      ///
      elle::Status	Server::Get(const nucleus::Address&	address,
				    nucleus::ImmutableBlock&	block)
      {
	enter();

	// does the block exist.
	if (block.Exist(this->network, address) == elle::StatusFalse)
	  escape("the block does not seem to exist");

	// load the block.
	if (block.Load(this->network, address) == elle::StatusError)
	  escape("unable to load the block");

	// validate the block.
	if (block.Validate(address) == elle::StatusFalse)
	  escape("the block seems to be invalid");

	leave();
      }

      ///
      /// this method retrieves a mutable block.
      ///
      elle::Status	Server::Get(const nucleus::Address&	address,
				    const nucleus::Version&	version,
				    nucleus::MutableBlock&	block)
      {
	enter();

	// does the block exist.
	if (block.Exist(this->network, address, version) == elle::StatusFalse)
	  escape("the block does not seem to exist");

	// load the block.
	if (block.Load(this->network, address, version) == elle::StatusError)
	  escape("unable to load the block");

	// validate the block.
	if (block.Validate(address) == elle::StatusFalse)
	  escape("the block seems to be invalid");

	leave();
      }

      ///
      /// this method removes a block.
      ///
      elle::Status	Server::Kill(const nucleus::Address&	address)
      {
	nucleus::Block	block;

	enter();

	// treat the request depending on the nature of the block which
	// the addres indicates.
	switch (address.family)
	  {
	  case nucleus::FamilyContentHashBlock:
	    {
	      nucleus::ImmutableBlock	ib;

	      // erase the immutable block.
	      if (ib.Erase(this->network, address) == elle::StatusError)
		escape("unable to erase the block");

	      break;
	    }
	  case nucleus::FamilyPublicKeyBlock:
	  case nucleus::FamilyOwnerKeyBlock:
	  case nucleus::FamilyImprintBlock:
	    {
	      nucleus::MutableBlock	mb;

	      // retrieve the mutable block.
	      if (mb.Erase(this->network, address) == elle::StatusError)
		escape("unable to erase the block");

	      break;
	    }
	  default:
	    {
	      escape("unknown block family");
	    }
	  }

	leave();
      }

//
// ---------- callbacks -------------------------------------------------------
//

      ///
      /// this method stores the given block.
      ///
      elle::Status	Server::Push(const nucleus::Address&	address,
				     const
				       nucleus::Derivable
				         <nucleus::Block>&	derivable)
      {
	nucleus::Block*	object;

	enter();

	printf("Server::Push\n");

	// infer the block from the derivable.
	if (derivable.Infer(object) == elle::StatusError)
	  escape("unable to infer the block from the derivable");

	// forward the request depending on the nature of the block which
	// the addres indicates.
	switch (address.family)
	  {
	  case nucleus::FamilyContentHashBlock:
	    {
	      nucleus::ImmutableBlock*	ib;

	      // cast to an immutable block.
	      ib = static_cast<nucleus::ImmutableBlock*>(object);

	      // store the immutable block.
	      if (this->Put(address, *ib) == elle::StatusError)
		escape("unable to put the block");

	      break;
	    }
	  case nucleus::FamilyPublicKeyBlock:
	  case nucleus::FamilyOwnerKeyBlock:
	  case nucleus::FamilyImprintBlock:
	    {
	      nucleus::MutableBlock*	mb;

	      // cast to a mutable block.
	      mb = static_cast<nucleus::MutableBlock*>(object);

	      // store the mutable block.
	      if (this->Put(address, *mb) == elle::StatusError)
		escape("unable to put the block");

	      break;
	    }
	  default:
	    {
	      escape("unknown block family");
	    }
	  }

	// answer to the caller.
	if (this->gate->Reply(
	      elle::Inputs<TagOk>()) == elle::StatusError)
	  escape("unable to answer to the caller");

	leave();
      }

      ///
      /// this method returns the block associated with the given address.
      ///
      elle::Status	Server::Pull(const nucleus::Address&	address,
				     const nucleus::Version&	version)
      {
	nucleus::Block*	block;

	enter(instance(block));

	printf("Server::Pull\n");

	// build the block according to the component.
	if (nucleus::Nucleus::Factory.Build(address.component,
					    block) == elle::StatusError)
	  escape("unable to build the block");

	// forward the request depending on the nature of the block which
	// the addres indicates.
	switch (address.family)
	  {
	  case nucleus::FamilyContentHashBlock:
	    {
	      nucleus::ImmutableBlock*	ib;

	      // cast to an immutable block.
	      ib = static_cast<nucleus::ImmutableBlock*>(block);

	      // retrieve the immutable block.
	      if (this->Get(address, *ib) == elle::StatusError)
		escape("unable to get the block");

	      break;
	    }
	  case nucleus::FamilyPublicKeyBlock:
	  case nucleus::FamilyOwnerKeyBlock:
	  case nucleus::FamilyImprintBlock:
	    {
	      nucleus::MutableBlock*	mb;

	      // cast to a mutable block.
	      mb = static_cast<nucleus::MutableBlock*>(block);

	      // retrieve the mutable block.
	      if (this->Get(address, version,
			    *mb) == elle::StatusError)
		escape("unable to get the block");

	      break;
	    }
	  default:
	    {
	      escape("unknown block family");
	    }
	  }

	nucleus::Derivable<nucleus::Block>	derivable(address.component,
							  *block);

	// return the block to the caller.
	if (this->gate->Reply(
	      elle::Inputs<TagBlock>(derivable)) == elle::StatusError)
	  escape("unable to return the block to the caller");

	// delete the block.
	delete block;

	// waive.
	waive(block);

	leave();
      }

      ///
      /// this method removes the block associated with the given address.
      ///
      elle::Status	Server::Wipe(const nucleus::Address&	address)
      {
	enter();

	printf("Server::Wipe\n");

	// forward the kill request to the implementation.
	if (this->Kill(address) == elle::StatusError)
	  escape("unable to erase the block");

	// answer the caller.
	if (this->gate->Reply(
	      elle::Inputs<TagOk>()) == elle::StatusError)
	  escape("unable to answer to the caller");

	leave();
      }

      ///
      /// this method handles new connections.
      ///
      elle::Status	Server::Connection(elle::Gate*&		gate)
      {
	elle::Callback<
	  elle::Parameters<
	    const elle::String
	  >
	>				error(&Server::Error, this);

	enter();

	// check if there already is a client.
	if (this->gate != NULL)
	  escape("a client seems to already be connected");

	// register the client.
	this->gate = gate;

	// register the error callback.
	if (this->gate->Monitor(error) == elle::StatusError)
	  escape("unable to monitor the connection");

	leave();
      }

      ///
      /// this callback is triggered whenever the connection is shut down.
      ///
      elle::Status	Server::Error(const elle::String&)
      {
	enter();

	// delete the gate.
	delete this->gate;

	// set the gate to null.
	this->gate = NULL;

	leave();
      }

//
// ---------- dumpable --------------------------------------------------------
//

      ///
      /// this method dumps the server.
      ///
      elle::Status	Server::Dump(const elle::Natural32	margin) const
      {
	elle::String	alignment(margin, ' ');

	enter();

	std::cout << alignment << "[Server]" << std::endl;

	// dump the parent.
	if (Node::Dump(margin + 2) == elle::StatusError)
	  escape("unable to dump the node");

	// dump the gate.
	if (this->gate != NULL)
	  {
	    if (this->gate->Dump(margin + 2) == elle::StatusError)
	      escape("unable to dump the gate");
	  }
	else
	  {
	    std::cout << alignment << elle::Dumpable::Shift
		      << "[Gate] " << elle::none
		      << std::endl;
	  }


	leave();
      }

    }
  }
}
