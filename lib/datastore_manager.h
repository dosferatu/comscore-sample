#ifndef DATASTORE_MANAGER_H
#define DATASTORE_MANAGER_H

#include <string>
#include <map>
#include "repository.h"

/// Simple mock security authenticator
class IAuthenticate
{
	public:
		virtual bool Authenticate(const std::string& authenticationToken) = 0;
		virtual std::string Connect(const std::string& clientId, const std::string& credentials) = 0;
		virtual void Disconnect(const std::string& authenticationToken) = 0;
		virtual ~IAuthenticate() {}
};

/// Manager for data access layer
class DataStoreManager : public IAuthenticate
{
	public:
		DataStoreManager() = delete;
		DataStoreManager(IRepository& repository, const std::string& dataStorePath);

		void ImportData(const std::string& authenticationToken, const std::string& dataFilePath);

		// IAuthenticate implementation
		bool Authenticate(const std::string& authenticationToken) override;
		std::string Connect(const std::string& clientId, const std::string& credentials) override;
		void Disconnect(const std::string& authenticationToken) override;

	private:
		/// Data access interface
		IRepository& m_repository;

		/// Map of security tokens to their respective client IDs to cache
		/// clients that have been authenticated for using the datastore.
		std::map<std::string, std::string> m_authenticatedClients;
};

#endif
