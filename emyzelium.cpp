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
 * Library
 */

#include "emyzelium.hpp"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <random>


using namespace std;


namespace Emyzelium {

const size_t KEY_BIN_LEN = 32;
const size_t KEY_Z85_LEN = 40;
const size_t KEY_Z85_CSTR_LEN = KEY_Z85_LEN + 1;

const char* CURVE_MECHANISM_ID = "CURVE"; // See https://rfc.zeromq.org/spec/27/
const char* ZAP_DOMAIN = "emyz";

const size_t ZAP_SESSION_ID_LEN = 32;

const int DEF_IPV6_STATUS = 1;

const size_t MAX_PUBLICKEYS_FILE_LINE_LEN = 96;


int64_t time_musec() {
	return chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
}


string cut_pad_key_str(const string& s) {
	return s.size() < KEY_Z85_LEN ? (s + string(KEY_Z85_LEN - s.size(), ' ')) : s.substr(0, KEY_Z85_LEN);
}


vector<uint8_t> cstr_to_vec_u8(const char *s) {
	size_t l = strlen(s);
	vector<uint8_t> bs(l);
	memcpy(bs.data(), s, l);
	return bs;
}


int zmqe_setsockopt(zsocket* socket, int option_name, int option_int) {
	return zmq_setsockopt(socket, option_name, &option_int, sizeof(int));
}

int zmqe_setsockopt(zsocket* socket, int option_name, const char* option_cstr) {
	return zmq_setsockopt(socket, option_name, option_cstr, strlen(option_cstr) + 1); // +1 for terminating zero byte
}


void zmqe_send(zsocket* socket, const vector<vector<uint8_t>>& parts) {
	zmq_msg_t msg;
	for (size_t i = 0; i < parts.size(); i++) {
		size_t size = parts[i].size();
		zmq_msg_init_size(&msg, size);
		memcpy(zmq_msg_data(&msg), parts[i].data(), size);
		if (zmq_msg_send(&msg, socket, (i + 1) < parts.size() ? ZMQ_SNDMORE : 0) < 0) {
			zmq_msg_close(&msg);
		}
	}
}


vector<vector<uint8_t>> zmqe_recv(zsocket* socket) {
	vector<vector<uint8_t>> parts{};
	zmq_msg_t msg;
	int more = 0;
	do {
		zmq_msg_init(&msg);
		zmq_msg_recv(&msg, socket, 0);
		size_t size = zmq_msg_size(&msg);
		vector<uint8_t> part(size);
		memcpy(part.data(), zmq_msg_data(&msg), size);
		// parts.push_back(move(part));
		parts.emplace_back(move(part));
		more = zmq_msg_get(&msg, ZMQ_MORE);
		zmq_msg_close(&msg);
	} while (more);
	return parts;
}

int zmqe_poll_in_now(zsocket* socket) {
	zmq_pollitem_t zpi;
	zpi.socket = socket;
	zpi.fd = 0;
	zpi.events = ZMQ_POLLIN;
	zpi.revents = 0;
	return zmq_poll(&zpi, 1, 0);
}

Etale::Etale(const vector<vector<uint8_t>>& parts, const int64_t t_out, const int64_t t_in, const bool paused)
: parts {parts}, t_out {t_out}, t_in {t_in}, paused {paused} {
}


Ehypha::Ehypha(zcontext* context, const string& secretkey, const string& publickey, const string& serverkey, const string& onion, const uint16_t pubsub_port, const uint16_t torproxy_port, const string& torproxy_host) {
	this->subsock = zmq_socket(context, ZMQ_SUB);
	zmqe_setsockopt(this->subsock, ZMQ_CURVE_SECRETKEY, secretkey.c_str());
	zmqe_setsockopt(this->subsock, ZMQ_CURVE_PUBLICKEY, publickey.c_str());
	zmqe_setsockopt(this->subsock, ZMQ_CURVE_SERVERKEY, serverkey.c_str());
	zmqe_setsockopt(this->subsock, ZMQ_SOCKS_PROXY, (torproxy_host + ":" + to_string(torproxy_port)).c_str());
	zmq_connect(this->subsock, ("tcp://" + onion + ".onion:" + to_string(pubsub_port)).c_str());
}


tuple<const Etale&, EW> Ehypha::add_etale(const string& title) {
	if (this->etales.count(title) == 0) {
		zmqe_setsockopt(this->subsock, ZMQ_SUBSCRIBE, title.c_str());
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


tuple<const Etale*, EW> Ehypha::get_etale_ptr(const string& title) {
	if (this->etales.count(title) == 1) {
		return tuple<const Etale*, EW>{&(this->etales.at(title)), EW::Ok};
	} else {
		return tuple<const Etale*, EW>{nullptr, EW::Absent};
	}
}


EW Ehypha::del_etale(const string& title) {
	if (this->etales.count(title) == 1) {
		this->etales.erase(title);
		zmqe_setsockopt(this->subsock, ZMQ_UNSUBSCRIBE, title.c_str());
		return EW::Ok;
	} else {
		return EW::AlreadyAbsent;
	}
}


EW Ehypha::pause_etale(const string& title) {
	if (this->etales.count(title) == 1) {
		Etale& etale = this->etales.at(title);
		if (!etale.paused) {
			zmqe_setsockopt(this->subsock, ZMQ_UNSUBSCRIBE, title.c_str());
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
			zmqe_setsockopt(this->subsock, ZMQ_SUBSCRIBE, title.c_str());
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

	while (zmqe_poll_in_now(this->subsock) > 0) {
		auto msg_parts = zmqe_recv(this->subsock);
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


Efunguz::Efunguz(const string& secretkey, const unordered_set<string>& whitelist_publickeys, const uint16_t pubsub_port, const uint16_t torproxy_port, const string& torproxy_host)
: pubsub_port {pubsub_port}, torproxy_port {torproxy_port}, torproxy_host {torproxy_host} {
	this->secretkey = cut_pad_key_str(secretkey);

	char publickey_cstr[KEY_Z85_CSTR_LEN]{0};
	zmq_curve_public(publickey_cstr, this->secretkey.c_str());
	this->publickey = cut_pad_key_str(string(publickey_cstr));

	for (const auto& key : whitelist_publickeys) {
		this->whitelist_publickeys.insert(cut_pad_key_str(key));
	}

	this->context = zmq_ctx_new();
	zmq_ctx_set(this->context, ZMQ_IPV6, DEF_IPV6_STATUS);
	zmq_ctx_set(this->context, ZMQ_BLOCKY, 0);

	// At first, REP socket for ZAP auth...
	this->zapsock = zmq_socket(this->context, ZMQ_REP);
	zmq_bind(this->zapsock, "inproc://zeromq.zap.01");

	random_device randev;
	this->zap_session_id.assign(ZAP_SESSION_ID_LEN, 0);
	for (size_t i = 0; i < ZAP_SESSION_ID_LEN; i++) {
		this->zap_session_id[i] = randev() & 0xFF; // must be cryptographically random... is it?
	}

	// ..and only then, PUB socket
	this->pubsock = zmq_socket(this->context, ZMQ_PUB);
	zmqe_setsockopt(this->pubsock, ZMQ_CURVE_SERVER, 1);
	zmqe_setsockopt(this->pubsock, ZMQ_CURVE_SECRETKEY, this->secretkey.c_str());
	zmq_setsockopt(this->pubsock, ZMQ_ZAP_DOMAIN, ZAP_DOMAIN, strlen(ZAP_DOMAIN)); // to enable auth, must be non-empty due to ZMQ RFC 27
	zmq_setsockopt(this->pubsock, ZMQ_ROUTING_ID, this->zap_session_id.data(), ZAP_SESSION_ID_LEN); // to make sure only this pubsock can pass auth through zapsock; see update()
	zmq_bind(this->pubsock, (string("tcp://*:") + to_string(this->pubsub_port)).c_str());
}


void Efunguz::add_whitelist_publickeys(const unordered_set<string>& publickeys) {
	for (const auto& key : publickeys) {
		this->whitelist_publickeys.insert(cut_pad_key_str(key));
	}
}


void Efunguz::del_whitelist_publickeys(const unordered_set<string>& publickeys) {
	for (const auto& key : publickeys) {
		this->whitelist_publickeys.erase(cut_pad_key_str(key));
	}
}


void Efunguz::clear_whitelist_publickeys() {
	this->whitelist_publickeys.clear();
}


void Efunguz::read_whitelist_publickeys(const string& filepath) {
	ifstream ifs(filepath, ios_base::in);
	char line_cstr_buf[MAX_PUBLICKEYS_FILE_LINE_LEN];
	while (ifs.getline(line_cstr_buf, MAX_PUBLICKEYS_FILE_LINE_LEN)) {
		string line(line_cstr_buf);
		if (line.size() >= KEY_Z85_LEN) {
			string key = line.substr(0, KEY_Z85_LEN);
			this->whitelist_publickeys.insert(key);
		}
	}
}


tuple<Ehypha&, EW> Efunguz::add_ehypha(const string& that_publickey, const string& onion, const uint16_t pubsub_port) {
	string serverkey = cut_pad_key_str(that_publickey);
	if (this->ehyphae.count(serverkey) == 0) {
		// insert() would destroy temporary Ehypha, whose destructor would close its subsock, making copied subsock ptr useless
		// (see the pair constructor in stl_pair.h that uses piecewise_construct_t)
		this->ehyphae.emplace(piecewise_construct,
			tuple<string>{serverkey},
			tuple<zcontext*, string, string, string, string, uint16_t, uint16_t, string>{this->context, this->secretkey, this->publickey, serverkey, onion, pubsub_port, this->torproxy_port, this->torproxy_host}
		);
		return tuple<Ehypha&, EW>{this->ehyphae.at(serverkey), EW::Ok};
	} else {
		return tuple<Ehypha&, EW>{this->ehyphae.at(serverkey), EW::AlreadyPresent};
	}
}


tuple<Ehypha*, EW> Efunguz::get_ehypha_ptr(const string& that_publickey) {
	string serverkey = cut_pad_key_str(that_publickey);
	if (this->ehyphae.count(serverkey) == 1) {
		return tuple<Ehypha*, EW>{&(this->ehyphae.at(serverkey)), EW::Ok};
	} else {
		return tuple<Ehypha*, EW>{nullptr, EW::Absent};
	}
}


EW Efunguz::del_ehypha(const string& that_publickey) {
	string serverkey = cut_pad_key_str(that_publickey);
	if (this->ehyphae.count(serverkey) == 1) {
		this->ehyphae.erase(serverkey);
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

	zmqe_send(this->pubsock, msg_parts);
}


void Efunguz::update() {
	while (zmqe_poll_in_now(this->zapsock) > 0) {
		vector<vector<uint8_t>> request = zmqe_recv(this->zapsock);
		vector<vector<uint8_t>> reply;

		auto& version = request[0];
		auto& sequence = request[1];
		// auto& domain = request[2];
		// auto& address = request[3];
		auto& identity = request[4];
		auto& mechanism = request[5];
		auto& key_u8 = request[6];

		char key_cstr[KEY_Z85_CSTR_LEN]{0};
		zmq_z85_encode(key_cstr, key_u8.data(), KEY_BIN_LEN);
		string key(key_cstr);

		reply.push_back(version);
		reply.push_back(sequence);

		if ((identity == this->zap_session_id) && (mechanism == cstr_to_vec_u8(CURVE_MECHANISM_ID)) && (this->whitelist_publickeys.empty() || (this->whitelist_publickeys.count(key) == 1))) {
			// Auth passed; though needless (yet), set user-id to client's publickey
			reply.push_back(cstr_to_vec_u8("200"));
			reply.push_back(cstr_to_vec_u8("OK"));
			reply.push_back(cstr_to_vec_u8(key_cstr));
			reply.push_back(cstr_to_vec_u8(""));
		} else {
			// Auth failed
			reply.push_back(cstr_to_vec_u8("400"));
			reply.push_back(cstr_to_vec_u8("FAILED"));
			reply.push_back(cstr_to_vec_u8(""));
			reply.push_back(cstr_to_vec_u8(""));
		}

		zmqe_send(this->zapsock, reply);
	}

	for (auto& keyval : this->ehyphae) {
		keyval.second.update();
	}
}


Efunguz::~Efunguz() {
	this->ehyphae.clear(); // to close subsock of each ehypha in its destructor before terminating context, to which those sockets belong

	zmq_close(this->pubsock);
	zmq_close(this->zapsock);

	zmq_ctx_shutdown(this->context);
	while (zmq_ctx_term(this->context) == -1) {
		if (zmq_errno() == EINTR) {
			continue;
		} else { // EFAULT or what?
			break;
		}

	}
}


}
