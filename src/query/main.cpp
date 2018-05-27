#include <exception>
#include <iostream>
#include <sstream>
#include "../../lib/datastore_manager.h"
#include "../../lib/query.h"

// Sample query tool for custom datastore

// ****************************************************************************
// Command line options
// ****************************************************************************
static void PrintUsage()
{
	std::cout << "usage: query -s <FIELD1,FIELD2:AGGREGATE> [options]" << std::endl
		<< "options:" << std::endl
		<< "    " << "-o <FIELD1,FIELD2>    Order by ',' delimited fields" << std::endl
		<< "    " << "-g <FIELD>            Group by field" << std::endl
		<< "    " << "-f <FIELD=\"value\" AND FIELD2=\"value\" OR FIELD3=\"value\">" << std::endl;

	std::cout << std::endl;
	std::cout << "example: query -s TITLE,DATE:collect -o TITLE -f DATE=2014-04-21 OR DATE=2014-04-22" << std::endl;
}

//static const std::map<std::string, 

int main(int argc, char **argv)
{
	try
	{
		std::string dataStorePath = "./datastore.sds";
		std::string logFilePath = "./datastore.log";

		// Parse command line options
		if (argc == 1) {
			PrintUsage();
			return 0;
		}

		// Parse commandline and build the query command
		std::stringstream ss;
		std::string queryString = "";
		for (int i = 1; i < argc; ++i) {
			ss << argv[i] << " ";
		}

		queryString = ss.str();
		Query query(queryString);
		Repository repository;
		DataStoreManager dataStore(repository, dataStorePath);

		// Authenticate with the datastore manager so we can perform our queries
		std::string clientId = "dosferatu";
		std::string password = "password123";
		Credentials credentials = dataStore.Connect(clientId, password);
		if (dataStore.Authenticate(credentials)) {
			for (auto& record : dataStore.QueryData(credentials, query)) {
				for (auto& field : record) {
					std::cout << field << "|";
				}

				std::cout << std::endl;
			}
		}
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}

	return 0;
}
