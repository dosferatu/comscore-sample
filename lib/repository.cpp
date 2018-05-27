#include <algorithm>
#include <iostream>
#include "model.h"
#include "repository.h"


// ****************************************************************************
// Construction
// ****************************************************************************
Repository::Repository() : m_dataStoreFile(), m_dataStoreCache()
{
}

Repository::~Repository()
{
}


// ****************************************************************************
// IRepository implementation
// ****************************************************************************
void Repository::Connect(const std::string& connectionString)
{
	// Check if already connected
	if (m_dataStoreFile.is_open())
	{
		return;
	}

	// Try to either open or create the datastore file
	m_dataStoreFile.open(connectionString, std::ios::in | std::ios::out);
	if (!m_dataStoreFile.is_open()) {
		m_dataStoreFile.open(connectionString, std::ios::in | std::ios::out | std::ios::trunc);
	}

	if (!m_dataStoreFile.is_open()) {
		throw std::invalid_argument("Unable to create file: " + connectionString);
	}

	return;
}

void Repository::Disconnect()
{
	if (m_dataStoreFile) {
		m_dataStoreFile.close();
	}

	return;
}

Query::table_t Repository::QueryData(Query& query)
{
	m_dataStoreFile.seekg(0, std::ios::beg);
	return query.QueryCommand(m_dataStoreFile);
}

Model Repository::GetModelByKey(const std::string& key) const
{
	Model model;

	// Check if object is cached already
	if (m_dataStoreCache.count(key) != 0)
	{
		model = m_dataStoreCache.at(key);	
	}
	else
	{
		// TODO: Read from data store on disk
	}

	return model;
}

void Repository::CreateModel(const Model& model)
{
	// Implementation for create and update are the same for this demo.
	this->UpdateModel(model);
	return;
}

void Repository::UpdateModel(const Model& model)
{
	// Don't process an empty model object
	if (!model)
	{
		return;
	}

	// TODO: Enforce a configured limit to size of memory cache.
	m_dataStoreCache.emplace(model.Key(), model);

	// Update the record on disk if present; create it otherwise.
	// Could memory map portions of a file using configured memory size limit.
	// This way an ordering scheme can be implemented if necessary.
	bool foundMatchingLogicalRecord = false;
	std::streampos recordPos = 0;
	std::string recordString = "";
	m_dataStoreFile.seekg(0, std::ios::beg);
	while (m_dataStoreFile.good() && std::getline(m_dataStoreFile, recordString)) {
		Model recordModel(recordString);
		if (!recordModel) {
			// Encountered an invalid record somehow, so go to next line.
			continue;
		} else if (recordModel.Key() == model.Key()) {
			foundMatchingLogicalRecord = true;
			break;
		}

		// Save the position of the logically identical record so we can overwrite.
		recordPos = m_dataStoreFile.tellg();
	}

	// Seek to either the record we will overwrite, or to the end of the file to append.
	m_dataStoreFile.clear();
	if (foundMatchingLogicalRecord && recordPos >= 0) {
		m_dataStoreFile.seekp(recordPos);
	} else {
		m_dataStoreFile.seekp(0, std::ios::end);
	}

	// Write the record to the datastore file
	m_dataStoreFile << model.ToString(Model::SerializeMode::DataStore) << std::endl;
	if (m_dataStoreFile.fail())
	{
		throw std::runtime_error("Failed to write to datastore.");
	}

	return;
}

void Repository::DeleteModel(std::string& key)
{
	// Don't process an empty model key
	if (key.empty())
	{
		return;
	}

	if (m_dataStoreCache.count(key) > 0)
	{
		auto model = m_dataStoreCache.find(key);
		m_dataStoreCache.erase(model);
	}

	// TODO: Erase from data store on disk
	return;
}
