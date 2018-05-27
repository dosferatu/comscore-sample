#ifndef MODEL_H
#define MODEL_H

#include <map>
#include <ostream>
#include <string>
#include <vector>

// TODO: Enforce field lengths
class Model
{
	public:
		enum SerializeMode {
			DataStore,
			Query,
		};

		typedef std::vector<std::string> field_list_t;
		typedef std::map<std::string, std::string> field_value_map_t;

		// Construction
		Model();
		Model(const std::string& modelRecord);

		// Operator overloads
		/// Implements a check for a default-constructed (empty) record model.
		bool operator! () const;

		/// Stream a textual record in as a Model object.
		friend std::istream& operator>> (std::istream& inStream, Model& model);


		// Public API
		/// Returns a string that is a hash of the 'stb', 'title', and 'date' field values.
		std::string Key() const;

		/// Set the value for a given field if the field is known by the schema.
		void Field(const std::string& field, const std::string& fieldValue);

		/// Get the value for a given field if the field is known by the schema.
		std::string Field(const std::string& field) const;

		/// Sets an override for the defaults ordering for serialization via ToString()
		void SetOrdering(const Model::field_list_t fieldOrdering);

		/// Serialize this object as a string using a mode parameter vs creating a custom
		/// I/O manipulator to support ostream custom formatting
		std::string ToString(Model::SerializeMode mode) const;


		/// Used as a schema for all the valid field names this record model defines.
		static const Model::field_list_t m_validFields;

	private:
		/// Contains the values for the fields this record model defines.
		Model::field_value_map_t m_fields;

		/// Specifies what fields and which order are to be printed in ToString().
		/// Default constructed to the known fields set in Model::m_knownFields.
		Model::field_list_t m_fieldOrdering;

		/// Set to true after any field has been modified from its default value.
		bool m_hasData;
};

#endif
