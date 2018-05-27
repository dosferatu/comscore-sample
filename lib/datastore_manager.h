#ifndef DATASTORE_MANAGER_H
#define DATASTORE_MANAGER_H

#include <string>
#include <map>
#include "authenticate.h"
#include "query.h"
#include "repository.h"

/// Manager for data access layer
class DataStoreManager : public IAuthenticate
{
	public:
		DataStoreManager() = delete;
		DataStoreManager(IRepository& repository, const std::string& dataStorePath);

		// Datastore API
		void ImportData(const Credentials& credentials, const std::string& importDataPath);
		Query::table_t QueryData(const Credentials& credentials, Query& query);

		// IAuthenticate implementation
		bool Authenticate(const Credentials& credentials) const override;
		Credentials Connect(const std::string& clientId, const std::string& credentials) override;
		void Disconnect(const Credentials& credentials) override;

	private:
		/// Data access interface
		IRepository& m_repository;

		/// Map of security tokens to their respective client IDs to cache
		/// clients that have been authenticated for using the datastore.
		std::map<std::string, std::string> m_authenticatedClients;
};

#endif
