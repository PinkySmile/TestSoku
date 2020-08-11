#include <iostream>
#include "Client.hpp"

int main(int argc, char **argv)
{
	Soku::Client client;
	Soku::CharacterSelectKeys keys{
		false,
		false,
		true,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false
	};

	if (argc == 1)
		return EXIT_FAILURE;

	if (argc == 2)
		client.host(atoi(argv[1]), [keys](Soku::Client::State &){ return Soku::Inputs{keys}; });
	else if (argc == 3)
		client.connect(argv[1], atoi(argv[2]), [keys](Soku::Client::State &){ return Soku::Inputs{keys}; });
	else if (argc == 4)
		client.connect(argv[1], atoi(argv[2]));

	return EXIT_SUCCESS;
}
