#ifndef MODEL_H
#define MODEL_H

#include <map>
#include <ostream>
#include <string>
#include <vector>

class Model
{
	public:
		enum SerializeMode {
			DataStore,
			Query,
		};

		typedef std::map<std::string, std::string> field_value_map_t;

		// Construction
		Model();
		Model(const std::string& modelRecord);

		// Operator overloads
		/// Implements a check for a default-constructed (empty) record model.
		bool operator! () const;

		/// Support streaming a record in and outusing the schema defined within this model.
		std::string ToString(Model::SerializeMode mode);
		friend std::ostream& operator<< (std::ostream& outStream, const Model& model);
		friend std::istream& operator>> (std::istream& inStream, Model& model);


		// Public API
		/// Returns a string that is a hash of the 'stb', 'title', and 'date' field values.
		std::string Key() const;

		/// Set the value for a given field if the field is known by the schema.
		void Field(const std::string& field, const std::string& fieldValue);

		/// Get the value for a given field if the field is known by the schema.
		std::string Field(const std::string& field) const;

		/// Used as a schema for all the valid field names this record model defines.
		static const std::vector<std::string> m_validFields;

	private:
		/// Contains the values for the fields this record model defines.
		field_value_map_t m_fields;

		/// Set to true after any field has been modified from its default value.
		bool m_hasData;
};

#endif
