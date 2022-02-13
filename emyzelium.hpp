/*
 * Emyzelium (C++)
 *
 * This is another gigathin wrapper around ZeroMQ's Publish-Subscribe and
 * Pipeline messaging patterns with mandatory Curve security and optional ZAP
 * authentication filter over TCP/IP for distributed artificial elife,
 * decision making etc. systems where each peer, identified by its public key,
 * provides and updates vectors of vectors of bytes under unique topics that
 * other peers can subscribe to and receive; peers obtain each other's
 * IP addresses:ports by sending beacons and subscribing to nameservers whose
 * addresses:ports are known.
 * 
 * https://github.com/emyzelium/emyzelium-cpp
 * 
 * emyzelium@protonmail.com
 * 
 * Copyright (c) 2022 Emyzelium caretakers
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


const string VERSION = "0.7.2";
const string DATE = "2022.02.13";

enum class EW {
	Ok,
	AlreadyPresent,
	AlreadyAbsent,
	AlreadyPaused,
	AlreadyResumed,
	Absent
};

const string DEF_IP = "127.0.0.1";

const uint16_t DEF_EFUNGI_PUBSUB_PORT = 0xEDAF; // 60847
const uint16_t DEF_ECATAL_BEACON_PORT = 0xCAEB; // 51947
const uint16_t DEF_ECATAL_PUBSUB_PORT = 0xD21F; // 53791

const int64_t DEF_EFUNGI_ECATAL_FORGET_INTERVAL = 60000000; // in microseconds
const int64_t DEF_EFUNGI_BEACON_INTERVAL = 2000000;

const int64_t DEF_ECATAL_DEACTIVATE_INTERVAL = 60000000;
const int64_t DEF_ECATAL_PUBLISH_INTERVAL = 1000000;
const int64_t DEF_ECATAL_IDLE_INTERVAL = 10000;


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
	int64_t ecatal_forget_interval;
	string connpoint;
	unordered_map<string, tuple<string, int64_t>> connpoints_via_ecatals;
	unordered_map<string, Etale> etales;

	void update_connpoint_via_ecatal(const string& ecatal_publickey, const string& connpoint, const int64_t t);
	void remove_connpoint_via_ecatal(const string& ecatal_publickey);

	void update();

public:
	// Owns socket, so cannot be copied
	Ehypha(const Ehypha&) = delete;
	Ehypha& operator=(const Ehypha&) = delete;

	Ehypha(zcontext* context, const string& secretkey, const string& publickey, const string& serverkey, const int64_t ecatal_forget_interval, const string& connpoint);

	void set_connpoint(const string& connpoint);

	tuple<const Etale&, EW> add_etale(const string& title);
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
	int64_t beacon_interval;
	int64_t t_last_beacon;
	int64_t ecatal_forget_interval;
	unordered_map<string, Ehypha> ehyphae;
	zcontext* context;
	zsocket* zapsock;
	zsocket* pubsock;
	unordered_map<string, zsocket*> ecatals_from;
	unordered_map<string, zsocket*> ecatals_to;

public:
	// Owns context and sockets, so cannot be copied
	Efunguz(const Efunguz&) = delete;
	Efunguz& operator=(const Efunguz&) = delete;

	Efunguz(const string& secretkey, const unordered_set<string>& whitelist_publickeys=unordered_set<string>{}, const uint16_t pubsub_port=DEF_EFUNGI_PUBSUB_PORT, const int64_t beacon_interval=DEF_EFUNGI_BEACON_INTERVAL, const int64_t ecatal_forget_interval=DEF_EFUNGI_ECATAL_FORGET_INTERVAL);

	void add_whitelist_publickeys(const unordered_set<string>& publickeys);
	void del_whitelist_publickeys(const unordered_set<string>& publickeys);

	tuple<Ehypha&, EW> add_ehypha(const string& that_publickey, const string& connpoint="", const int64_t ecatal_forget_interval=0);
	EW del_ehypha(const string& that_publickey);

	EW add_ecatal_from(const string& that_publickey, const string& connpoint);
	EW del_ecatal_from(const string& that_publickey);

	EW add_ecatal_to(const string& that_publickey, const string& connpoint);
	EW del_ecatal_to(const string& that_publickey);

	void emit_etale(const string& title, const vector<vector<uint8_t>>& parts);
	void emit_beacon();

	void update();

	~Efunguz();
};


class Ecataloguz {
	string secretkey;
	string publickey;
	unordered_set<string> beacon_whitelist_publickeys;
	unordered_set<string> pubsub_whitelist_publickeys;
	unordered_map<string, tuple<string, int64_t, string>> beacon_recs; // publickey -> {connpoint, beacon_time, comment}
	uint16_t beacon_port;
	uint16_t pubsub_port;
	int64_t deactivate_interval;
	int64_t publish_interval;
	int64_t idle_interval;
	zcontext* context;
	zsocket* zapsock;
	zsocket* pullsock;
	zsocket* pubsock;
	
public:
	// Owns context and sockets, so cannot be copied
	Ecataloguz(const Ecataloguz&) = delete;
	Ecataloguz& operator=(const Ecataloguz&) = delete;

	Ecataloguz(const string& secretkey, const unordered_map<string, string>& beacon_whitelist_publickeys_with_comments=unordered_map<string, string>{}, const unordered_set<string>& pubsub_whitelist_publickeys=unordered_set<string>{}, const uint16_t beacon_port=DEF_ECATAL_BEACON_PORT, const uint16_t pubsub_port=DEF_ECATAL_PUBSUB_PORT, const int64_t deactivate_interval=DEF_ECATAL_DEACTIVATE_INTERVAL, const int64_t publish_interval=DEF_ECATAL_PUBLISH_INTERVAL, const int64_t idle_interval=DEF_ECATAL_IDLE_INTERVAL);

	EW read_beacon_whitelist_publickeys_with_comments(string filepath);
	EW read_pubsub_whitelist_publickeys(string filepath);

	void run();

	~Ecataloguz();
};

}

#endif