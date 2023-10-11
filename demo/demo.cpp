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
 * Demo
 */

#include "../emyzelium.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <ncurses.h>
#include <random>
#include <set>
#include <sstream>
#include <thread>


// Of course, person_SECRETKEY should be known only to that person
// Here they are "revealed" at once for demo purpose

const string ALIEN_SECRETKEY = "gr6Y.04i(&Y27ju0g7m0HvhG0:rDmx<Y[FvH@*N(";
const string ALIEN_PUBLICKEY = "iGxlt)JYh!P9xPCY%BlY4Y]c^<=W)k^$T7GirF[R";
const string ALIEN_ONION = "PLACEHOLDER PLACEHOLDER PLACEHOLDER PLACEHOLDER PLACEHOL"; // from service_dir/hostname, without .onion
const uint16_t ALIEN_PORT = 60847;

const string JOHN_SECRETKEY = "gbMF0ZKztI28i6}ax!&Yw/US<CCA9PLs.Osr3APc";
const string JOHN_PUBLICKEY = "(>?aRHs!hJ2ykb?B}t6iGgo3-5xooFh@9F/4C:DW";
const string JOHN_ONION = "PLACEHOLDER PLACEHOLDER PLACEHOLDER PLACEHOLDER PLACEHOL"; // from service_dir/hostname, without .onion
const uint16_t JOHN_PORT = 60848;

const string MARY_SECRETKEY = "7C*zh5+-8jOI[+^sh[dbVnW{}L!A&7*=j/a*h5!Y";
const string MARY_PUBLICKEY = "WR)%3-d9dw)%3VQ@O37dVe<09FuNzI{vh}Vfi+]0";
const string MARY_ONION = "PLACEHOLDER PLACEHOLDER PLACEHOLDER PLACEHOLDER PLACEHOL"; // from service_dir/hostname, without .onion
const uint16_t MARY_PORT = 60849;


int64_t time_musec() {
	return chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
}


vector<uint8_t> str_to_vec_u8(const string& s) {
	vector<uint8_t> v(s.size());
	memcpy(v.data(), s.data(), s.size());
	return v;
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


string set_to_str(const set<int>& s) {
	string str = "{";
	bool first = true;
	for (const auto& el : s) {
		if (!first) {
			str += ",";
		} else {
			first = false;
		}
		str += to_string(el);
	}
	str += "}";
	return str;
}


void init_term_graphics() {
	setlocale(LC_ALL, "");

	initscr();

	start_color();

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

	if (has_colors()) {
		for (int bc = 0; bc < 8; bc++) {
			for (int fc = 0; fc < 8; fc++) {
				if ((bc > 0) || (fc > 0)) {
					init_pair((bc << 3) + fc, fc, bc);
				}
			}
		}
	}
}


void drop_term_graphics() {
	// reset_color_pairs();
	curs_set(1);
	keypad(stdscr, false);
	scrollok(stdscr, true);
	intrflush(stdscr, true);
	nl();
	echo();
	nocbreak();
	nodelay(stdscr, false);
	
	endwin();
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


void print_rect(int y, int x, int h, int w, int attr=0) {
	mvaddstrattr(y, x, "┌", attr);
	mvaddstrattr(y, x + w - 1, "┐", attr);
	mvaddstrattr(y + h - 1, x + w - 1, "┘", attr);
	mvaddstrattr(y + h - 1, x, "└", attr);
	for (int i = 1; i < h - 1; i++) {
		mvaddstrattr(y + i, x, "│", attr);
		mvaddstrattr(y + i, x + w - 1, "│", attr);
	}
	for (int j = 1; j < w - 1; j++) {
		mvaddstrattr(y, x + j, "─", attr);
		mvaddstrattr(y + h - 1, x + j, "─", attr);
	}
}


struct Other {
	string name;
	string publickey;
};


class Realm_CA {
	string name;
	Emyzelium::Efunguz* efunguz;
	int height;
	int width;
	vector<vector<uint8_t>> cells; // bits: 0 - current state, 1-4 - number of alive neighbours
	set<int> birth;
	set<int> survival;
	double autoemit_interval;
	int framerate;
	vector<Other> others;
	int i_turn;
	int cursor_y;
	int cursor_x;

public:
	Realm_CA(const string& name, const string& secretkey, const unordered_set<string>& whitelist_publickeys, const uint16_t pubport, const int height, const int width, const set<int>& birth, const set<int>& survival, const double autoemit_interval=4.0, const int framerate=30)
	: name {name}, width {width}, birth {birth}, survival {survival}, autoemit_interval {autoemit_interval}, framerate {framerate} {
		this->efunguz = new Emyzelium::Efunguz(secretkey, whitelist_publickeys, pubport);
		this->height = (height >> 1) << 1;

		for (int y = 0; y < this->height; y++) {
			this->cells.push_back(vector<uint8_t>(this->width));
		}

		this->i_turn = 0;

		this->cursor_y = this->height >> 1;
		this->cursor_x = this->width >> 1;
	}


	void add_other(const string& name, const string& publickey, const string& onion, const uint16_t port) {
		auto& ehypha = get<0>(this->efunguz->add_ehypha(publickey, onion, port));
		ehypha.add_etale("");
		ehypha.add_etale("zone");
		this->others.push_back(Other{name, publickey});
	}


	void add_whitelist_publickeys(const unordered_set<string>& publickeys) {
		this->efunguz->add_whitelist_publickeys(publickeys);
	}


	void flip(const int y=-1, const int x=-1) {
		int fy = (y < 0) ? this->cursor_y : y;
		int fx = (x < 0) ? this->cursor_x : x;
		this->cells[fy][fx] ^= 1;
	}


	void clear() {
		for (int y = 0; y < this->height; y++) {
			for (int x = 0; x < this->width; x++) {
				this->cells[y][x] = 0;
			}
		}
		this->i_turn = 0;
	}


	void reset() {
		mt19937_64 mt_engine(time_musec());
		for (int y = 0; y < this->height; y++) {
			for (int x = 0; x < this->width; x++) {
				this->cells[y][x] = mt_engine() & 1;
			}
		}
		this->i_turn = 0;
	}


	void render(const bool show_cursor=false) {
		int h = this->height;
		int w = this->width;
		int w_tert = w / 3;

		print_rect(0, 0, (h >> 1) + 2, w + 2); // grey on black
		mvaddstrattr(0, w_tert, "┬┬");
		mvaddstrattr(0, w - w_tert, "┬┬");
		mvaddstrattr(1 + (h >> 1), w_tert, "┴┴");
		mvaddstrattr(1 + (h >> 1), w - w_tert, "┴┴");
		mvaddstrattr(0, 2, "[ From others ]");
		mvaddstrattr(0, 3 + w - w_tert, "[ To others ]");
		
		vector<vector<string>> cell_chars = {{" ", "▀"}, {"▄", "█"}};

		for (int i = 0; i < (h >> 1); i++) {
			int y = i << 1;
			string row_str = "";
			for (int x = 0; x < w; x++) {
				row_str += cell_chars[this->cells[y + 1][x] & 1][this->cells[y][x] & 1];				
			}
			mvaddstrattr(1 + i, 1, row_str, COLOR_PAIR(7) | A_BOLD); // white on black
		}

		string status_str = string("[ T = ") + to_string(this->i_turn);

		if (show_cursor) {
			int i = this->cursor_y >> 1;
			int m = this->cursor_y & 1;
			int cell_high = this->cells[i << 1][this->cursor_x] & 1;
			int cell_low = this->cells[(i << 1) + 1][this->cursor_x] & 1;

			vector<vector<vector<string>>> chars = {{{"▀",      "▄"},      {"▀",      "▀"}},      {{"▄",      "▄"},      {"▄",      "▀"}}};
			vector<vector<vector<int>>>    clrps = {{{1|(0<<3), 1|(0<<3)}, {3|(0<<3), 7|(1<<3)}}, {{7|(1<<3), 3|(0<<3)}, {7|(3<<3), 7|(3<<3)}}};
			vector<vector<vector<int>>>    bolds = {{{0,        0},        {0,        1}},        {{1,        0},        {1,         1}}};
			// Exercise: compactify

			string s_char = chars[cell_low][cell_high][m];
			int s_clrp = clrps[cell_low][cell_high][m];
			int s_bold = bolds[cell_low][cell_high][m];

			mvaddstrattr(1 + i, 1 + this->cursor_x, s_char, COLOR_PAIR(s_clrp) | (s_bold * A_BOLD));

			status_str += ", X = " + to_string(this->cursor_x) + ", Y = " + to_string(this->cursor_y) + ", C = " + to_string(this->cells[this->cursor_y][this->cursor_x] & 1);
		}

		status_str += " ]";
		mvaddstrattr(1 + (h >> 1), 1 + ((w - status_str.size()) >> 1), status_str);
	}


	void move_cursor(int dy, int dx) {
		this->cursor_y = max(0, min(this->height - 1, this->cursor_y + dy));
		this->cursor_x = max(0, min(this->width - 1, this->cursor_x + dx));
	}


	void turn() {
		// Not much optimization...
		int h = this->height;
		int w = this->width;
		// Count alive neighbours
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				if (this->cells[y][x] & 1) { // increment number of neighbours for all neighbouring cells
					for (int ny = y - 1; ny <= y + 1; ny++) {
						if ((ny >= 0) && (ny < h)) {
							for (int nx = x - 1; nx <= x + 1; nx++) {
								if (((ny != y) || (nx != x)) && (nx >= 0) && (nx < w)) {
									this->cells[ny][nx] += 2;
								}
							}
						}
					}
				}
			}
		}
		// Update
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				uint8_t c = this->cells[y][x];
				if (c & 1) {
					c = this->survival.count(c >> 1);
				} else {
					c = this->birth.count(c >> 1);
				}
				this->cells[y][x] = c;
			}
		}
		this->i_turn++;
	}


	vector<vector<uint8_t>> get_etale_from_zone() {
		vector<vector<uint8_t>> parts;
		int h = this->height;
		int w = this->width;
		int zh = h;
		int zw = w / 3;

		parts.emplace_back(2);
		memcpy(parts[0].data(), &zh, 2);

		parts.emplace_back(2);
		memcpy(parts[1].data(), &zw, 2);

		parts.emplace_back(zh * zw);
		for (int y = 0; y < zh; y++) {
			for (int x = 0; x < zw; x++) {
				parts[2][y * zw + x] = this->cells[y][w - zw + x] & 1; // could compress to bits...
			}
		}

		return parts;
	}


	void put_etale_to_zone(const vector<vector<uint8_t>>& parts) {
		if (parts.size() == 3) {
			if ((parts[0].size() == 2) && (parts[1].size() == 2)) {
				int szh = *((uint16_t *)parts[0].data());
				int szw = *((uint16_t *)parts[1].data());
				if (parts[2].size() == szh * szw) {
					int dzh = min(szh, this->height);
					int dzw = min(szw, this->width / 3);
					for (int y = 0; y < dzh; y++) {
						for (int x = 0; x < dzw; x++) {
							this->cells[y][x] = parts[2][y * szw + x] & 1;
						}
					}
				}
			}
		}
	}


	void emit_etales() {
		this->efunguz->emit_etale("", {str_to_vec_u8("zone"), str_to_vec_u8("2B height (h), 2B width (w), h×wB zone by rows")});
		this->efunguz->emit_etale("zone", this->get_etale_from_zone());
	}


	void update_efunguz() {
		this->efunguz->update();
	}


	void run() {
		int h = this->height;
		int w = this->width;

		bool quit = false;
		bool paused = false;
		bool render = true;
		bool autoemit = true;

		int64_t t_start = time_musec();

		double t_last_render = -65536.0;
		double t_last_emit = -65536.0;

		while (!quit) {
			double t = 1e-6 * (time_musec() - t_start);

			if (t - t_last_render > 1.0 / this->framerate) {
				erase();

				if (render) {
					this->render(paused);
				} else {
					mvaddstrattr(0, 0, "Render OFF");
				}
				mvaddstrattr((h >> 1) + 2, 0, string("This realm: \"") + this->name + "'s\" (birth " + set_to_str(this->birth) + ", survival " + set_to_str(this->survival) + "), SLE " + to_str(t - t_last_emit, 1) + ", autoemit (" + to_str(this->autoemit_interval, 1) + ") " + (autoemit ? "ON" : "OFF"));
				mvaddstrattr((h >> 1) + 3, 0, "Other realms: ");
				for (int i_other = 0; i_other < this->others.size(); i_other++) {
					const auto& that = this->others[i_other];
					addstrattr((i_other > 0 ? string(", ") : string("")) + "[" + to_string(i_other + 1) + "] \"" + that.name + "'s\" (SLU " + to_str(t - 1e-6 * (get<0>(get<0>(this->efunguz->get_ehypha_ptr(that.publickey))->get_etale_ptr("zone"))->t_in - t_start), 1) + ")");
				}
				mvaddstrattr(LINES - 3, 0, "[Q] quit, [C] clear, [R] reset, [V] render on/off, [P] pause/resume");
				mvaddstrattr(LINES - 2, 0, "[A] autoemit on/off, [E] emit, [1-9] import");
				mvaddstrattr(LINES - 1, 0, "If paused: [T] turn, [→ ↑ ← ↓] move cursor, [ ] flip cell");

				refresh();

				t_last_render = t;
			}

			if (autoemit && ((t - t_last_emit > this->autoemit_interval))) {
				this->emit_etales();
				t_last_emit = t;
			}

			this->update_efunguz();

			if (!paused) {
				this->turn();
			}

			int ch = getch();

			switch (ch) {
				case 'q': case 'Q':
					quit = true;
					break;
				case 'c': case 'C':
					this->clear();
					break;
				case 'r': case 'R':
					this->reset();
					break;
				case 'v': case 'V':
					render = !render;
					break;
				case 'p': case 'P':
					paused = !paused;
					break;
				case 'a': case 'A':
					autoemit = !autoemit;
					break;
				case 'e': case 'E':
					this->emit_etales();
					t_last_emit = t;
					break;
				case '1'...'9':
					int i_other = ch - '1';
					if (i_other < this->others.size()) {
						const auto* that_zone = get<0>(get<0>(this->efunguz->get_ehypha_ptr(this->others[i_other].publickey))->get_etale_ptr("zone"));
						this->put_etale_to_zone(that_zone->parts);
					}
					break;
			}
			if (paused) {
				switch (ch) {
					case 't': case 'T':
						this->turn();
						break;
					case ' ':
						this->flip();
						break;
					case KEY_RIGHT:
						this->move_cursor(0, 1);
						break;
					case KEY_UP:
						this->move_cursor(-1, 0);
						break;
					case KEY_LEFT:
						this->move_cursor(0, -1);
						break;
					case KEY_DOWN:
						this->move_cursor(1, 0);
						break;
				}				
			}
		}
	}


	~Realm_CA() {
		delete this->efunguz;
	}
};


int run_realm(string name) {
	string name_up = name;
	transform(name_up.begin(), name_up.end(), name_up.begin(), ::toupper);

	string secretkey("");
	uint16_t pubport = 0;
	string that1_name("");
	string that1_publickey("");
	string that1_onion("");
	uint16_t that1_port = 0;
	string that2_name("");
	string that2_publickey("");
	string that2_onion("");
	uint16_t that2_port = 0;
	set<int> birth{};
	set<int> survival{};

	if (name_up == "ALIEN") {
		secretkey = ALIEN_SECRETKEY;
		pubport = ALIEN_PORT;
		that1_name = "John";
		that1_publickey = JOHN_PUBLICKEY;
		that1_onion = JOHN_ONION;
		that1_port = JOHN_PORT;
		that2_name = "Mary";
		that2_publickey = MARY_PUBLICKEY;
		that2_onion = MARY_ONION;
		that2_port = MARY_PORT;
		birth = {3, 4};
		survival = {3, 4}; // 3-4 Life
	} else if (name_up == "JOHN") {
		secretkey = JOHN_SECRETKEY;
		pubport = JOHN_PORT;
		that1_name = "Alien";
		that1_publickey = ALIEN_PUBLICKEY;
		that1_onion = ALIEN_ONION;
		that1_port = ALIEN_PORT;
		that2_name = "Mary";
		that2_publickey = MARY_PUBLICKEY;
		that2_onion = MARY_ONION;
		that2_port = MARY_PORT;
		birth = {3};
		survival = {2, 3}; // classic Conway's Life
	} else if (name_up == "MARY") {
		secretkey = MARY_SECRETKEY;
		pubport = MARY_PORT;
		that1_name = "Alien";
		that1_publickey = ALIEN_PUBLICKEY;
		that1_onion = ALIEN_ONION;
		that1_port = ALIEN_PORT;
		that2_name = "John";
		that2_publickey = JOHN_PUBLICKEY;
		that2_onion = JOHN_ONION;
		that2_port = JOHN_PORT;
		birth = {3};
		survival = {2, 3}; // classic Conway's Life
	} else {
		printf("Unknown realm name: \"%s\". Must be \"Alien\", \"John\", or \"Mary\".\n", name.c_str());
		return (-1);
	}

	init_term_graphics();

	int height = (LINES - 8) << 1; // even
	int width = COLS - 2;

	Realm_CA realm(name, secretkey, unordered_set<string>{}, pubport, height, width, birth, survival);

	// Uncomment to restrict: Alien gets data from John and Mary; John gets data from Alien but not from Mary; Mary gets data from neither Alien, nor John
	// realm.add_whitelist_publickeys({that1_publickey});

	realm.add_other(that1_name, that1_publickey, that1_onion, that1_port);
	realm.add_other(that2_name, that2_publickey, that2_onion, that2_port);

	realm.reset();

	realm.run();

	drop_term_graphics();

	return 0;
}


int main(int argc, char** argv) {
	if (argc < 2) {
		printf("Syntax:\n");
		printf("demo <Alien|John|Mary>\n");
		return (-1);
	}

	vector<string> args{};
	for (int i = 0; i < argc; i++) {
		args.emplace_back(argv[i]);
	}

	return run_realm(args[1]);
}
