#include <iostream>
#include <sstream>
#include "model.h"
#include "query.h"


// ****************************************************************************
// Static initialization
// ****************************************************************************
const std::map<std::string, Query::Command> Query::m_knownCommands =
{
	{ "s", Query::Command::SelectCommand},  // SELECT command
	{ "o", Query::Command::OrderCommand},   // ORDER command
	{ "g", Query::Command::GroupCommand},   // GROUP command
	{ "f", Query::Command::FilterCommand},  // FILTER command
};


// ****************************************************************************
// Construction
// ****************************************************************************
Query::Query(const std::string& queryString)
	: m_commandChain()
{
	if (!this->IsValidQueryString(queryString)) {
		throw std::invalid_argument("Invalid query string: " + queryString);
	}

	m_commandChain = this->ParseQueryString(queryString);
}

Query::~Query()
{
}


// ****************************************************************************
// Public API
// ****************************************************************************
Query::table_t Query::QueryCommand(std::istream& inputStream)
{
	Query::table_t results;
	if (!inputStream.good())
	{
		throw std::invalid_argument("Input stream given to query is not valid");
	}

	// TODO: Replace with use of a command pattern
	// for each command in commands, call command and pass in associated args.
	for (auto& command : m_commandChain) {
		switch (std::get<0>(command)) {
			case Query::SelectCommand:
				results = this->Select(inputStream, std::get<1>(command));
				break;
			case Query::OrderCommand:
			case Query::GroupCommand:
			case Query::FilterCommand:
			case Query::InvalidCommand:
			default:
			break;
		}
	}

	return results;
}


// ****************************************************************************
// Private implementation
// ****************************************************************************
bool Query::IsValidQueryString(std::string queryString) const
{
	//bool isValidQueryString = false;
	if (queryString.empty()) {
		return false;
	}

	// Tokenize the string and validate option / value format against schema
	//bool tokenWasParameterSpecifier = false; // true if previous token is one of the known options, i.e. "-{s,o,g,f}" 
	std::string token = "";
	std::istringstream iss(queryString);
	while (std::getline(iss, token, ' ')) {
	}

	return true;
}

Query::command_queue_t Query::ParseQueryString(const std::string& queryString)
{
	std::string token = "";
	std::string commandArgs = "";
	std::string commandString = "";
	std::istringstream iss(queryString);
	Query::command_queue_t commands;

	// Command parsing needs to be as follows:
	// ---------------------------------------
	//
	// tokenize by - to get command options; will be a single letter
	// an argument for the command will follow, and will have a command specific format
	while (std::getline(iss, token, '-')) {
		iss >> commandString;
		Query::Command command = m_knownCommands.at(commandString);
		switch (command) {
			case Query::SelectCommand:  // ',' delimited field names
			case Query::OrderCommand:
				iss >> commandArgs;
				break;

			case Query::GroupCommand:   // single field name
				iss >> commandArgs;
				break;

			case Query::FilterCommand:  // complicated mess
			case Query::InvalidCommand:
			default:
				break;
		}

		commands.emplace_back(std::make_tuple(command, commandArgs));
	}

	return commands;
}

Query::table_t Query::Select(std::istream& inputStream, const std::string& fields)
{
	Query::table_t results;
	if (!inputStream.good())
	{
		throw std::invalid_argument("Input stream given to query is not valid");
	}

	// Parse fields
	std::string token;
	std::istringstream iss(fields);
	while (std::getline(iss, token, ',')) {
		//std::cout << token << std::endl;
	}

	Model model;
	while (inputStream.good()) {
		inputStream >> model;
		std::cout << model << std::endl;
	}

	return results;
}
