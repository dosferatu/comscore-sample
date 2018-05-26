#include <exception>
#include <iostream>
#include <vector>
#include "../../lib/authenticate.h"
#include "../../lib/datastore_manager.h"

static void PrintUsage();


// ****************************************************************************
// Sample custom datastore manager
// ****************************************************************************
//
// Below are the foll
// -d [/path/to/datastore.sds]	Specify the path to the datastore (default: ./datastore.sds)
// -l [/path/to/logfile]		Specify the path to a log file (default: ./datastore.log)

int main(int argc, char **argv)
{
	try
	{
		std::string dataStorePath = "./datastore.sds";
		std::string logFilePath = "./datastore.log";
		std::vector<std::string> importDataPaths;

		// Parse command line arguments
		if (argc == 1) {
			PrintUsage();
			return 0;
		}

		for (int i = 1; i < argc; ++i) {
			// TODO: Write better arg parsing that doesn't suck
			importDataPaths.emplace_back(std::string(argv[i]));
		}

		// Create instances of the datastore manager and it's repository dependency.
		Repository repository;
		DataStoreManager dataStore(repository, dataStorePath);

		// Authenticate with the datastore manager so we can import our data sets.
		std::string clientId = "dosferatu";
		std::string password = "password123";
		Credentials credentials = dataStore.Connect(clientId, password);
		if (dataStore.Authenticate(credentials)) {
			for (auto& importDataPath : importDataPaths) {
				dataStore.ImportData(credentials, importDataPath);
			}
		}

		// Inject datastore interface in to API layer (message loop)
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}

	return 0;
}

static void PrintUsage()
{
	std::cout << "" << std::endl;
	return;
}
