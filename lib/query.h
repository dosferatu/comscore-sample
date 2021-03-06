#ifndef QUERY_H
#define QUERY_H

#include "model.h"


/// Simple class to store information about a known command.
class Command
{
	public:
		enum class Type {
			Select,     // Query and filter commands.
			Order,
			Group,
			Filter,
			Min,        // Aggregate commands.
			Max,
			Sum,
			Count,
			Collect,
			Invalid,    // Used for initialization.
			NoCommand,  // Indicates that the given parameter does not have an associated command.
		};

		typedef Command command_t;

		Command(const Command::Type commandType, const std::string& commandArgs) :
			m_commandType(commandType), m_commandArgs(commandArgs)
		{
		}

		// Public accessors
		/// Gets the command type for this objec.
		Command::Type CommandType() const { return m_commandType; }

		/// Sets the command type for this object given the supplied command type.
		void CommandType(const Command::Type commandType) { m_commandType = commandType; }

		/// Gets the command argument for this object.
		std::string CommandArgs() const { return m_commandArgs; }

		/// Sets the command argument for this object given the supplied string.
		void CommandArgs(const std::string& commandArgs) { m_commandArgs = commandArgs; }

	private:
		/// The stored command type
		Command::Type m_commandType;

		/// The stored command argument - format depends on the command type.
		std::string m_commandArgs;
};


/// Represents a query for the data store and any functionality associated with it.
class Query
{
	public:
		// Type defines these data structures so implementation is easier to read/change.
		typedef Model row_t;                 /// Represents a record produced by a query.
		typedef std::vector<row_t> table_t;  /// Collection of records produced by a query.

		/// Collection of fields + aggregate commands
		typedef std::vector<Command::command_t> command_vector_t;

		/// Collection of commands + arguments
		typedef std::map<Command::Type, std::string> command_map_t;

		/// Construction
		Query() = delete;
		Query(const std::string& queryString);
		~Query();

		// Public API
		Query::table_t QueryCommand(std::istream& inputStream);
		static bool IsAggregateCommand(Command::Type commandType);
		static bool IsValidQueryString(const std::string& queryString);

	private:
		// Private query API
		/// Selects specified fields from each record in the datastore.
		/// if a filter was specified then records will be checked against it.
		Query::table_t Select(std::istream& inputStream, const std::string& commandArgs);

		// Order by the given fields
		void Order(Query::table_t& queryData, const std::string& fields);

		// Group by the given field if it is in the select statement, with or without specified aggregates.
		void Group(Query::table_t& queryData, const std::string& groupField);

		// Accumulate the specified aggregating fields from previousRecord, and return the accumulated record
		Query::row_t AggregateFields(const Query::row_t& prevRecord, Query::row_t& accumulator, const std::string& groupField) const;

		// Filters records out of the select command using either a single field value or boolean logical AND/OR
		/// Returns true or false for whether the given record passes the filter.
		static bool EvaluateFilterString(const row_t& record, const std::string& logicString);
		static bool EvaluateFilterOperandString(const row_t& record, const std::string& operand);

		/// Creates an ordered collection of commands to perform from the given query string.
		static command_map_t ParseQueryString(const std::string& queryString);

		/// Parse the select command argument for fields and their respective aggregate functions.
		static command_vector_t ParseSelectCommandArgs(const std::string& commandArgs);

		/// Map of known strings to their related Query functions for parsing query strings.
		static const std::map<std::string, Command::Type> m_knownCommands;

		/// Ordered collection of commands to be performed when Command is called.
		Query::command_map_t m_commandChain;

		/// Cache the fields and their aggregate functions specified in the select command.
		Query::command_vector_t m_selectArgs;

		/// Cache the fields that will have aggregate functions run on them
		Query::command_vector_t m_aggregateCommands;
};

#endif
