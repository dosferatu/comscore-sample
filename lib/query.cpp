#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include "model.h"
#include "query.h"


// ****************************************************************************
// Static initialization
// ****************************************************************************
const std::map<std::string, Command::Type> Query::m_knownCommands =
{
	{ "s", Command::Type::Select},         // SELECT command
	{ "o", Command::Type::Order},          // ORDER command
	{ "g", Command::Type::Group},          // GROUP command
	{ "f", Command::Type::Filter},         // FILTER command
	{ "min", Command::Type::Min},          // MIN aggregate command
	{ "max", Command::Type::Max},          // MAX aggregate command
	{ "sum", Command::Type::Sum},          // SUM aggregate command
	{ "count", Command::Type::Count},      // COUNT aggregate command
	{ "collect", Command::Type::Collect},  // COLLECT aggregate command
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

	// Query the input stream and filter out the results by the given fields and aggregates.
	// Select the data for the given fields
	results = this->Select(inputStream, m_commandChain.at(Command::Type::Select));

	// Order the results if requested.
	if (m_commandChain.count(Command::Type::Order) > 0) {
		this->Order(results, m_commandChain.at(Command::Type::Order));
	}

	// Group the results if requested.
	if (m_commandChain.count(Command::Type::Group) > 0) {
		this->Group(results, m_commandChain.at(Command::Type::Group));
	}

	return results;
}

bool Query::IsAggregateCommand(Command::Type command)
{
	return ((Command::Type::Min     == command) ||
			(Command::Type::Max     == command) ||
			(Command::Type::Sum     == command) ||
			(Command::Type::Count   == command) ||
			(Command::Type::Collect == command));
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

	// Cache any aggregate commands from the select statement
	for (auto& command : m_selectArgs) {
		if (Query::IsAggregateCommand(command.CommandType())) {
			m_aggregateCommands.emplace_back(command);
		}
	}

	// Select, filter, and accumulate the data
	Query::row_t previousRecord;
	while (inputStream.good() && inputStream >> record) {
		row_t::field_list_t fieldOrdering;
		for (auto& command : m_selectArgs) {
			std::string field = command.CommandArgs();
			fieldOrdering.emplace_back(field);
		}

		// Save the select filter and store the record; accumulate with previous row if requested
		record.SetOrdering(fieldOrdering);
		if (!results.empty()) {
			previousRecord = results.back();
		}

		// If a filter was given, then Skip this record if it doesn't pass through the filter
		if (m_commandChain.count(Command::Type::Filter) > 0) {
			if (!Query::EvaluateFilterString(record, m_commandChain.at(Command::Type::Filter))) {
				continue;
			}
		}

		// TODO: Implement aggregate functions
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
	if (m_selectArgs.size() == 0) {
		throw std::invalid_argument("Cannot execute query: select statement is missing.");
	}

	// Ensure the group by field is present in the select query, and that all other fields are aggregate commands
	bool hasMatchingSelectField = false;
	for (auto& selectArg : m_selectArgs) {
		if (selectArg.CommandArgs() == groupField) {
			hasMatchingSelectField = true;
		} else if (!Query::IsAggregateCommand(selectArg.CommandType())) {
			throw std::invalid_argument("Cannot execute query: " + selectArg.CommandArgs() + " is not part of an aggregate function.");
		}
	}

	if (!hasMatchingSelectField) {
		throw std::invalid_argument("Cannot execute query: " + groupField + " is not one of the select specifiers.");
	}

	// Order by group field, then strip out redundant entries using the specified group specifier.
	this->Order(queryData, groupField);
	queryData.erase(std::unique(std::begin(queryData), std::end(queryData),
			[&](row_t& lhs, row_t& rhs) { return (lhs.Field(groupField) == rhs.Field(groupField)); }),
			std::end(queryData));
}

Query::row_t Query::AggregateFields(const Query::row_t& prevRecord, Query::row_t& accumulator, const std::string& groupField) const
{
	if (!prevRecord) {
		return accumulator;
	}

	// for aggregate: if current group field value matches a previous group field value then accumulate

	std::string accumulatorField = "";
	std::string accumulatedValue = "";
	for (auto& command : m_aggregateCommands) {
		accumulatorField = command.CommandArgs();
		std::cout << "Accumulating field: |" << accumulatorField << "|" << std::endl;

		// TODO: Apply specific accumulator
		// Apply 
		if (prevRecord.Field(groupField) == accumulator.Field(groupField)) {
			prevRecord.Field(accumulatorField);
			accumulator.Field(accumulatorField);
			accumulatedValue = prevRecord.Field(accumulatorField) + accumulator.Field(accumulatorField);
			accumulator.Field(accumulatorField, accumulatedValue);
		}
	}

	return accumulator;
}


// ****************************************************************************
// Private implementation
// ****************************************************************************
bool Query::EvaluateFilterString(const row_t& record, const std::string& logicString)
{
	bool result = false;
	bool needsLHSOperand = true;
	static const std::string orToken = " OR ";
	static const std::string andToken = " AND ";

	// Evaluate parenthesis recursively.
	std::string logicSubString = logicString;
	if ('(' == logicString[0]) {
		logicSubString = logicString.substr(1, logicString.find_first_of(")") - 1);
		result = Query::EvaluateFilterString(record, logicSubString);
		logicSubString = logicString.substr(logicString.find_first_of(")"));
		needsLHSOperand = false;
	}

	// Find the closest operator
	std::string::size_type orPos = logicSubString.find(orToken);
	std::string::size_type andPos = logicSubString.find(andToken);
	std::string::size_type operatorPos = (orPos < andPos) ? orPos : andPos;

	// Process the operator.
	if (operatorPos != std::string::npos) {
		// Evaluate the left operand if needed.
		if (needsLHSOperand) { 
			result = Query::EvaluateFilterOperandString(record, logicSubString.substr(0, operatorPos));
		}

		// Evaluate the right operand recursively.
		if (orPos < andPos) {
			result = (result || Query::EvaluateFilterString(record, logicSubString.substr(operatorPos + orToken.length())));
			return result;
		} else {
			result = (result && Query::EvaluateFilterString(record, logicSubString.substr(operatorPos + andToken.length())));
			return result;
		}
	} else {
		// This filter string is a single field specifier, or the last right hand side operand of a grouping.
		result = Query::EvaluateFilterOperandString(record, logicSubString);
		return result;
	}
}

bool Query::EvaluateFilterOperandString(const row_t& record, const std::string& operand)
{
	std::string::size_type pos = operand.find("=");
	std::string field = operand.substr(operand.find_first_not_of(" "), pos);

	// Strip the condition of trailing whitespace and then remove surrounding ""
	std::string condition = operand.substr(pos + 1, operand.find_last_not_of(" "));
	condition = condition.substr(condition.find_first_of("\"") + 1, condition.find_last_of("\"") - 1);
	return (record.Field(field) == condition);
}

Query::command_map_t Query::ParseQueryString(const std::string& queryString)
{
	std::string commandArgs = "";
	std::string commandString = "";
	Query::command_map_t commands;

	// Tokenize by - to get command options; will be a single letter.
	// An argument for the command will follow, and will have a command specific format.
	std::string item;
	std::vector<std::string> tokens;
	std::stringstream queryStream(queryString);
	while (std::getline(queryStream, item, '-')) {
		if (item.empty()) {
			continue;
		}

		// Parse the first character that signfies a command
		std::stringstream commandStream(item);
		commandStream >> commandString;
		if (m_knownCommands.count(commandString) == 0) {
			throw std::invalid_argument("Invalid query command string " + commandString + " given.");
		}

		Command::Type command = m_knownCommands.at(commandString);
		if (command == Command::Type::Invalid) {
			throw std::invalid_argument("Invalid query command string " + commandString + " given.");
		}

		// Parse the associated arguments for the command (trim whitespace)
		std::getline(commandStream, commandArgs);
		commandArgs = commandArgs.substr(commandArgs.find_first_not_of(" "), commandArgs.length());
		commandArgs = commandArgs.substr(0, commandArgs.find_last_not_of(" ") + 1);
		commands.emplace(command, commandArgs);
	}

	return commands;
}

Query::command_vector_t Query::ParseSelectCommandArgs(const std::string& commandArgs)
{
	std::string field;
	std::string token;
	std::istringstream iss(commandArgs);
	Query::command_vector_t selectArgs;
	Command::Type command = Command::Type::Invalid;

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
			command = Command::Type::NoCommand;
		}

		selectArgs.emplace_back(command, field);
	}

	return selectArgs;
}
