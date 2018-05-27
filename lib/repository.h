#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <fstream>
#include <string>
#include <vector>
#include "model.h"
#include "query.h"

typedef std::map<std::string, Model> data_cache_t;

class IRepository
{
	public:
		virtual void Connect(const std::string& connectionString) = 0;
		virtual void Disconnect() = 0;
		virtual Query::table_t QueryData(Query& query) = 0;
		virtual Model GetModelByKey(const std::string& key) const = 0;
		virtual void CreateModel(const Model& model) = 0;
		virtual void UpdateModel(const Model& model) = 0;
		virtual void DeleteModel(std::string& key) = 0;
		virtual ~IRepository() {}
};

class Repository : public IRepository
{
	public:
		Repository();
		~Repository();

		// IRepository implementation
		void Connect(const std::string& connectionString) override;
		void Disconnect() override;
		Query::table_t QueryData(Query& query) override;
		Model GetModelByKey(const std::string& key) const override;
		void CreateModel(const Model& model) override;
		void UpdateModel(const Model& model) override;
		void DeleteModel(std::string& key) override;

	private:
		void ValidateDataStore();

		/// File handle for the persistent data store.
		std::fstream m_dataStoreFile;

		/// Memory cache of data store.
		// TODO: Implement IDs / state for faster file store schemes
		data_cache_t m_dataStoreCache;
};

#endif
