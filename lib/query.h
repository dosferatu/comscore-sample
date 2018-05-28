#ifndef QUERY_H
#define QUERY_H

#include <tuple>
#include "model.h"


// need ordered commands with precedence
class Query
{
	public:
		enum class Command {
			SelectCommand,   // Query and filter commands
			OrderCommand,
			GroupCommand,
			FilterCommand,
			MinCommand,      // Aggregate commands
			MaxCommand,
			SumCommand,
			CountCommand,
			CollectCommand,
			InvalidCommand,  // Used for initialization
			NoCommand,       // Used to indicate that the given parameter does not have an associated command
		};

		// Type defines these data structures so implementation is easier to read/change
		typedef Model row_t;           /// Represents a record produced by a query
		typedef std::vector<Model> table_t;  /// Collection of records produced by a query

		/// Collection of fields + aggregate commands
		typedef std::vector<std::tuple<std::string, Query::Command>> select_command_t;

		/// Collection of commands + arguments
		typedef std::vector<std::tuple<Query::Command, std::string>> command_queue_t;

		/// Construction
		Query() = delete;
		Query(const std::string& queryString);
		~Query();

		// Public API
		Query::table_t QueryCommand(std::istream& inputStream);
		static bool IsAggregateCommand(Query::Command command);
		static bool IsValidQueryString(const std::string& queryString);

	private:
		// Query API
		Query::table_t Select(std::istream& inputStream, const std::string& commandArgs);
		void Order(Query::table_t& queryData, const std::string& fields);
		void Group(Query::table_t& queryData, const std::string& groupField);
		void Filter(Query::table_t& queryData, const std::string& filter);
		void Aggregate(Query::table_t& queryData, const std::string& groupField);

		/// Creates an ordered collection of commands to perform from the given query string.
		static Query::command_queue_t ParseQueryString(const std::string& queryString);

		/// Parse the select command argument for fields and their respective aggregate functions.
		static Query::select_command_t ParseSelectCommandArgs(const std::string& commandArgs);

		/// Map of known strings to their related Query functions for parsing query strings.
		static const std::map<std::string, Query::Command> m_knownCommands;

		/// Ordered collection of commands to be performed when QueryCommand is called.
		command_queue_t m_commandChain;

		/// Cache the fields and their aggregate functions specified in the select command.
		Query::select_command_t m_selectCommands;
};

#endif
