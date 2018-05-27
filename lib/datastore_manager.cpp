#include <iostream>
#include <random>
#include <string>
#include "datastore_manager.h"
#include "model.h"


// ****************************************************************************
// Construction
// ****************************************************************************
DataStoreManager::DataStoreManager(IRepository& repository, const std::string& dataStorePath)
	: m_repository(repository), m_authenticatedClients()
{
	m_repository.Connect(dataStorePath);
}


// ****************************************************************************
// Public API
// ****************************************************************************
void DataStoreManager::ImportData(const Credentials& credentials, const std::string& importDataPath)
{
	if (!this->Authenticate(credentials)) {
		std::cout << "Unable to authenticate token " << credentials.AuthenticationToken()
			<< " for client " << credentials.ClientId() << std::endl;
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

Query::table_t DataStoreManager::QueryData(const Credentials& credentials, Query& query)
{
	Query::table_t results;
	if (!this->Authenticate(credentials)) {
		std::cout << "Unable to authenticate token " << credentials.AuthenticationToken()
			<< " for client " << credentials.ClientId() << std::endl;
		return results;
	}

	return m_repository.QueryData(query);
}


// ****************************************************************************
// IAuthenticate implementation
// ****************************************************************************
bool DataStoreManager::Authenticate(const Credentials& credentials) const
{
	if (m_authenticatedClients.count(credentials.AuthenticationToken()) == 0) {
		return false;
	}

	return (m_authenticatedClients.at(credentials.AuthenticationToken()) == credentials.ClientId());
}

Credentials DataStoreManager::Connect(const std::string& clientId, const std::string& password)
{
	// Better than Blockchain
	Credentials credentials;
	std::string authenticationToken = "";
	if (!password.empty()) {
		authenticationToken = std::to_string(std::rand()); // definitely a true random number
		credentials.AuthenticationToken(authenticationToken);
		credentials.ClientId(clientId);
		m_authenticatedClients[authenticationToken] = clientId;
	}

	// TODO: Return invalid credentials? throw? how to handle failed connection.
	return credentials;
}

void DataStoreManager::Disconnect(const Credentials& credentials)
{
	if (m_authenticatedClients.count(credentials.AuthenticationToken()) == 0) {
		return;
	}

	if (m_authenticatedClients.at(credentials.AuthenticationToken()) == credentials.ClientId()) {
		auto client = m_authenticatedClients.find(credentials.AuthenticationToken());
		m_authenticatedClients.erase(client);
	}
}
