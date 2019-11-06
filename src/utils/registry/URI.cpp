#include "URI.h"
#include <algorithm>
#include <cctype>
#include <functional>

namespace PR {
void URI::parse(const std::string& s)
{
	mProtocol = "";
	mHost	 = "";
	mPort	 = "";
	mPath	 = "";
	mQuery	= "";
	mFragment = "";

	std::string::const_iterator path_i;

	const std::string prot_end("://");
	std::string::const_iterator prot_i = std::search(s.begin(), s.end(),
													 prot_end.begin(), prot_end.end());

	if (prot_i != s.end()) {
		mProtocol.reserve(std::distance(s.begin(), prot_i));
		std::transform(s.begin(), prot_i,
					   std::back_inserter(mProtocol),
					   std::ptr_fun<int, int>(std::tolower)); // protocol is icase

		std::advance(prot_i, prot_end.length());

		path_i = std::find(prot_i, s.end(), '/');

		const auto distAuth = std::distance(prot_i, path_i);
		if (distAuth > 0) {
			std::string auth;
			auth.reserve(distAuth);
			std::transform(prot_i, path_i,
						   std::back_inserter(auth),
						   std::ptr_fun<int, int>(std::tolower)); // host is icase

			if (!auth.empty()) {
				std::string::const_iterator port_i = std::find(auth.begin(), auth.end(), ':');
				mHost.assign(auth.cbegin(), port_i);

				if (port_i != auth.end()) {
					++port_i; // Skip :
					mPort.assign(port_i, auth.cend());
				}
			}
		}
	} else { // No protocol and no host
		path_i = s.begin();
	}

	std::string::const_iterator query_i = std::find(path_i, s.end(), '?');

	if (query_i != s.end()) { // Query found
		mPath.assign(path_i, query_i);

		++query_i; // Skip ?

		std::string::const_iterator fragment_i = std::find(query_i, s.end(), '#');
		mQuery.assign(query_i, fragment_i);

		if (fragment_i != s.end())
			++fragment_i; // Skip #

		mFragment.assign(fragment_i, s.cend());
	} else { // No query found -> But maybe fragment?
		std::string::const_iterator fragment_i = std::find(path_i, s.end(), '#');
		mPath.assign(path_i, fragment_i);

		if (fragment_i != s.end())
			++fragment_i; // Skip #

		mFragment.assign(fragment_i, s.cend());
	}
}

URI URI::makeAbsolute(const URI& p, const URI& base)
{
	if (p.isAbsolute())
		return p;

	if (!base.isValid() || !p.isValid())
		throw std::runtime_error("Given URI should be valid");

	if (!base.isAbsolute())
		throw std::runtime_error("Given base URI is not absolute");

	if (!base.fragment().empty() && !base.query().empty())
		throw std::runtime_error("Base URI containing query or fragment information is not supported");

	std::string newPath = base.path();
	PR_ASSERT(!newPath.empty(), "Absolute base path can not be empty");

	if (*newPath.end() != '/')
		newPath += '/';

	std::string::const_iterator it = p.path().cbegin();
	if (*it == '.') {
		it++;
		if (it != p.path().cend()) {
			if (*it == '.') // Potential Directory Up... -> Currently not supported :(
			{
				++it;
				if (it == p.path().cend() || *it == '/')
					throw std::runtime_error("Currently no support for ../");
				else // .. is part of the path
					newPath += p.path();
			} else if (*it == '/') {
				++it;
				if (it != p.path().cend())
					newPath.append(it, p.path().cend()); // TODO: Recursive?
			} else {									 // . is part of the path
				newPath += p.path();
			}
		}
		// else -> Ignore. Just a path with .
	} else {
		PR_ASSERT(*it != '/', "p can not be absolute!");
		newPath += p.path();
	}

	URI newURI;
	newURI.setProtocol(base.protocol());
	newURI.setHost(base.host());
	newURI.setPort(base.port());
	newURI.setPath(newPath);
	newURI.setQuery(p.query());
	newURI.setFragment(p.fragment());

	return newURI;
}
} // namespace PR
