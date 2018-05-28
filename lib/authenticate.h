#ifndef AUTHENTICATE_H
#define AUTHENTICATE_H

#include <string>

// Simple mock credentials object
class Credentials
{
	public:
		Credentials() : m_clientId(), m_authenticationToken() {}
		~Credentials() {}
		std::string ClientId() const { return m_clientId; }
		void ClientId(const std::string& clientId) { m_clientId = clientId; }

		std::string AuthenticationToken() const { return m_authenticationToken; }
		void AuthenticationToken(const std::string& authenticationToken) { m_authenticationToken = authenticationToken; }

	private:
		std::string m_clientId;
		std::string m_authenticationToken;
};

/// Simple mock security authenticator
class IAuthenticate
{
	public:
		virtual bool Authenticate(const Credentials& credentials) const = 0;
		virtual Credentials Connect(const std::string& clientId, const std::string& credentials) = 0;
		virtual void Disconnect(const Credentials& credentials) = 0;
		virtual ~IAuthenticate() {}
};

#endif
