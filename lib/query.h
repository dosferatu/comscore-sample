#ifndef QUERY_H
#define QUERY_H

#include <functional>
#include "model.h"

// need ordered commands with precedence
class Query
{
	public:
		Query() = delete;
		Query(const std::string& queryString);
		std::vector<Model> QueryCommand(const std::istream& inputStream) const;

	private:
		std::vector<Model> Select(std::string fields);
		std::vector<Model> Order(std::string order);
		std::vector<Model> Group(std::string query);
		std::vector<Model> Filter(std::string query);

		std::string m_queryString;

		/// Map of known strings to their related Query functions for parsing query strings.
		//static const std::map<std::string, std::function<std::vector<Model>(std::string)>> m_commands;
};

#endif
