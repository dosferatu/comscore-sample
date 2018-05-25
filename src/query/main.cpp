#include <exception>
#include <iostream>

// Sample query tool for custom datastore

//Your first task is to parse and import the file into a simple datastore.  You may use any file format that you want to implement to store the data. For this exercise, you must write your own datastore instead of reusing an existing one such as a SQL database or a NoSQL store. Also, you must assume that after importing many files the entire datastore will be too large to fit in memory.  Records in the datastore should be unique by STB, TITLE and DATE.  Subsequent imports with the same logical record should overwrite the earlier records. 


// ****************************************************************************
// Command line options
// ****************************************************************************
// -d [/path/to/datastore.sds]	Specify the path to the datastore (default: ./datastore.sds)
// -l [/path/to/logfile]		Specify the path to a log file (default: ./datastore.log)
int main(int argc, char **argv)
{
	try
	{
		std::string dataStorePath = "./datastore.sds";
		std::string logFilePath = "./datastore.log";

		// Parse command line options
		for (int i = 1; i < argc; ++i) {
			std::cout << argv[i] << std::endl;

			// TODO: Write arg parsing that doesn't suck
		}
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}

	return 0;
}
