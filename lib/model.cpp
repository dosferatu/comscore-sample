#include <sstream>
#include "model.h"


// ****************************************************************************
// Static initialization
// ****************************************************************************
const std::vector<std::string> Model::m_validFields =
{
	{ "stb" },      // The set top box id on which the media asset was viewed. (Text, max size 64 char).
	{ "title" },    // The title of the media asset. (Text, max size 64 char).
	{ "provider" }, // The distributor of the media asset. (Text, max size 64 char).
	{ "date" },     // The local date on which the content was leased by through the STB (A date in YYYY-MM-DD format).
	{ "rev" },      // The price incurred by the STB to lease the asset. (Price in US dollars and cents).
	{ "viewtime" }, // The amount of time the STB played the asset.  (Time in hours:minutes).
};


// ****************************************************************************
// Construction
// ****************************************************************************
Model::Model() : m_fields(), m_hasData(false)
{
	for (auto& field : Model::m_validFields) {
		m_fields[field] = "";
	}
}

Model::Model(const std::string& modelRecord) : m_fields(), m_hasData(false)
{
	// Use the known valid fields collection as a schema for parsing.
	// Could inject a schema dependency into this constructor instead.
	std::string fieldValue = "";
	std::istringstream iss(modelRecord);
	for (auto& field : Model::m_validFields) {
		// Set the field's value if there was a matching token to parse.
		if (std::getline(iss, fieldValue, '|')) {
			m_fields[field] = fieldValue;
			m_hasData = true; // Flag that we are no longer in default constructed state.
		} else {
			m_fields[field] = "";
		}
	}
}


// ****************************************************************************
// Public API
// ****************************************************************************
std::string Model::Key() const
{
	// Apply constraint that models be unique by fields 'stb', 'title', and 'date'
	return m_fields.at("stb") + m_fields.at("title") + m_fields.at("date");
}

void Model::Field(const std::string& field, const std::string& fieldValue)
{
	// Known fields are populated to null values on construction of this class.
	if (m_fields.count(field) == 0) {
		throw std::invalid_argument("Unknown field: " + field);
	}

	m_fields[field] = fieldValue;
	m_hasData = true; // Flag that we are no longer in default constructed state.
	return;
}

std::string Model::Field(const std::string& field) const
{
	if (m_fields.count(field) == 0) {
		throw std::invalid_argument("Field not in schema: " + field);
	}

	return m_fields.at(field);
}
