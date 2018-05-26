#include <istream>
#include "model.h"
#include "query.h"

Query::Query(const std::string& queryString) : m_queryString(queryString)
{
}

std::vector<Model> Query::QueryCommand(const std::istream& inputStream) const
{
	std::vector<Model> results;
	if (!inputStream.good())
	{
		throw std::invalid_argument("Input stream given to query is not valid");
	}

	return results;
}
