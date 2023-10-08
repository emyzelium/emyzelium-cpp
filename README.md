# ![logo disk](https://github.com/emyzelium/visuals/blob/main/logo_disk_32.png) Emyzelium (C++)

is another wrapper around [ZeroMQ](https://zeromq.org/)'s [Publish-Subscribe](https://zeromq.org/socket-api/#publish-subscribe-pattern) messaging pattern with mandatory [Curve](https://rfc.zeromq.org/spec/26/) security and optional [ZAP](https://rfc.zeromq.org/spec/27/) authentication filter, over [Tor](https://torproject.org), through Tor SOCKS5 proxy, for distributed artificial elife, decision making etc. systems where each peer, identified by its onion address, port, and public key, provides and updates vectors of vectors of bytes under unique topics that other peers can subscribe to and receive.

Requires [C++11 compiler support](https://en.cppreference.com/w/cpp/compiler_support#References) and, of course, [libzmq](https://github.com/zeromq/libzmq) ([more on build](http://wiki.zeromq.org/build:_start)). Demo requires [ncursesw](https://packages.ubuntu.com/bionic/libncursesw5-dev).

Versions in other languages: [Python](https://github.com/emyzelium/emyzelium-py).

## Warning

There are tasks and scales where this model may succeed, seemingly (starting with pet-project-grade ones), and then there are tasks and scales where it will fail, miserably (ending with industry-, and especially critical-infrastructure-grade ones), up to the international day of mourning for its victims. Proceed with caution.

See also [mycoses](https://en.wikipedia.org/wiki/Fungal_infection).

## Demo

Let's use Emyzelium to introduce distributiveness into cellular automata, classical [Conway's Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life) and its variations. [Once...](https://github.com/XC-Li/Parallel_CellularAutomaton_Wildfire) [more...](http://www.shodor.org/media/content/petascale/materials/UPModules/GameOfLife/Life_Module_Document_pdf.pdf) [once...](https://books.google.com.ua/books?id=QN18DwAAQBAJ&pg=PA403&lpg=PA403) [again...](https://www.semanticscholar.org/paper/A-Distributed-Cellular-Automata-Simulation-on-of-Topa/164c577848b943e460aff91255f348256471faa0)

For the sake of definiteness, Linux with installed C++ compiler, `libzmq5`, `libzmq3-dev`, `libncursesw5`, and `libncursesw5-dev` packages is assumed. But the demo should run under other OSes as well.

### On single PC, connected to Internet

Are Tor and public key cryptography *required* to connect peers that reside in RAM of one and the same PC that you own? Of course not. However, before involving many computers, maybe you want to be sure that this thing can work *in principle*.

First of all, [install Tor](https://community.torproject.org/onion-services/setup/install/), [to your Linux](https://support.torproject.org/apt/tor-deb-repo/), and [set up 3 hidden services](https://community.torproject.org/onion-services/setup/) on 3 different ports, arbitrary free ones above 1024. To be more precise, in your `/etc/tor/torrc` add lines like these:

```
HiddenServiceDir /var/lib/tor/p2p_dummysite1/
HiddenServicePort 60847

HiddenServiceDir /var/lib/tor/p2p_dummysite2/
HiddenServicePort 60848

HiddenServiceDir /var/lib/tor/p2p_dummysite3/
HiddenServicePort 60849
```

and in a terminal (note `tor@default` instead of `tor`):

```console
$ sudo systemctl restart tor@default
```

then check if there are any problems:

```console
$ systemctl status tor@default
```

should show `... active(running) ...`

Wait a little for 3 specified dirs to appear, and, in each of them, the file `hostname`.

Now download Emyzelium files, say, to `~/emz-cpp/`. Open `demo/demo.cpp` and, right after `#include`-s, change `ALIEN_ONION` value to onion address from `/var/lib/tor/p2p_dummysite1/hostname` *without `.onion` suffix*. Also change `ALIEN_PORT` value to 60847.

Make analogous changes to `JOHN_ONION`, `JOHN_PORT` (2) and `MARY_ONION`, `MARY_PORT` (3). Save changes to `demo.cpp`.

---

You can also check whether these onion addresses have become known to Tor network; if they have, e.g. `netcat` should work — open 2 terminals and see if it is the case:

```console
term1$ nc -v -l 60847
```

```console
term2$ torsocks nc -v ONION1.onion 60847
```

`connection from ...` and `connected to ...` means that onions are reachable. If they are not, wait for several minutes.

---

From `~/emz-cpp/demo/` build by

```console
$ make
```

Finally, to emyzeliumisation of Life. Open 3 terminals and from `~/emz-cpp/demo` run the following in any order:

```console
term1$ ./demo Alien
```

```console
term2$ ./demo John
```

```console
term3$ ./demo Mary
```

Then you should see something like this:

* Terminal 1 (peer Alien):

![Demo animation, Alien](https://github.com/emyzelium/visuals/blob/main/anim_demo_Alien.gif)

* Terminal 2 (peer John):

![Demo animation, John](https://github.com/emyzelium/visuals/blob/main/anim_demo_John.gif)

* Terminal 3 (peer Mary):

![Demo animation, Mary](https://github.com/emyzelium/visuals/blob/main/anim_demo_Mary.gif)

As soon as Alien's, John's, and Mary's peers (*efungi*) have connected to each other via Tor, their cellular automatons (*realms*) can exchange cell regions (*etales*) not far from realtime.

Before the connections are established, SLUs (Since Last Update) are "large" (no updates yet); afterward they stay in 0–10 sec range. Press "1" or "2" to actually import updated region from other realm.

If you make that import automatic as well as emission, e.g. import from random other realm every 8 seconds, the process will become even more autonomous. Of course, in an environment so tiny, there is not much potential for evolution.

Note that birth/survival rules of Alien's CA, B34/S34, are different from classic B3/S23 of John's and Mary's CAs. In other words, although the "local geometry" of the realms is the same (Moore neigborhood), their "physics" are different.

The names "Alien", "John", "Mary" are not required and are used for convenience. Each peer is identified by its onion, port, and public key.

You can quit any of these 3 instances at any time and run it again after a while, the connections will be restored. The last "snapshot" of the region published by given peer is kept at each peer that has received it before being replaced by the next snapshot.

And you can mix versions in different languages, as long as no more than single instance of each peer runs at the same time. That is, you can replace

```console
t1$ ./demo Alien
```

by

```console
t1$ python3 demo.py Alien
```

from [Emyzelium in Python](https://github.com/emyzelium/emyzelium-py).

### On multiple PCs connected to Internet

As it should be, the only principal difference from "Single PC" scenario is that hidden services are split between PCs. Let there be 3 of them, PC1 "Alien's", PC2 "John's", and PC3 "Mary's".

This time the port can be the same: on all 3 PCs, `/etc/tor/torrc` contains

```
HiddenServiceDir /var/lib/tor/p2p_dummysite/
HiddenServicePort 60847
```

only onion addresses in `hostname` files will be different and, as before, should be specified as `ALIEN_ONION`, `JOHN_ONION`, `MARY_ONION` values in `demo.cpp`. Also, this time it suffices on each PC to have in `demo.cpp` only the single corresponding `SECRETKEY`.

After Emyzelium files with accordingly modified `demo.cpp` have been put on these PCs and `demo` executable has been built, do the following:

```console
pc1$ ./demo Alien
```

```console
pc2$ ./demo John
```

```console
pc3$ ./demo Mary
```

and almost exactly the same as above should be observed.

## Security and keys

Emyzelium relies on ZeroMQ's Curve and ZAP encryption and authentication schemes, a variety of [public key cryptography](https://en.wikipedia.org/wiki/Public-key_cryptography) (basic knowledge of which is presumed). Therefore each "subject" within emyzelium needs and is partially defined by a secret key and a corresponding public key. There are 2 encodings of such keys: raw (32 bytes, each from 0–255 range) and printable [Z85](https://rfc.zeromq.org/spec/32/) (40 symbols, each from 85-element subset of ASCII).

Emyzelium's methods expect the keys as `string`s in Z85 encoding.

To obtain such pair of keys,

```cpp
#include <cstdio>
#include <zmq.h>

int main() {
    char publickey[41];
    char secretkey[41];
    zmq_curve_keypair(publickey, secretkey);
    // Make sure no one is behind your back...
    printf("Public key: %s\nSecret key: %s\n", publickey, secretkey);
    return 0;
}
```

Obviously, *keys are not arbitrary ASCII strings of length 40* that could be "typed by smashing on keyboard". In particular, the public one can be derived from the secret one:

```cpp
char publickey[41];
zmq_curve_public(publickey, secretkey);
```

You construct peers with unique secret keys, one for each. No one except you or those whom you trust should know these keys. Anyone who wants to communicate with your peers must know the corresponding public keys. Accordingly, you must know the public keys of peers run by others if you want to communicate with them, in addition to their onion and port. Using "whitelist" feature (see below) based on ZAP, the owner of a peer can restrict those who are able to communicate with this peer.

Now we look closely at these

## Entities, their roles and usage

In short, efunguz publishes etales, stretches out ehyphae to reach other efungi and receive etales that they publish (you should feel déjà vu here, because it is Synopsis with "mycological" metaphors).

All this throng of blobs better serve some purpose, — there are programs that need to be connected, to exchange data. We call such programs "realms" to emphasize that they may belong to very different environments, may be written in different languages, cannot be easily replaced all at once. Like two mushrooms in a forest, where one grows in a cave and the other at a river bank, but they are part of the same [mycelium](https://en.wikipedia.org/wiki/Mycelium), efungi that you attach to each program communicate, and via efungi, "cave program" and "river bank program" communicate as well. See demo screencasts, Terminals 1–3.

Following the publish-subscribe pattern, Emyzelium model prefers "read" to "write": you cannot write some data to someone's program memory until *they* want to read it and do whatever *they* want with it, no one can write their data to the memory of your program unless *you* want to read that data and do whatever *you* want with it.

Another important property of this pattern is *multicast*: the data has no single intended recipient. Everyone who is allowed to subscribe to etales of your efunguz can receive any etale it publishes, if they know its title. Moreover, you do not know who actually received what etale, even if they received it at all, until they somehow communicate it back to you.

More on this below, but for now, consider these limitations and decide if they are compatible with your goals.

In case they do and one of your programs is in C++, proceed. Otherwise, see [S&B](#sab), [this list](https://en.wikipedia.org/wiki/Category:Message-oriented_middleware) etc.

The header, `emyzelium.hpp`, has to be included, and the source, `emyzelium.cpp`, has to be compiled for your program to use it. Code snippets below assume

```cpp
#include "emyzelium.hpp"

using namespace Emyzelium;
```

See also `demo.cpp` and `Makefile`. In addition, `demo.cpp` contains methods' calls with more arguments.

So, *Efunguz*, *Ehypha*, and *Etale* are just fancy names of well known concepts:

---

**Efunguz**, a.k.a. peer, is the mediator between some "realm", represented by your program, and Tor network, represented by ZeroMQ on top of Tor SOCKS5 proxy, to which it talks. To the former, it simplifies security, (re)connection, and data flow tasks.

The simplest way to construct efunguz is

```cpp
string my_secretkey = "gbMF0ZKztI28i6}ax!&Yw/US<CCA9PLs.Osr3APc";
Efunguz efunguz(my_secretkey);
// or
auto* efunguz = new Efunguz(my_secretkey); // then "delete efunguz;" at the end
```

Some additional parameters:

```cpp
set<string> whitelist_publickeys = {"WR)%3-d9dw)%3VQ@O37dVe<09FuNzI{vh}Vfi+]0", "iGxlt)JYh!P9xPCY%BlY4Y]c^<=W)k^$T7GirF[R"};
uint16_t pubsub_port = 54321;
Efunguz efunguz(my_secretkey, whitelist_publickeys, pubsub_port);
```

Now only the owners of secret keys corresponding to `whitelist_publickeys` will be able to subscribe to and receive etales of this efunguz. And they must connect to port `54321` instead of "default" one.

By default whitelist is empty, which means... opposite to what you might have thought: everyone is allowed to subscribe.

Efunguz is mutable. You can

* add and delete keys from whitelist via `add_whitelist_publickeys()`, `read_whitelist_publickeys()` and `del_whitelist_publickeys()`, `clear_whitelist_publickeys()` methods of Efunguz object

* add and delete *ehyphae* (see below) via `add_ehypha()` and `del_ehypha()`:

```cpp
string that_onion = "abcde23456abcde23456abcde23456abcde23456abcde23456abcdef"
uint16_t that_port = 12345
string that_publickey = "WR)%3-d9dw)%3VQ@O37dVe<09FuNzI{vh}Vfi+]0";
auto& ehypha = get<0>(efunguz.add_ehypha(that_publickey, that_onion, that_port));
```

* publish/emit etales via `emit_etale()`:

```cpp
string title = "status2";
vector<vector<uint8_t>> parts = {{2, 1}, {255, 0, 2, 1}};
efunguz.emit_etale(title, parts);
```

Title can be empty, `""`. It may be an agreement to publish some description of "normal" etales under empty title, so that other efungi will be able to obtain the list of (publicly) available etales:

```cpp
efunguz.emit_etale("",
 {str_to_vec_u8("status2"), str_to_vec_u8("2B humidity, 4B kappa level"),
  str_to_vec_u8("advice"), str_to_vec_u8("C string with today's advice")});
```

* update its state, ehyphae and their etales, using the data received from efungi it is connected to, via `update()`

The appropriate place to call `update()` from is the main loop of your program. Like this:

```cpp
while (!quit) { // main program loop
    // do something here
    efunguz.update();
    if (my_status_updated) {
        efunguz.emit_etale("status2", status_parts);
    }
    if (that_etale.t_in > t_last_etale) {
        if ((that_etale.parts.size() == 2) && (that_etale.parts[1].size() == 4)) { // sanity checks
            int32_t kappa_level = *((int32_t *)that_etale.parts[1].data());
            // do something with kappa level
        }
        t_last_etale = that_etale.t_in;
    }
}
```

See also `Realm_CA::run()` in `demo.cpp`.

*Internally, Efunguz owns ZeroMQ context, PUB socket for etales, and REP socket for ZAP authentication.*

---

**Ehypha**, a.k.a. the connection from one efunguz to another. Via ehypha, the former receives *etales* from the latter. It is a part of Efunguz, thus its construction was considered above.

Ehypha is mutable. You can

* subscribe and unsubscribe to etales from target efunguz via `add_etale()` and `del_etale()`:

```cpp
const auto& that_etale = get<0>(ehypha.add_etale("status3"));
```

At first, etale is empty (no parts). If efunguz with public key `WR)%3-d9dw)%3VQ@O37dVe<09FuNzI{vh}Vfi+]0` is available at onion `abcde23456abcde23456abcde23456abcde23456abcde23456abcdef`, port `12345`, allows subscriptions from your efunguz, and publishes etale under the title `status3`, then, after a while, this etale will be received by you after `efunguz.update()` call, and will be updated as long as these conditions hold. Its fields are described below in *Etale* paragraph.

* pause and resume update of either single etale, or all etales, via `pause_etale[s]()` and `resume_etale[s]()`

*Internally, Ehypha owns SUB socket for etales. The context is the one of Efunguz.*

---

**Etale**, a.k.a. partitioned data chunk with metadata, is the main data unit that efungi exchange.
It has the following public fields:

* `parts` (`vector<vector<uint8_t>>&`) contains the data

* `t_out` (`int64_t`) is the time in microseconds since Unix epoch, measured at sender, when the etale was published

* `t_in` (`int64_t`) is the time in microseconds since Unix epoch, measured at receiver, when the etale was obtained

Etale is immutable from outside and is owned by Ehypha from which it was constructed.

Let "tale" in the name remind that a *tale* may be a *lie*, regardless of intentions of a teller or expectations of a listener.

---

The main data flow then is

Realm 1 ↔ Efunguz 1 ↔ ZeroMQ ↔ Tor ↔ *turtles* ↔ Tor ↔ ZeroMQ ↔ Efunguz 2 ↔ Realm 2

Here it is bidirectional, but may be unidirectional as well. For this to work, each efunguz must know network addresses of all efungi it gathers etales from, that is, of efungi to which its ehyphae are connected. This is where [onions of Tor network](https://community.torproject.org/onion-services/overview/) come in...

...before v0.9.0, there were *Ecataloguzes* a.k.a. nameservers, Efungi talked to them exchanging their (dynamic) IP addresses, and Emyzelium worked over basic TCP/IP Internet... or rather *would* work were there no NATs, firewalls etc. (It actually worked — inside LAN only :) Now, because Tor solves this problem, we do not have to write here another boring section on those nameservers! Also, さよなら port forwarding, hole punching et al.

## PAQ (Potentially Asked Questions)

**Q.** What *novelty* does Emyzelium introduce in comparison with other similar projects?

**A.** IOHO, none.

---

**Q.** How reliable Emyzelium is? How secure? Are there backdoors?

**A.** No "audit" has been performed, so... read the source through carefully, it is small enough — C++ version is almost twice smaller than this README. The buck then goes to underlying layers — ZeroMQ, Curve, Tor, TCP/IP etc. Sorry, there is no other way if you trust only yourself of today.

Yes, there are backdoors. No, there are no backdoors.

Do not omit sanity checks of received etales and during their deserialization.

Do not use keys from demo, generate your own unique pairs.

---

**Q.** Emyzelium is crap, I will never use it, but I want to exchange data with some efungi.
What do I need in addition to their onions, ports, and public keys?

**A.** There is nothing "Emyzelium-specific" in the data that flows between the entities described here. You subscribe to some null-terminated topic on "publisher" port of Efunguz? — It will send corresponding etale to you, if there is no whitelist or you are in it. Write your own/use written by someone wrapper around the parts of ZeroMQ, Tor etc. that you need (cf. [STREAM](http://api.zeromq.org/master:zmq-socket#toc20) sockets). In fact, Emyzelium *implementation* of Emyzelium (or whatever traditional name it has) *architecture* is here already. And to exchange data with emyzelium is to be its part. If the data then goes somewhere else, maybe you aim at a [bridge](https://en.wikipedia.org/wiki/Protocol_converter). After all, you can always rewrite it from scratch or improve what causes your antipathy, rename to "Epór"/"Ekinzhitai"/"E..." (based on "mycelium" in Irish/Japanese/...) and use that.

---

**Q.** Some scoundrels are using emyzelium to commit bad things. How can they be stopped?

**A.** Nothing special for such architectures, probably. Restrict access to Tor, identify devices that participate in the network... some metadata analysis... and there are areas beside mycology.

---

**Q.** Some scoundrels are shutting down emyzelium we use to commit good things. How can they be stopped?

**A.** Nothing special for such architectures, probably. Use Tor bridges, switch devices that participate in the network... some metadata obfuscation... and there are areas beside mycology.

## S&B <a name="sab"></a>

Mostly big ones...

* [ActiveMQ](https://activemq.apache.org/)

* [AMQP](https://www.amqp.org/)

* [DALS](https://github.com/RomuloCANunes/dals)

* [Environs](https://github.com/danja/environs)

* [MQTT](https://mqtt.org/)

* [RabbitMQ](https://www.rabbitmq.com/)

* [XMPP](https://xmpp.org/)

* [Zato](https://zato.io/en/index.html)

* [ZeroEQ](https://github.com/HBPVIS/ZeroEQ)

* [ZeroMQ](https://zeromq.org/)

## License

This wrapper is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This wrapper is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.