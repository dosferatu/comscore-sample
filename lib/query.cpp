#include <algorithm>
#include <iostream>
#include <map>
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
	: m_commandChain(), m_selectCommands()
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

	for (auto& command : m_commandChain) {
		Query::Command queryCommand = std::get<0>(command);
		std::string commandArgs = std::get<1>(command);

		// Yoda notation
		if (Query::Command::SelectCommand == queryCommand) {
			results = this->Select(inputStream, commandArgs);
		} else if (Query::Command::OrderCommand == queryCommand) {
			this->Order(results, commandArgs);
		} else if (Query::Command::GroupCommand == queryCommand) {
			this->Group(results, commandArgs);
		} else if (Query::Command::FilterCommand == queryCommand) {
			this->Filter(results, commandArgs);
		}
	}

	return results;
}

bool Query::IsAggregateCommand(Query::Command command)
{
	return ((Query::Command::MinCommand == command) ||
		(Query::Command::MaxCommand     == command) ||
		(Query::Command::SumCommand     == command) ||
		(Query::Command::CountCommand   == command) ||
		(Query::Command::CollectCommand == command));
}

bool Query::IsValidQueryString(std::string queryString)
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


// ****************************************************************************
// Private query API
// ****************************************************************************
Query::table_t Query::Select(std::istream& inputStream, const std::string& commandArgs)
{
	Query::table_t results;
	if (!inputStream.good())
	{
		return results;
	}

	// Query the input stream and filter out the results by the given fields and aggregates.
	Model record;
	m_selectCommands = this->ParseSelectCommandArgs(commandArgs);
	if (m_selectCommands.size() == 0) {
		throw std::invalid_argument("Cannot execute query: select statement is missing.");
	}

	while (inputStream.good() && inputStream >> record) {
		Model::field_list_t fieldOrdering;
		for (auto& command : m_selectCommands) {
			std::string field = std::get<0>(command);
			fieldOrdering.emplace_back(field);
		}

		// Save the select filter and store the record
		record.SetOrdering(fieldOrdering);
		results.emplace_back(record);
	}

	return results;
}

void Query::Order(Query::table_t& queryData, const std::string& fields)
{
	if (m_selectCommands.size() == 0) {
		throw std::invalid_argument("Cannot execute query: select statement is missing.");
	}

	// Save the tokenized ordering fields to be easily used in a sort comparator
	std::string token;
	std::istringstream iss(fields);
	std::vector<std::string> fieldList;
	while (std::getline(iss, token, ',')) {
		// TODO: validate token
		fieldList.emplace_back(token);
	}

	// Sort using all given ordering fields as custom comparator
	std::sort(std::begin(queryData), std::end(queryData),
			[&] (Model& lhs, Model& rhs) { bool isLessThan = true;
			for (auto& field : fieldList) {
				isLessThan = isLessThan && (lhs.Field(field) < rhs.Field(field));
			}

			return isLessThan; });
}

void Query::Group(Query::table_t& queryData, const std::string& groupField)
{
	std::string selectField = "";
	Query::Command command = Query::Command::InvalidCommand;
	if (m_selectCommands.size() == 0) {
		throw std::invalid_argument("Cannot execute query: select statement is missing.");
	} else if (m_selectCommands.size() == 1) {
		selectField = std::get<0>(m_selectCommands[0]);
		if (selectField != groupField) {
			throw std::invalid_argument("Cannot execute query: " + selectField + " is not part of an aggregate function.");
		}
	} else {
		for (size_t i = 1; i < m_selectCommands.size(); ++i) {
			selectField = std::get<0>(m_selectCommands[i]);
			command = std::get<1>(m_selectCommands[i]);
			if (!Query::IsAggregateCommand(command)) {
				throw std::invalid_argument("Cannot execute query: " + selectField + " is not part of an aggregate function.");
			}
		}
	}

	// Order by group field, then strip out redundant entries using the specified group specifier.
	this->Order(queryData, groupField);
	std::unique(std::begin(queryData), std::end(queryData),
			[&](Model& lhs, Model& rhs) { return (lhs.Field(groupField) == rhs.Field(groupField)); });
}

void Query::Filter(Query::table_t& queryData, const std::string& filter)
{
	if (m_selectCommands.size() == 0) {
		throw std::invalid_argument("Cannot execute query: select statement is missing.");
	}

	// TODO: Compare operands of ANDs and ORs to find which chain together.
	// That or find a better way.

	// Find AND conditions first due to precedence
	if (filter.find(" AND ") != std::string::npos) {
	}

	// Next find OR conditions
	if (filter.find(" OR ") != std::string::npos) {
	}

	// Lastly find field value conditions
	
	std::string::size_type pos = filter.find("=");
	std::string field = filter.substr(0, pos);
	std::string condition = filter.substr(pos + 1);

	// Remove any elements that don't match the filter criteria
	queryData.erase(std::remove_if(std::begin(queryData), std::end(queryData),
				[&] (Model& model) { return (model.Field(field) != condition); }),
			std::end(queryData));

	return;
}


// ****************************************************************************
// Private implementation
// ****************************************************************************
Query::command_queue_t Query::ParseQueryString(const std::string& queryString)
{
	std::string token = "";
	std::string commandArgs = "";
	std::string commandString = "";
	std::istringstream iss(queryString);
	Query::command_queue_t commands;

	// tokenize by - to get command options; will be a single letter
	// an argument for the command will follow, and will have a command specific format
	// TODO: Handle aggregate functions and their coupling with the GROUP command
	while (std::getline(iss, token, '-') && iss >> commandString) {
		Query::Command command = m_knownCommands.at(commandString);
		switch (command) {
			case Query::Command::SelectCommand:  // ',' delimited field names
			case Query::Command::OrderCommand:
				iss >> commandArgs;
				break;

			case Query::Command::GroupCommand:   // single field name
				iss >> commandArgs;
				break;

			case Query::Command::FilterCommand:  // complicated mess
				iss >> commandArgs;
				break; 

			case Query::Command::MinCommand:
			case Query::Command::MaxCommand:
			case Query::Command::SumCommand:
			case Query::Command::CountCommand:
			case Query::Command::CollectCommand:
			case Query::Command::InvalidCommand:
			case Query::Command::NoCommand:
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
	Query::Command command = Query::Command::InvalidCommand;

	// Parse the fields for aggregate command specifiers
	while (std::getline(iss, token, ',')) {

		// Save the field and aggregate specifier before and after the ':' delimiter.
		// Otherwise token is just a field specifier.
		std::string::size_type pos = token.find(":");
		if (pos != std::string::npos) {
			field = token.substr(0, pos);
			std::string aggregateCommand = token.substr(pos + 1);
			if (Query::m_knownCommands.count(aggregateCommand) > 0) {
				command = Query::m_knownCommands.at(aggregateCommand);
			}
		} else {
			field = token;
			command = Query::Command::NoCommand;
		}

		selectCommands.emplace_back(std::make_tuple(field, command));
	}

	return selectCommands;
}
