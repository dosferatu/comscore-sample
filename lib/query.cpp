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
	: m_commandChain(), m_selectArgs(), m_aggregateCommands()
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
	return ((Query::Command::MinCommand     == command) ||
			(Query::Command::MaxCommand     == command) ||
			(Query::Command::SumCommand     == command) ||
			(Query::Command::CountCommand   == command) ||
			(Query::Command::CollectCommand == command));
}

bool Query::IsValidQueryString(const std::string& queryString)
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
	row_t record;
	m_selectArgs = this->ParseSelectCommandArgs(commandArgs);
	if (m_selectArgs.size() == 0) {
		throw std::invalid_argument("Cannot execute query: select statement is missing.");
	}

	while (inputStream.good() && inputStream >> record) {
		row_t::field_list_t fieldOrdering;
		for (auto& command : m_selectArgs) {
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
	if (m_selectArgs.size() == 0) {
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
			[&] (row_t& lhs, row_t& rhs)
			{
				bool isLessThan = true;
				for (auto& field : fieldList) {
					isLessThan = isLessThan && (lhs.Field(field) < rhs.Field(field));
				}

				return isLessThan;
			});
}

void Query::Group(Query::table_t& queryData, const std::string& groupField)
{
	std::string selectField = "";
	//if (m_selectArgs.size() == 0) {
		//throw std::invalid_argument("Cannot execute query: select statement is missing.");
	//} else if (m_selectArgs.size() == 1) {
		//if (m_selectArgs.count(groupField) == 0) {
			//throw std::invalid_argument("Cannot execute query: " + selectField + " is not part of an aggregate function.");
		//}
	//} else {
		//for (auto& selectArg : m_selectArgs) {
			//if (!Query::IsAggregateCommand(selectArg.second)) {
				//throw std::invalid_argument("Cannot execute query: " + selectField + " is not part of an aggregate function.");
			//}
		//}
	//}

	// Order by group field, then strip out redundant entries using the specified group specifier.
	this->Order(queryData, groupField);
	std::unique(std::begin(queryData), std::end(queryData),
			[&](row_t& lhs, row_t& rhs) { return (lhs.Field(groupField) == rhs.Field(groupField)); });
}

void Query::Filter(Query::table_t& queryData, const std::string& filter)
{
	if (m_selectArgs.size() == 0) {
		throw std::invalid_argument("Cannot execute query: select statement is missing.");
	}

	// Remove any elements that don't match the filter criteria
	queryData.erase(std::remove_if(std::begin(queryData), std::end(queryData),
				[&] (row_t& record) { return Query::EvaluateFilterString(record, filter); }), std::end(queryData));

	return;
}

void Query::Aggregate(Query::table_t& queryData, const std::string& groupField)
{
	std::cout << "Aggregate" << std::endl;
	std::string field = "";
	Query::Command command = Query::Command::InvalidCommand;
	for (auto& aggregate : m_selectArgs) {
		field = std::get<0>(aggregate);
		command = std::get<1>(aggregate);

		if (Query::IsAggregateCommand(command)) {
			std::cout << "Is command" << std::endl;
			std::string accumulator = "["; // TODO: Determine when we need this
			// TODO: Validate aggregate command for field type

			// Accumulate the field values now that the operation is validated.
			std::for_each(std::begin(queryData), std::end(queryData),
					[&](row_t& record)
					{
						// TODO: Set accumulator operation according to field type and aggregate operation
						std::cout << "Accumulator: " << accumulator << std::endl;
						std::cout << "Field: " << field << std::endl;
						std::cout << "Group Field: " << groupField << std::endl;
						std::cout << "Value: " << record.Field(field) << std::endl;
						accumulator += record.Field(field) + ","; 
						return !record;
					});

			accumulator += "]";
		}
	}
}


// ****************************************************************************
// Private implementation
// ****************************************************************************
const std::vector<std::string> logicTokens = { " OR ", " AND " };
bool Query::EvaluateFilterString(const row_t& record, const std::string& logicString)
{
	bool result = false;
	// TODO: Handle single field specifier
	//std::string::size_type pos = filter.find("=");
	//std::string field = filter.substr(0, pos);
	//std::string condition = filter.substr(pos + 1);

	// Evaluate parenthesis recursively
	std::string::size_type pos = logicString.find("(");
	if (pos == 0) {
		result = result && Query::EvaluateFilterString(record, logicString.substr(pos + 1, logicString.find(")")));	
	}

	// TODO: Parse
	std::istringstream iss(logicString);
	return result;
}

Query::command_map_t Query::ParseQueryString(const std::string& queryString)
{
	std::string token = "";
	std::string commandArgs = "";
	std::string commandString = "";
	std::istringstream iss(queryString);
	Query::command_map_t commands;

	// Tokenize by - to get command options; will be a single letter.
	// An argument for the command will follow, and will have a command specific format.
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

		commands.emplace(command, commandArgs);
	}

	return commands;
}

Query::select_args_t Query::ParseSelectCommandArgs(const std::string& commandArgs)
{
	std::string field;
	std::string token;
	std::istringstream iss(commandArgs);
	Query::select_args_t selectArgs;
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

		selectArgs.emplace(field, command);
	}

	return selectArgs;
}
