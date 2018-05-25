#include <iostream>
#include <random>
#include <string>
#include "datastore_manager.h"
#include "model.h"


// ****************************************************************************
// Construction
// ****************************************************************************
DataStoreManager::DataStoreManager(IRepository& repository, const std::string& dataStorePath) : m_repository(repository), m_authenticatedClients()
{
	m_repository.Connect(dataStorePath);
}


// ****************************************************************************
// Public API
// ****************************************************************************
void DataStoreManager::ImportData(const std::string& authenticationToken, const std::string& importDataPath)
{
	if (m_authenticatedClients.count(authenticationToken) == 0) {
		std::cout << "Unable to authenticate token " << authenticationToken << std::endl;
		return;
	}

	std::ifstream importDataFile(importDataPath);
	if (!importDataFile)
	{
		throw std::invalid_argument("Unable to open file: " + importDataPath);
	}

	std::string record;
	while (std::getline(importDataFile, record))
	{
		Model model(record);
		if (!model)
		{
			continue;
		}

		m_repository.CreateModel(model);
	}
}


// ****************************************************************************
// IAuthenticate implementation
// ****************************************************************************
bool DataStoreManager::Authenticate(const std::string& authenticationToken)
{
	return (m_authenticatedClients.count(authenticationToken) > 0);
}

std::string DataStoreManager::Connect(const std::string& clientId, const std::string& credentials)
{
	// Better than Blockchain
	std::string authenticationToken = "";
	if (!credentials.empty()) {
		authenticationToken = std::to_string(std::rand()); // definitely a true random number
		m_authenticatedClients[clientId] = authenticationToken;
	}

	return authenticationToken;
}

void DataStoreManager::Disconnect(const std::string& authenticationToken)
{
	if (m_authenticatedClients.count(authenticationToken) > 0) {
		auto client = m_authenticatedClients.find(authenticationToken);
		m_authenticatedClients.erase(client);
	}
	
}
