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
	if (m_dataStoreFile)
	{
		return;
	}

	m_dataStoreFile.open(connectionString, std::fstream::in | std::fstream::out | std::fstream::trunc);
	if (!m_dataStoreFile)
	{
		throw std::invalid_argument("Unable to open file: " + connectionString);
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

std::map<std::string, Model> Repository::GetModels() const
{
	std::map<std::string, Model> models;
	// TODO: Read from data store on disk
	return models;
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
	// Don't process an empty model object
	if (!model)
	{
		return;
	}

	m_dataStoreCache[model.Key()] = model;
	// TODO: Create in data store on disk
	return;
}

void Repository::UpdateModel(const Model& model)
{
	// Don't process an empty model object
	if (!model)
	{
		return;
	}

	m_dataStoreCache[model.Key()] = model;
	// TODO: Update data store on disk
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
