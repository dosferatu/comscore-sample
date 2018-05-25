#ifndef MODEL_H
#define MODEL_H

#include <map>
#include <string>
#include <vector>

class Model
{
	public:
		Model();
		Model(const std::string& modelRecord);

		bool operator! () const { return m_hasData; }

		// Public API
		std::string  Key() const;
		void         Field(const std::string& field, const std::string& fieldValue);
		std::string  Field(const std::string& field) const;

		/// Used as a schema for all the valid field names this record model defines.
		static const std::vector<std::string> m_validFields;

	private:
		/// Contains the values for the fields this record model defines.
		std::map<std::string, std::string> m_fields;

		/// Set to true after any field has been modified from its default value.
		bool m_hasData;
};

#endif
