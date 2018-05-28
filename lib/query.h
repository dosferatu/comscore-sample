#ifndef QUERY_H
#define QUERY_H

#include "model.h"


class Command
{
	public:
		enum class Type {
			Select,   // Query and filter commands
			Order,
			Group,
			Filter,
			Min,      // Aggregate commands
			Max,
			Sum,
			Count,
			Collect,
			Invalid,  // Used for initialization
			NoCommand,       // Used to indicate that the given parameter does not have an associated command
		};

		typedef Command command_t;

		Command(const Command::Type commandType, const std::string& commandArgs) :
			m_commandType(commandType), m_commandArgs(commandArgs)
		{
		}

		// Public API
		Command::Type CommandType() const { return m_commandType; }
		void CommandType(const Command::Type commandType) { m_commandType = commandType; }

		std::string CommandArgs() const { return m_commandArgs; }
		void CommandArgs(const std::string& commandArgs) { m_commandArgs = commandArgs; }

	private:
		Command::Type m_commandType;
		std::string m_commandArgs;
};


class Query
{
	public:
		// Type defines these data structures so implementation is easier to read/change
		typedef Model row_t;                 /// Represents a record produced by a query
		typedef std::vector<row_t> table_t;  /// Collection of records produced by a query

		/// Collection of fields + aggregate commands
		typedef std::vector<Command::command_t> select_args_t;

		/// Collection of commands + arguments
		typedef std::map<Command::Type, std::string> command_vector_t;

		/// Construction
		Query() = delete;
		Query(const std::string& queryString);
		~Query();

		// Public API
		Query::table_t QueryCommand(std::istream& inputStream);
		static bool IsAggregateCommand(Command::Type commandType);
		static bool IsValidQueryString(const std::string& queryString);

	private:
		// Query API
		Query::table_t Select(std::istream& inputStream, const std::string& commandArgs);
		void Order(Query::table_t& queryData, const std::string& fields);
		void Group(Query::table_t& queryData, const std::string& groupField);
		void Filter(Query::table_t& queryData, const std::string& filter);
		void Aggregate(Query::table_t& queryData, const std::string& groupField);

		/// Returns true or false for whether the given record passes the filter.
		static bool EvaluateFilterString(const row_t& record, const std::string& logicString);
		static bool EvaluateFilterOperandString(const row_t& record, const std::string& operand);

		/// Creates an ordered collection of commands to perform from the given query string.
		static command_vector_t ParseQueryString(const std::string& queryString);

		/// Parse the select command argument for fields and their respective aggregate functions.
		static select_args_t ParseSelectCommandArgs(const std::string& commandArgs);

		/// Map of known strings to their related Query functions for parsing query strings.
		static const std::map<std::string, Command::Type> m_knownCommands;

		/// Ordered collection of commands to be performed when Command is called.
		Query::command_vector_t m_commandChain;

		/// Cache the fields and their aggregate functions specified in the select command.
		Query::select_args_t m_selectArgs;

		/// Cache the fields that will have aggregate functions run on them
		Query::select_args_t m_aggregateCommands;
};

#endif
