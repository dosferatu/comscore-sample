#ifndef MODEL_H
#define MODEL_H

#include <map>
#include <ostream>
#include <string>
#include <vector>

class Model
{
	public:
		// Construction
		Model();
		Model(const std::string& modelRecord);


		// Operator overloads
		/// Implements a check for a default-constructed (empty) record model.
		bool operator! () const;

		/// Support streaming out a record using the schema defined within this model.
		friend std::ostream &operator<< (std::ostream &outStream, Model const &model);


		// Public API
		/// Returns a string that is a hash of the 'stb', 'title', and 'date' field values.
		std::string  Key() const;

		/// Set the value for a given field if the field is known by the schema.
		void         Field(const std::string& field, const std::string& fieldValue);

		/// Get the value for a given field if the field is known by the schema.
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
