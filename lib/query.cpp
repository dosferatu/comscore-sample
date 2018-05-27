#include <algorithm>
#include <iostream>
#include <sstream>
#include "model.h"
#include "query.h"


// ****************************************************************************
// Static initialization
// ****************************************************************************
const std::map<std::string, Query::Command> Query::m_knownCommands =
{
	{ "s", Query::Command::SelectCommand},         // SELECT command
	{ "o", Query::Command::OrderCommand},          // ORDER command
	{ "g", Query::Command::GroupCommand},          // GROUP command
	{ "f", Query::Command::FilterCommand},         // FILTER command
	{ "min", Query::Command::MinCommand},          // MIN aggregate command
	{ "max", Query::Command::MaxCommand},          // MAX aggregate command
	{ "sum", Query::Command::SumCommand},          // SUM aggregate command
	{ "count", Query::Command::CountCommand},      // COUNT aggregate command
	{ "collect", Query::Command::CollectCommand},  // COLLECT aggregate command
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
	// TODO: Parse args here and run query; will be faster than separate calls
	// for each command in commands, call command and pass in associated args.
	for (auto& command : m_commandChain) {
		switch (std::get<0>(command)) {
			case Query::SelectCommand:
				results = this->Select(inputStream, std::get<1>(command));
				break;
			case Query::OrderCommand:
				break;
			case Query::GroupCommand:
				break;
			case Query::FilterCommand:
				this->Filter(results, std::get<1>(command));
				break;

			case Query::MinCommand:
			case Query::MaxCommand:
			case Query::SumCommand:
			case Query::CountCommand:
			case Query::CollectCommand:
			case Query::InvalidCommand:
			case Query::NoCommand:
			default:
				break;
		}
	}

	return results;
}


// ****************************************************************************
// Private query API
// ****************************************************************************
Query::table_t Query::Select(std::istream& inputStream, const std::string& commandArgs)
{
	Query::table_t results;
	if (!inputStream.good())
	{
		//throw std::invalid_argument("Input stream given to query is not valid");
		return results;
	}

	// Query the input stream and filter out the results by the given fields and aggregates.
	Model model;
	Query::select_command_t selectCommands = this->ParseSelectCommandArgs(commandArgs);
	while (inputStream.good() && inputStream >> model) {

		// For any field not found in the set of fields from the select command, set the value to null.
		for (auto& field : Model::m_validFields) {
			if (std::find_if_not( selectCommands.begin(), selectCommands.end(),
						[&] (auto& selectCommand) { return (field != std::get<0>(selectCommand)); })
					== std::end(selectCommands)) {

				// Field not in select list, so clear it.
				model.Field(field, "");
			}
		}

		results.emplace_back(model);
	}

	return results;
}

void Query::Filter(Query::table_t& queryData, const std::string& filter)
{
	// Assume single field fiter for now.
	size_t pos = filter.find("=");
	std::string field = filter.substr(0, pos);
	std::string condition = filter.substr(pos + 1);

	// Remove any elements that don't match the filter criteria
	queryData.erase(std::remove_if(std::begin(queryData), std::end(queryData),
				[&] (Model& model) { return (model.Field(field) != condition); }), // Implement the predicate
			std::end(queryData));

	return;
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
		// TODO: implement
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
	while (std::getline(iss, token, '-') && iss >> commandString) {
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
				iss >> commandArgs;
				break; 

			case Query::MinCommand:
			case Query::MaxCommand:
			case Query::SumCommand:
			case Query::CountCommand:
			case Query::CollectCommand:
			case Query::InvalidCommand:
			case Query::NoCommand:
			default:
				break;
		}

		commands.emplace_back(std::make_tuple(command, commandArgs));
	}

	return commands;
}

Query::select_command_t Query::ParseSelectCommandArgs(const std::string& commandArgs)
{
	std::string field;
	std::string token;
	std::istringstream iss(commandArgs);
	Query::select_command_t selectCommands;
	Query::Command command = Query::InvalidCommand;

	// Parse the fields for aggregate command specifiers
	while (std::getline(iss, token, ',')) {
		size_t pos = token.find(":");
		if (pos != std::string::npos) {
			// Save the field and aggregate specifier before and after the ':' delimiter
			field = token.substr(0, pos);
			std::string aggregateCommand = token.substr(pos + 1);
			if (Query::m_knownCommands.count(aggregateCommand) > 0) {
				command = Query::m_knownCommands.at(aggregateCommand);
			}
		} else {
			field = token;
			command = Query::NoCommand;
		}

		selectCommands.emplace_back(std::make_tuple(field, command));
	}

	return selectCommands;
}
