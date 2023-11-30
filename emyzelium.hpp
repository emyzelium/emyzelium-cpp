/*
 * Emyzelium (C++)
 *
 * is another wrapper around ZeroMQ's Publish-Subscribe messaging pattern
 * with mandatory Curve security and optional ZAP authentication filter,
 * over Tor, through Tor SOCKS proxy,
 * for distributed artificial elife, decision making etc. systems where
 * each peer, identified by its public key, onion address, and port,
 * publishes and updates vectors of vectors of bytes of data
 * under unique topics that other peers subscribe to
 * and receive the respective data.
 * 
 * https://github.com/emyzelium/emyzelium-cpp
 * 
 * emyzelium@protonmail.com
 * 
 * Copyright (c) 2022-2023 Emyzelium caretakers
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

/*
 * Header
 */ 

#ifndef EMYZELIUM_HPP
#define EMYZELIUM_HPP


#include <zmq.h>

#include <cstdint>
#include <unordered_map>
#include <string>
#include <unordered_set>
#include <tuple>
#include <vector>


using namespace std;


namespace Emyzelium {


namespace typeAliases {
	using zcontext = void;
	using zsocket = void;
}

using namespace typeAliases;


const string LIB_VERSION = "0.9.6";
const string LIB_DATE = "2023.11.30";

enum class EW {
	Ok 				= 0,
	AlreadyPresent 	= 1,
	AlreadyAbsent 	= 2,
	AlreadyPaused 	= 3,
	AlreadyResumed 	= 4,
	Absent 			= 5
};

const uint16_t DEF_PUBSUB_PORT = 0xEDAF; // 60847

const uint16_t DEF_TOR_PROXY_PORT = 9050; // default from /etc/tor/torrc
const string DEF_TOR_PROXY_HOST = "127.0.0.1";  // default from /etc/tor/torrc


class Etale {
	friend class Ehypha;

	bool paused;

public:
	Etale(const vector<vector<uint8_t>>& parts={}, const int64_t t_out=-1, const int64_t t_in=-1, const bool paused=false);

	vector<vector<uint8_t>> parts;
	int64_t t_out;
	int64_t t_in;
};


class Ehypha {
	friend class Efunguz;
	
	zsocket* subsock;
	unordered_map<string, Etale> etales;

	void update();

public:
	// Owns socket, so cannot be copied
	Ehypha(const Ehypha&) = delete;
	Ehypha& operator=(const Ehypha&) = delete;

	Ehypha(zcontext* context, const string& secretkey, const string& publickey, const string& serverkey, const string& onion, const uint16_t pubsub_port, const uint16_t torproxy_port, const string& torproxy_host);

	tuple<const Etale&, EW> add_etale(const string& title);
	tuple<const Etale*, EW> get_etale_ptr(const string& title);
	EW del_etale(const string& title);

	EW pause_etale(const string& title);
	EW resume_etale(const string& title);

	void pause_etales();
	void resume_etales();

	~Ehypha();
};


class Efunguz {
	string secretkey;
	string publickey;
	unordered_set<string> whitelist_publickeys;
	uint16_t pubsub_port;
	uint16_t torproxy_port;
	string torproxy_host;
	unordered_map<string, Ehypha> ehyphae;
	zcontext* context;
	zsocket* zapsock;
	vector<uint8_t> zap_session_id;
	zsocket* pubsock;

public:
	// Owns context and sockets, so cannot be copied
	Efunguz(const Efunguz&) = delete;
	Efunguz& operator=(const Efunguz&) = delete;

	Efunguz(const string& secretkey, const unordered_set<string>& whitelist_publickeys=unordered_set<string>{}, const uint16_t pubsub_port=DEF_PUBSUB_PORT, const uint16_t torproxy_port=DEF_TOR_PROXY_PORT, const string& torproxy_host=DEF_TOR_PROXY_HOST);

	void add_whitelist_publickeys(const unordered_set<string>& publickeys);
	void del_whitelist_publickeys(const unordered_set<string>& publickeys);
	void clear_whitelist_publickeys();
	void read_whitelist_publickeys(const string& filepath);

	tuple<Ehypha&, EW> add_ehypha(const string& that_publickey, const string& onion, const uint16_t pubsub_port=DEF_PUBSUB_PORT);
	tuple<Ehypha*, EW> get_ehypha_ptr(const string& that_publickey);
	EW del_ehypha(const string& that_publickey);

	void emit_etale(const string& title, const vector<vector<uint8_t>>& parts);

	void update();

	~Efunguz();
};


}

#endif