#ifndef QUERY_H
#define QUERY_H

#include <tuple>
#include "model.h"


// need ordered commands with precedence
class Query
{
	public:
		enum Command {
			SelectCommand,
			OrderCommand,
			GroupCommand,
			FilterCommand,
			InvalidCommand,
		};

		// Type defines these data structures so implementation is easier to read/change
		typedef std::vector<std::vector<std::string>> table_t;
		typedef std::vector<std::tuple<Query::Command, std::string>> command_queue_t;

		// Construction
		Query() = delete;
		Query(const std::string& queryString);
		~Query();

		// Public API
		table_t QueryCommand(std::istream& inputStream);

	private:
		// Query API
		table_t Select(std::istream& inputStream, const std::string& fields);
		table_t Order(const std::string& fields);
		table_t Group(const std::string& field);
		table_t Filter(const std::string& filter);

		/// Validates command options and their respective values in the given string.
		bool IsValidQueryString(std::string queryString) const;

		/// Creates an ordered collection of commands to perform from the given query string.
		Query::command_queue_t ParseQueryString(const std::string& queryString);

		/// Map of known strings to their related Query functions for parsing query strings.
		static const std::map<std::string, Query::Command> m_knownCommands;

		/// Ordered collection of commands to be performed when QueryCommand is called.
		command_queue_t m_commandChain;
};

#endif
