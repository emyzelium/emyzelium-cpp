/*
 * Emyzelium (C++)
 *
 * Emyzelium is another gigathin wrapper around ZeroMQ's Publish-Subscribe and
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
 * Source
 */

#define EMYZELIUM_ECATAL_TUI


#include "emyzelium.hpp"

#define ZMQ_BUILD_DRAFT_API
#include <zmq.h>

#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>


#ifdef EMYZELIUM_ECATAL_TUI

#include <ncurses.h>
#include <sstream>

#endif // EMYZELIUM_ECATAL_TUI


using namespace std;


namespace Emyzelium {

const size_t KEY_BIN_LEN = 32;
const size_t KEY_Z85_LEN = 40;
const size_t KEY_Z85_CSTR_LEN = KEY_Z85_LEN + 1;

const char* ROUTING_ID_PUBSUB = "pubsub";
const char* ROUTING_ID_BEACON = "beacon";

const int DEF_IPV6_STATUS = 1;

const int DEF_LINGER = 0;

const size_t MAX_PUBLICKEYS_FILE_LINE_LEN = 96;


struct ZMQ_Message_Properties {
	// string RoutingId;
	// string SocketType;
	string userId;
	string peerAddress;
};


int64_t time_musec() {
	return chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
}


string cut_pad_str(const string& src, size_t len, char pad_ch=' ') {
	string dst(len, pad_ch);
	src.copy((char *)dst.data(), len);
	return dst;
}


vector<uint8_t> cstr_to_vec_u8(const char *s) {
	size_t l = strlen(s);
	vector<uint8_t> bs(l);
	memcpy(bs.data(), s, l);
	return bs;
}


int zmqp_poll_in(zsocket* socket, long timeout) {
	zmq_pollitem_t zpi;
	zpi.socket = socket;
	zpi.events = ZMQ_POLLIN;
	return zmq_poll(&zpi, 1, timeout);
}


void zmqp_send(zsocket* socket, const vector<vector<uint8_t>>& parts) {
	zmq_msg_t msg;
	for (size_t i = 0; i < parts.size(); i++) {
		size_t size = parts[i].size();
		void* part_data = malloc(size);
		memcpy(part_data, parts[i].data(), size);
		zmq_msg_init_data(&msg, part_data, size, [](void* data, void* hint) {free(data);}, NULL);
		if (zmq_msg_send(&msg, socket, (i + 1) < parts.size() ? ZMQ_SNDMORE : 0) < 0) {
			zmq_msg_close(&msg);
		}
	}
}


tuple<vector<vector<uint8_t>>, ZMQ_Message_Properties> zmqp_recv(zsocket* socket) {
	vector<vector<uint8_t>> parts{};
	ZMQ_Message_Properties props;
	bool propsNeeded = true;
	zmq_msg_t msg;
	int more = 0;
	do {
		zmq_msg_init(&msg);
		zmq_msg_recv(&msg, socket, 0);
		if (propsNeeded) {
			const char* userId = zmq_msg_gets(&msg, ZMQ_MSG_PROPERTY_USER_ID);
			props.userId = (userId != NULL) ? string(userId) : "";
			const char* peerAddress = zmq_msg_gets(&msg, ZMQ_MSG_PROPERTY_PEER_ADDRESS);
			props.peerAddress = (peerAddress != NULL) ? string(peerAddress) : "";
			propsNeeded = false;
		}
		size_t size = zmq_msg_size(&msg);
		vector<uint8_t> part(size);
		memcpy(part.data(), zmq_msg_data(&msg), size);
		// parts.push_back(move(part));
		parts.emplace_back(move(part));
		more = zmq_msg_get(&msg, ZMQ_MORE);
		zmq_msg_close(&msg);
	} while (more);

	return tuple<vector<vector<uint8_t>>, ZMQ_Message_Properties>{parts, props};
}


#ifdef EMYZELIUM_ECATAL_TUI

string to_str(int x, int base=10) {
	ostringstream oss;
	switch (base) {
		case 8:
			oss << oct;
			break;
		case 16:
			oss << hex << uppercase;
			break;
	}
	oss << x;
	return oss.str();
}


string to_str(double x, int prec=1, bool is_scientific=false) {
	ostringstream oss;
	oss.precision(prec);
	if (is_scientific) {
		oss << scientific;
	} else {
		oss << fixed;
	}
	oss << x;
	return oss.str();
}


void addstrattr(const string& str, int attr=0) {
	attrset(attr);
	addstr(str.c_str());
	attrset(0);	
}


void mvaddstrattr(int y, int x, const string& str, int attr=0) {
	move(y, x);
	addstrattr(str, attr);
}

#endif // EMYZELIUM_ECATAL_TUI


Etale::Etale(const vector<vector<uint8_t>>& parts, const int64_t t_out, const int64_t t_in, const bool paused)
: parts {parts}, t_out {t_out}, t_in {t_in}, paused {paused} {
}


void Ehypha::set_connpoint(const string& connpoint) {
	if (connpoint != this->connpoint) {
		if (!this->connpoint.empty()) {
			zmq_disconnect(this->subsock, this->connpoint.c_str());
		}
		this->connpoint = connpoint;
		if (!this->connpoint.empty()) {
			zmq_connect(this->subsock, this->connpoint.c_str());
		}
	}
}


void Ehypha::update_connpoint_via_ecatal(const string& ecatal_publickey, const string& connpoint, const int64_t t) {
	// Yeah, this-> is redundant here, unlike set_connpoint(). But 1st version was in Python...
	this->connpoints_via_ecatals.erase(ecatal_publickey);
	this->connpoints_via_ecatals.insert({ecatal_publickey, {connpoint, t}});
}


void Ehypha::remove_connpoint_via_ecatal(const string& ecatal_publickey) {
	this->connpoints_via_ecatals.erase(ecatal_publickey);
}


Ehypha::Ehypha(zcontext* context, const string& secretkey, const string& publickey, const string& serverkey, const int64_t ecatal_forget_interval, const string& connpoint)
: ecatal_forget_interval {ecatal_forget_interval} {
	this->subsock = zmq_socket(context, ZMQ_SUB);
	zmq_setsockopt(this->subsock, ZMQ_LINGER, &DEF_LINGER, sizeof(DEF_LINGER));
	zmq_setsockopt(this->subsock, ZMQ_CURVE_SECRETKEY, secretkey.c_str(), KEY_Z85_CSTR_LEN);
	zmq_setsockopt(this->subsock, ZMQ_CURVE_PUBLICKEY, publickey.c_str(), KEY_Z85_CSTR_LEN);
	zmq_setsockopt(this->subsock, ZMQ_CURVE_SERVERKEY, serverkey.c_str(), KEY_Z85_CSTR_LEN);

	this->connpoint = "";
	this->set_connpoint(connpoint);
}


tuple<const Etale&, EW> Ehypha::add_etale(const string& title) {
	if (this->etales.count(title) == 0) {
		zmq_setsockopt(this->subsock, ZMQ_SUBSCRIBE, title.c_str(), title.size() + 1);
		// emplace() instead of insert() to avoid redundant destruction and copying of Etale (see also Efunguz::add_ehypha())
		this->etales.emplace(piecewise_construct,
			tuple<string>{title},
			tuple<>{}
		);
		return tuple<const Etale&, EW>{this->etales.at(title), EW::Ok};
	} else {
		return tuple<const Etale&, EW>{this->etales.at(title), EW::AlreadyPresent};
	}
}


EW Ehypha::del_etale(const string& title) {
	if (this->etales.count(title) == 1) {
		this->etales.erase(title);
		zmq_setsockopt(this->subsock, ZMQ_UNSUBSCRIBE, title.c_str(), title.size() + 1);
		return EW::Ok;
	} else {
		return EW::AlreadyAbsent;
	}
}


EW Ehypha::pause_etale(const string& title) {
	if (this->etales.count(title) == 1) {
		Etale& etale = this->etales.at(title);
		if (!etale.paused) {
			zmq_setsockopt(this->subsock, ZMQ_UNSUBSCRIBE, title.c_str(), title.size() + 1);
			etale.paused = true;
			return EW::Ok;
		} else {
			return EW::AlreadyPaused;
		}
	} else {
		return EW::Absent;
	}
}


EW Ehypha::resume_etale(const string& title) {
	if (this->etales.count(title) == 1) {
		Etale& etale = this->etales.at(title);
		if (etale.paused) {
			zmq_setsockopt(this->subsock, ZMQ_SUBSCRIBE, title.c_str(), title.size() + 1);
			etale.paused = false;
			return EW::Ok;
		} else {
			return EW::AlreadyResumed;
		}
	} else {
		return EW::Absent;
	}
}


void Ehypha::pause_etales() {
	for (const auto& keyval : this->etales) {
		this->pause_etale(keyval.first);
	}
}

void Ehypha::resume_etales() {
	for (const auto& keyval : this->etales) {
		this->resume_etale(keyval.first);
	}
}


void Ehypha::update() {
	int64_t t = time_musec();

	map<string, int> connpoints_votes = {};
	for (const auto& keyval : this->connpoints_via_ecatals) {
		const auto& cp = get<0>(keyval.second);
		const auto& t_upd = get<1>(keyval.second);
		if ((this->ecatal_forget_interval < 0) || ((t - t_upd) <= this->ecatal_forget_interval)) {
			if (connpoints_votes.count(cp) == 1) {
				connpoints_votes[cp] += 1;
			} else {
				connpoints_votes[cp] = 1;
			}
		}
	}

	if (connpoints_votes.size() > 0) {
		int max_v = 0;
		string max_v_cp = "";
		for (const auto& keyval : connpoints_votes) {
			if (keyval.second > max_v) {
				max_v = keyval.second;
				max_v_cp = keyval.first;
			}
		}
		this->set_connpoint(max_v_cp);
	}

	while (zmqp_poll_in(this->subsock, 0) > 0) {
		auto msg_parts = get<0>(zmqp_recv(this->subsock));
		if (msg_parts.size() >= 2) {
			// 0th is topic, 1st is remote time, rest (optional) is data
			const vector<uint8_t>& topic = msg_parts[0];
			if ((topic.size() >= 1) && (topic[topic.size() - 1] == 0)) {
				string title((char *)topic.data());
				if (this->etales.count(title) == 1) {
					Etale& etale = this->etales.at(title);
					if (!etale.paused) {
						if (msg_parts[1].size() == 8) {
							etale.parts.clear();
							// etale.parts.shrink_to_fit();
							for (size_t i = 2; i < msg_parts.size(); i++) {
								etale.parts.emplace_back(move(msg_parts[i]));
							}
							etale.t_out = *((int64_t*)msg_parts[1].data());
							etale.t_in = t;
						}
					}
				}
			}
		}
	}
}


Ehypha::~Ehypha() {
	zmq_close(this->subsock);
}


Efunguz::Efunguz(const string& secretkey, const set<string>& whitelist_publickeys, const uint16_t pubsub_port, const int64_t beacon_interval, const int64_t ecatal_forget_interval)
: pubsub_port {pubsub_port}, beacon_interval {beacon_interval}, ecatal_forget_interval {ecatal_forget_interval} {
	this->secretkey = cut_pad_str(secretkey, KEY_Z85_LEN);

	uint8_t publickey_cstr[KEY_Z85_CSTR_LEN]{0};
	zmq_curve_public((char *)publickey_cstr, this->secretkey.c_str());
	this->publickey = cut_pad_str(string((char *)publickey_cstr), KEY_Z85_LEN);

	for (const auto& key : whitelist_publickeys) {
		this->whitelist_publickeys.insert(cut_pad_str(key, KEY_Z85_LEN));
	}

	this->t_last_beacon = -1;

	this->context = zmq_ctx_new();
	zmq_ctx_set(this->context, ZMQ_IPV6, DEF_IPV6_STATUS);

	this->zapsock = zmq_socket(this->context, ZMQ_REP);
	zmq_setsockopt(this->zapsock, ZMQ_LINGER, &DEF_LINGER, sizeof(DEF_LINGER));
	zmq_bind(this->zapsock, "inproc://zeromq.zap.01");

	this->pubsock = zmq_socket(this->context, ZMQ_PUB);
	zmq_setsockopt(this->pubsock, ZMQ_LINGER, &DEF_LINGER, sizeof(DEF_LINGER));
	int one = 1;
	zmq_setsockopt(this->pubsock, ZMQ_CURVE_SERVER, &one, sizeof(one));
	zmq_setsockopt(this->pubsock, ZMQ_CURVE_SECRETKEY, this->secretkey.c_str(), KEY_Z85_CSTR_LEN);
	zmq_setsockopt(this->pubsock, ZMQ_ROUTING_ID, ROUTING_ID_PUBSUB, strlen(ROUTING_ID_PUBSUB));
	zmq_bind(this->pubsock, (string("tcp://*:") + to_string(this->pubsub_port)).c_str());
}


void Efunguz::add_whitelist_publickeys(const set<string>& publickeys) {
	for (const auto& key : publickeys) {
		this->whitelist_publickeys.insert(cut_pad_str(key, KEY_Z85_LEN));
	}
}


void Efunguz::del_whitelist_publickeys(const set<string>& publickeys) {
	for (const auto& key : publickeys) {
		this->whitelist_publickeys.erase(cut_pad_str(key, KEY_Z85_LEN));
	}
}


tuple<Ehypha&, EW> Efunguz::add_ehypha(const string& that_publickey, const string& connpoint, const int64_t ecatal_forget_interval) {
	string serverkey = cut_pad_str(that_publickey, KEY_Z85_LEN);
	if (this->ehyphae.count(serverkey) == 0) {
		int64_t ecatal_forget_interval_actual = (ecatal_forget_interval != 0) ? ecatal_forget_interval : this->ecatal_forget_interval;
		// insert() would destroy temporary Ehypha, whose destructor would close its subsock, making copied subsock ptr useless
		// (see the pair constructor in stl_pair.h that uses piecewise_construct_t)
		this->ehyphae.emplace(piecewise_construct,
			tuple<string>{serverkey},
			tuple<zcontext*, string, string, string, int64_t, string>{this->context, this->secretkey, this->publickey, serverkey, ecatal_forget_interval_actual, connpoint}
		);
		for (const auto& keyval : this->ecatals_from) {
			zmq_setsockopt(keyval.second, ZMQ_SUBSCRIBE, serverkey.c_str(), KEY_Z85_CSTR_LEN - 1);
		}
		return tuple<Ehypha&, EW>{this->ehyphae.at(serverkey), EW::Ok};
	} else {
		return tuple<Ehypha&, EW>{this->ehyphae.at(serverkey), EW::AlreadyPresent};
	}
}


EW Efunguz::del_ehypha(const string& that_publickey) {
	string serverkey = cut_pad_str(that_publickey, KEY_Z85_LEN);
	if (this->ehyphae.count(serverkey) == 1) {
		this->ehyphae.erase(serverkey);
		for (const auto& keyval : this->ecatals_from) {
			zmq_setsockopt(keyval.second, ZMQ_UNSUBSCRIBE, serverkey.c_str(), KEY_Z85_CSTR_LEN - 1);
		}
		return EW::Ok;
	} else {
		return EW::AlreadyAbsent;
	}
}


EW Efunguz::add_ecatal_from(const string& that_publickey, const string& connpoint) {
	string serverkey = cut_pad_str(that_publickey, KEY_Z85_LEN);
	if (this->ecatals_from.count(serverkey) == 0) {
		zsocket* subsock = zmq_socket(this->context, ZMQ_SUB);
		zmq_setsockopt(subsock, ZMQ_LINGER, &DEF_LINGER, sizeof(DEF_LINGER));
		zmq_setsockopt(subsock, ZMQ_CURVE_SECRETKEY, this->secretkey.c_str(), KEY_Z85_CSTR_LEN);
		zmq_setsockopt(subsock, ZMQ_CURVE_PUBLICKEY, this->publickey.c_str(), KEY_Z85_CSTR_LEN);
		zmq_setsockopt(subsock, ZMQ_CURVE_SERVERKEY, serverkey.c_str(), KEY_Z85_CSTR_LEN);
		for (const auto& keyval : this->ehyphae) {
			zmq_setsockopt(subsock, ZMQ_SUBSCRIBE, keyval.first.c_str(), KEY_Z85_CSTR_LEN - 1);
		}
		zmq_connect(subsock, connpoint.c_str());
		this->ecatals_from.insert({serverkey, subsock});
		return EW::Ok;
	} else {
		return EW::AlreadyPresent;
	}
}


EW Efunguz::del_ecatal_from(const string& that_publickey) {
	string serverkey = cut_pad_str(that_publickey, KEY_Z85_LEN);
	if (this->ecatals_from.count(serverkey) == 1) {
		zmq_close(this->ecatals_from.at(serverkey));
		this->ecatals_from.erase(serverkey);
		for (auto& keyval : this->ehyphae) {
			keyval.second.remove_connpoint_via_ecatal(serverkey);
		}
		return EW::Ok;
	} else {
		return EW::AlreadyAbsent;
	}
}


EW Efunguz::add_ecatal_to(const string& that_publickey, const string& connpoint) {
	string serverkey = cut_pad_str(that_publickey, KEY_Z85_LEN);
	if (this->ecatals_to.count(serverkey) == 0) {
		zsocket* pushsock = zmq_socket(this->context, ZMQ_PUSH);
		zmq_setsockopt(pushsock, ZMQ_LINGER, &DEF_LINGER, sizeof(DEF_LINGER));
		int one = 1;
		zmq_setsockopt(pushsock, ZMQ_CONFLATE, &one, sizeof(one));
		zmq_setsockopt(pushsock, ZMQ_CURVE_SECRETKEY, this->secretkey.c_str(), KEY_Z85_CSTR_LEN);
		zmq_setsockopt(pushsock, ZMQ_CURVE_PUBLICKEY, this->publickey.c_str(), KEY_Z85_CSTR_LEN);
		zmq_setsockopt(pushsock, ZMQ_CURVE_SERVERKEY, serverkey.c_str(), KEY_Z85_CSTR_LEN);
		zmq_connect(pushsock, connpoint.c_str());
		this->ecatals_to.insert({serverkey, pushsock});
		return EW::Ok;
	} else {
		return EW::AlreadyPresent;
	}
}


EW Efunguz::del_ecatal_to(const string& that_publickey) {
	string serverkey = cut_pad_str(that_publickey, KEY_Z85_LEN);
	if (this->ecatals_to.count(serverkey) == 1) {
		zmq_close(this->ecatals_to.at(serverkey));
		this->ecatals_to.erase(serverkey);
		return EW::Ok;
	} else {
		return EW::AlreadyAbsent;
	}
}


void Efunguz::emit_etale(const string& title, const vector<vector<uint8_t>>& parts) {
	vector<vector<uint8_t>> msg_parts{};

	vector<uint8_t> topic(title.size() + 1);
	title.copy((char *)topic.data(), title.size());
	msg_parts.push_back(topic);

	int64_t t_out = time_musec();
	vector<uint8_t> t_out_bs(8);
	memcpy(t_out_bs.data(), &t_out, 8);
	msg_parts.push_back(t_out_bs);

	for (const auto& part : parts) {
		msg_parts.push_back(part);
	}

	zmqp_send(this->pubsock, msg_parts);
}


void Efunguz::emit_beacon() {
	vector<vector<uint8_t>> msg_parts;
	vector<uint8_t> pubsub_port_bs(2);
	memcpy(pubsub_port_bs.data(), &(this->pubsub_port), 2);
	msg_parts.push_back(pubsub_port_bs);
	for (const auto& keyval : this->ecatals_to) {
		zmqp_send(keyval.second, msg_parts);
	}
}


void Efunguz::update() {
	while (zmqp_poll_in(this->zapsock, 0) > 0) {
		vector<vector<uint8_t>> request = get<0>(zmqp_recv(this->zapsock));
		vector<vector<uint8_t>> reply;

		auto& version = request[0];
		auto& sequence = request[1];
		auto& domain = request[2];
		auto& address = request[3];
		auto& identity = request[4];
		auto& mechanism = request[5];
		auto& key_32 = request[6];

		uint8_t key_cstr[KEY_Z85_CSTR_LEN]{0};
		zmq_z85_encode((char *)key_cstr, key_32.data(), KEY_BIN_LEN);
		string key((char *)key_cstr);

		reply.push_back(version);
		reply.push_back(sequence);

		if ((identity == cstr_to_vec_u8(ROUTING_ID_PUBSUB)) && (this->whitelist_publickeys.empty() || (this->whitelist_publickeys.count(key) == 1))) {
			// Auth passed; though needless (yet), set user-id to client's publickey
			reply.push_back(cstr_to_vec_u8("200"));
			reply.push_back(cstr_to_vec_u8("OK"));
			reply.push_back(cstr_to_vec_u8((const char *)key_cstr));
			reply.push_back(cstr_to_vec_u8(""));
		} else {
			// Auth failed
			reply.push_back(cstr_to_vec_u8("400"));
			reply.push_back(cstr_to_vec_u8("FAILED"));
			reply.push_back(cstr_to_vec_u8(""));
			reply.push_back(cstr_to_vec_u8(""));
		}

		zmqp_send(this->zapsock, reply);
	}

	int64_t t = time_musec();

	if ((t - this->t_last_beacon) >= this->beacon_interval) {
		this->emit_beacon();
		this->t_last_beacon = t;
	}

	for (const auto& keyval : this->ecatals_from) {
		string ec_key = keyval.first;
		zsocket* subsock = keyval.second;
		while (zmqp_poll_in(subsock, 0) > 0) {
			auto msg_parts = get<0>(zmqp_recv(subsock));
			// 0th must be serverkey, 1st must be connpoint
			if (msg_parts.size() == 2) {
				// Sanity checks...
				if (msg_parts[0].size() == KEY_Z85_LEN) { // fails if key has wrong length
					string that_publickey((char *)msg_parts[0].data(), KEY_Z85_LEN);
					if (this->ehyphae.count(that_publickey) == 1) { // fails if there is no ehypha with this key
						string connpoint((char *)msg_parts[1].data(), msg_parts[1].size());
						// connpoint must begin with "tcp://"
						if ((connpoint.size() >= strlen("tcp://")) && (connpoint.substr(0, strlen("tcp://")) == string("tcp://"))) { // TODO: more sanity checks for connpoint ("ip:port", IPv4/IPv6 etc.)
							this->ehyphae.at(that_publickey).update_connpoint_via_ecatal(ec_key, connpoint, t);
						}
					}
				}
			}
		}
	}

	for (auto& keyval : this->ehyphae) {
		keyval.second.update();
	}
}


Efunguz::~Efunguz() {
	this->ehyphae.clear(); // to close subsock of each ehypha in its destructor before terminating context, to which those sockets belong

	for (const auto& keyval : this->ecatals_to) {
		zmq_close(keyval.second);
	}
	for (const auto& keyval : this->ecatals_from) {
		zmq_close(keyval.second);
	}

	zmq_close(this->pubsock);
	zmq_close(this->zapsock);

	zmq_ctx_shutdown(this->context);
	zmq_ctx_term(this->context);
}


Ecataloguz::Ecataloguz(const string& secretkey, const map<string, string>& beacon_whitelist_publickeys_with_comments, const set<string>& pubsub_whitelist_publickeys, const uint16_t beacon_port, const uint16_t pubsub_port, const int64_t deactivate_interval, const int64_t publish_interval, const int64_t idle_interval)
: beacon_port {beacon_port}, pubsub_port {pubsub_port}, deactivate_interval {deactivate_interval}, publish_interval {publish_interval}, idle_interval {idle_interval} {
	this->secretkey = cut_pad_str(secretkey, KEY_Z85_LEN);

	uint8_t publickey_cstr[KEY_Z85_CSTR_LEN]{0};
	zmq_curve_public((char *)publickey_cstr, this->secretkey.c_str());
	this->publickey = cut_pad_str(string((char *)publickey_cstr), KEY_Z85_LEN);

	for (const auto& keyval : beacon_whitelist_publickeys_with_comments) {
		string dkey = cut_pad_str(keyval.first, KEY_Z85_LEN);
		this->beacon_whitelist_publickeys.insert(dkey);
		this->beacon_recs.insert({dkey, {"", -1, keyval.second}});
	}
	for (const auto& key : pubsub_whitelist_publickeys) {
		this->pubsub_whitelist_publickeys.insert(cut_pad_str(key, KEY_Z85_LEN));
	}

	this->context = zmq_ctx_new();
	zmq_ctx_set(this->context, ZMQ_IPV6, DEF_IPV6_STATUS);
	
	this->zapsock = zmq_socket(this->context, ZMQ_REP);
	zmq_setsockopt(this->zapsock, ZMQ_LINGER, &DEF_LINGER, sizeof(DEF_LINGER));
	zmq_bind(this->zapsock, "inproc://zeromq.zap.01");

	int one = 1;

	this->pullsock = zmq_socket(this->context, ZMQ_PULL);
	zmq_setsockopt(this->pullsock, ZMQ_LINGER, &DEF_LINGER, sizeof(DEF_LINGER));
	zmq_setsockopt(this->pullsock, ZMQ_CURVE_SERVER, &one, sizeof(one));
	zmq_setsockopt(this->pullsock, ZMQ_CURVE_SECRETKEY, this->secretkey.c_str(), KEY_Z85_CSTR_LEN);
	zmq_setsockopt(this->pullsock, ZMQ_ROUTING_ID, ROUTING_ID_BEACON, strlen(ROUTING_ID_BEACON));

	this->pubsock = zmq_socket(this->context, ZMQ_PUB);
	zmq_setsockopt(this->pubsock, ZMQ_LINGER, &DEF_LINGER, sizeof(DEF_LINGER));
	zmq_setsockopt(this->pubsock, ZMQ_CURVE_SERVER, &one, sizeof(one));
	zmq_setsockopt(this->pubsock, ZMQ_CURVE_SECRETKEY, this->secretkey.c_str(), KEY_Z85_CSTR_LEN);
	zmq_setsockopt(this->pubsock, ZMQ_ROUTING_ID, ROUTING_ID_PUBSUB, strlen(ROUTING_ID_PUBSUB));
}


EW Ecataloguz::read_beacon_whitelist_publickeys_with_comments(string filepath) {
	ifstream ifs(filepath, ios_base::in);
	char line_cstr_buf[MAX_PUBLICKEYS_FILE_LINE_LEN];
	while (ifs.getline(line_cstr_buf, MAX_PUBLICKEYS_FILE_LINE_LEN)) {
		string line(line_cstr_buf);
		if (line.size() >= KEY_Z85_LEN) {
			string key = line.substr(0, KEY_Z85_LEN);
			string comment = (line.size() >= (KEY_Z85_LEN + 2)) ? line.substr(KEY_Z85_LEN + 1) : ""; // " " or "\t" after key, then non-empty comment
			this->beacon_whitelist_publickeys.insert(key);
			this->beacon_recs.insert({key, {"", -1, comment}});
		}
	}
	return EW::Ok;
}


EW Ecataloguz::read_pubsub_whitelist_publickeys(string filepath) {
	ifstream ifs(filepath, ios_base::in);
	char line_cstr_buf[MAX_PUBLICKEYS_FILE_LINE_LEN];
	while (ifs.getline(line_cstr_buf, MAX_PUBLICKEYS_FILE_LINE_LEN)) {
		string line(line_cstr_buf);
		if (line.size() >= KEY_Z85_LEN) {
			string key = line.substr(0, KEY_Z85_LEN);
			this->pubsub_whitelist_publickeys.insert(key);
		}
	}
	return EW::Ok;
}


void Ecataloguz::run() {
	zmq_bind(this->pullsock, (string("tcp://*:") + to_string(this->beacon_port)).c_str());
	zmq_bind(this->pubsock, (string("tcp://*:") + to_string(this->pubsub_port)).c_str());

	int64_t t_last_pub = 0;


#ifdef EMYZELIUM_ECATAL_TUI

	bool show_active_now = true;
	bool show_comments = true;
	int i_page = 0;
	int page_size = 1;
	int64_t t_start = 0;

	// Start using ncurses...
	initscr();

	nodelay(stdscr, true);
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, false);
	scrollok(stdscr, false);
	keypad(stdscr, true);
	curs_set(0);
	if (can_change_color()) {
		init_color(0, 0, 0, 0);
	}

	page_size = max(1, LINES - 11);

	t_start = time_musec();

#endif // EMYZELIUM_ECATAL_TUI


	bool quit = false;

	while (!quit) {
		while (zmqp_poll_in(this->zapsock, 0) > 0) {
			vector<vector<uint8_t>> request = get<0>(zmqp_recv(this->zapsock));
			vector<vector<uint8_t>> reply;

			auto& version = request[0];
			auto& sequence = request[1];
			auto& domain = request[2];
			auto& address = request[3];
			auto& identity = request[4];
			auto& mechanism = request[5];
			auto& key_32 = request[6];

			uint8_t key_cstr[KEY_Z85_CSTR_LEN]{0};
			zmq_z85_encode((char *)key_cstr, key_32.data(), KEY_BIN_LEN);
			string key((char *)key_cstr);

			reply.push_back(version);
			reply.push_back(sequence);

			if ( ((identity == cstr_to_vec_u8(ROUTING_ID_BEACON)) && (this->beacon_whitelist_publickeys.empty() || (this->beacon_whitelist_publickeys.count(key) == 1)))
				|| ((identity == cstr_to_vec_u8(ROUTING_ID_PUBSUB)) && (this->pubsub_whitelist_publickeys.empty() || (this->pubsub_whitelist_publickeys.count(key) == 1))) ) {
				// Auth passed, set user-id to client's publickey
				reply.push_back(cstr_to_vec_u8("200"));
				reply.push_back(cstr_to_vec_u8("OK"));
				reply.push_back(cstr_to_vec_u8((const char *)key_cstr));
				reply.push_back(cstr_to_vec_u8(""));
			} else {
				// Auth failed
				reply.push_back(cstr_to_vec_u8("400"));
				reply.push_back(cstr_to_vec_u8("FAILED"));
				reply.push_back(cstr_to_vec_u8(""));
				reply.push_back(cstr_to_vec_u8(""));
			}

			zmqp_send(this->zapsock, reply);
		}

		int64_t t = time_musec();

		long idle_interval = this->idle_interval / 1000;
		while (zmqp_poll_in(this->pullsock, idle_interval) > 0) {
			idle_interval = 0;
			// Auth passed
			auto msg_with_props = zmqp_recv(this->pullsock);
			auto parts = get<0>(msg_with_props);
			auto props = get<1>(msg_with_props);
			if ((parts.size() == 1) && (parts[0].size() == 2)) {
				auto key = props.userId;
				auto ip = props.peerAddress;
				uint16_t port = *((uint16_t *)parts[0].data());
				string connpoint = string("tcp://") + ip + ":" + to_string(port);
				if (this->beacon_recs.count(key) == 1) {
					string comment = get<2>(this->beacon_recs.at(key));
					this->beacon_recs.erase(key);
					this->beacon_recs.insert({key, {connpoint, t, comment}});
				} else {
					this->beacon_recs.insert({key, {connpoint, t, ""}});
				}
			}
		}

		if (t - t_last_pub > this->publish_interval) {
			for (const auto& keyval : this->beacon_recs) {
				auto connpoint = get<0>(keyval.second);
				auto t_last_beac = get<1>(keyval.second);
				auto comment = get<2>(keyval.second);
				if ((this->deactivate_interval > 0) && (t - t_last_beac > this->deactivate_interval)) {
					connpoint = "";
					this->beacon_recs.at(keyval.first) = {connpoint, t_last_beac, comment};
				}
				if (!connpoint.empty()) {
					vector<vector<uint8_t>> msg_parts{};

					vector<uint8_t> key(KEY_Z85_LEN);
					keyval.first.copy((char *)key.data(), KEY_Z85_LEN);
					msg_parts.push_back(key);

					vector<uint8_t> connpoint_vec_u8(connpoint.size());
					connpoint.copy((char *)connpoint_vec_u8.data(), connpoint.size());
					msg_parts.push_back(connpoint_vec_u8);

					zmqp_send(this->pubsock, msg_parts);
				}
			}
			t_last_pub = t;
		}


#ifdef EMYZELIUM_ECATAL_TUI

		// Show TUI and process input
		erase();

		mvaddstrattr(0, 0, string("ecataloguz of Emyzelium v") + VERSION + " (" + DATE + ")\n", A_REVERSE | A_BOLD);
		string since_start_str_begin = "Since start: ";
		string since_start_str_end = to_string((t - t_start) / 1000000);
		mvaddstrattr(0, COLS - since_start_str_begin.size() - since_start_str_end.size(), since_start_str_begin);
		addstrattr(since_start_str_end, A_BOLD);
		mvaddstrattr(1, 0, "Public key (Z85) ");
		addstrattr(this->publickey, A_BOLD);
		mvaddstrattr(2, 0, "Ports: beacon ");
		addstrattr(to_str(this->beacon_port), A_BOLD);
		addstrattr(", pubsub ");
		addstrattr(to_str(this->pubsub_port), A_BOLD);
		mvaddstrattr(3, 0, "Intervals: deactivate ");
		addstrattr(to_str(1e-6 * this->deactivate_interval, 1), A_BOLD);
		addstrattr(", publish ");
		addstrattr(to_str(1e-6 * this->publish_interval, 1), A_BOLD);
		addstrattr(", sleep ");
		addstrattr(to_str(1e-6 * this->idle_interval, 1), A_BOLD);
		// Line 4 will be filled later
		mvaddstrattr(5, 0, string(112, '-'));
		mvaddstrattr(5, 2, (show_active_now ? string("[ Active now") : string("[ Active once")) + ": page " + to_string(i_page + 1) + " ]");
		mvaddstrattr(6, 0, show_comments ? "COMMENT" : "PUBLIC KEY");
		mvaddstrattr(6, 44, "CONNPOINT");
		mvaddstrattr(6, 92, "SINCE LAST BEACON");

		int n_active_now = 0;
		int n_active_once = 0;
		for (const auto& keyval : this->beacon_recs) {
			auto t_last_beac = get<1>(keyval.second);
			if (t_last_beac >= 0) {
				n_active_once += 1;
				auto connpoint = get<0>(keyval.second);
				auto comment = get<2>(keyval.second);
				if (!connpoint.empty()) {
					n_active_now++;
				}
				int j = show_active_now ? n_active_now : n_active_once;
				j -= i_page * page_size;
				if ((j > 0) && (j <= page_size) && ((!show_active_now) || (!connpoint.empty()))) {
					mvaddstrattr(6 + j, 0, show_comments ? comment.substr(0, KEY_Z85_LEN) : keyval.first, A_BOLD);
					mvaddstrattr(6 + j, 44, connpoint.substr(0, 48), A_BOLD);
					mvaddstrattr(6 + j, 92, to_str(1e-6 * (t - t_last_beac), 1).substr(0, 16), A_BOLD);
				}
			}
		}

		mvaddstrattr(7 + page_size, 0, string(112, '-'));
		mvaddstrattr(8 + page_size, 0, "Pages: [PageUp] previous, [PageDown] next, [Home] 1st, [End] last");
		
		mvaddstrattr(4, 0, "Efungi: beacon - whitelisted ");
		addstrattr(to_string(this->beacon_whitelist_publickeys.size()), A_BOLD);
		addstrattr(", active once ");
		addstrattr(to_string(n_active_once), A_BOLD);
		addstrattr(", active now ");
		addstrattr(to_string(n_active_now), A_BOLD);
		addstrattr("; pubsub - whitelisted ");
		addstrattr(to_string(this->pubsub_whitelist_publickeys.size()), A_BOLD);

		mvaddstrattr(LINES - 1, 0, "[Q] quit, [A] show active now/once, [C] show comments/keys");

		refresh();

		int ch = getch();
		switch (ch) {
			case 'q': case 'Q':
				quit = true;
				break;
			case 'a': case 'A':
				show_active_now = !show_active_now;
				break;
			case 'c': case 'C':
				show_comments = !show_comments;
				break;
			case KEY_PPAGE:
				if (i_page > 0) {
					i_page--;
				}
				break;
			case KEY_NPAGE:
				i_page++;
				break;
			case KEY_HOME:
				i_page = 0;
				break;
			case KEY_END:
				i_page = max(0, ((show_active_now ? n_active_now : n_active_once) - 1) / page_size);
				break;
		}

#endif // EMYZELIUM_ECATAL_TUI


	}


#ifdef EMYZELIUM_ECATAL_TUI
		
	curs_set(1);
	keypad(stdscr, false);
	scrollok(stdscr, true);
	intrflush(stdscr, true);
	nl();
	echo();
	nocbreak();
	nodelay(stdscr, false);
	
	endwin();

#endif // EMYZELIUM_ECATAL_TUI


}


Ecataloguz::~Ecataloguz() {
	zmq_close(this->pubsock);
	zmq_close(this->pullsock);
	zmq_close(this->zapsock);

	zmq_ctx_shutdown(this->context);
	zmq_ctx_term(this->context);
}

}
