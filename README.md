# Emyzelium (C++)

## Synopsis

Emyzelium is another gigathin wrapper around [ZeroMQ](https://zeromq.org/)'s [Publish-Subscribe](https://zeromq.org/socket-api/#publish-subscribe-pattern) and [Pipeline](https://zeromq.org/socket-api/#pipeline-pattern) messaging patterns with mandatory [Curve](https://rfc.zeromq.org/spec/26/) security and optional [ZAP](https://rfc.zeromq.org/spec/27/) authentication filter over TCP/IP for distributed artificial elife, decision making etc. systems where each peer, identified by its public key, provides and updates vectors of vectors of bytes under unique topics that other peers can subscribe to and receive; peers obtain each other's IP addresses:ports by sending beacons and subscribing to nameservers whose addresses:ports are known.

Requires [C++14 compiler support](https://en.wikipedia.org/wiki/C%2B%2B14#Compiler_support) and, of course, [libzmq](https://github.com/zeromq/libzmq) ([more on build](http://wiki.zeromq.org/build:_start)). Nameserver *with* TUI requires [ncurses](https://invisible-island.net/ncurses/announce.html), *without* TUI it does not. Demo requires [ncursesw](https://packages.ubuntu.com/bionic/libncursesw5-dev).

Versions in other languages: [Python](https://github.com/emyzelium/emyzelium-py).

## Warning

There are tasks and scales where this model may succeed, seemingly (starting with pet-project-grade ones), and then there are tasks and scales where it will fail, miserably (ending with industry-, and especially critical-infrastructure-grade ones), up to the international day of mourning for its victims. Proceed with caution.

## Demo

Let's use Emyzelium to introduce distributiveness into cellular automata, classical [Conway's Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life) and its variations. [Once...](https://github.com/XC-Li/Parallel_CellularAutomaton_Wildfire) [more...](http://www.shodor.org/media/content/petascale/materials/UPModules/GameOfLife/Life_Module_Document_pdf.pdf) [once...](https://books.google.com.ua/books?id=QN18DwAAQBAJ&pg=PA403&lpg=PA403) [again...](https://www.semanticscholar.org/paper/A-Distributed-Cellular-Automata-Simulation-on-of-Topa/164c577848b943e460aff91255f348256471faa0)

For the sake of definiteness, Linux with installed C++ compiler, `libzmq5`, `libzmq3-dev`, `libncursesw5`, and `libncursesw5-dev` packages is assumed. But the demo should run under other OSes as well.

### On single PC

(This scenario has only illustration purpose, because TCP/IP and public key cryptography are hardly needed to connect peers that reside in the RAM of a PC that you own.)

Download, say, to `~/emz-cpp/` and from `~/emz-cpp/demo/` build by

```console
$ make
```

Via Ctrl+Alt+T launch 4 terminals of no less than 112×30 size and from `~/emz-cpp/demo/` run the following in any order:

* Terminal 0:

```console
$ ./demo ecatal ▒
```

where `▒` is either `A`, `B`, or `C`.

* Terminals 1, 2, 3:

```console
t1$ ./demo realm Alien
```

```console
t2$ ./demo realm John
```

```console
t3$ ./demo realm Mary
```

Then you should see something like this:

* Terminal 0 (nameserver):

![Demo animation, ecatal](https://github.com/emyzelium/visuals/blob/main/anim_demo_ecatal.gif)

* Terminal 1 (peer Alien):

![Demo animation, Alien](https://github.com/emyzelium/visuals/blob/main/anim_demo_Alien.gif)

* Terminal 2 (peer John):

![Demo animation, John](https://github.com/emyzelium/visuals/blob/main/anim_demo_John.gif)

* Terminal 3 (peer Mary):

![Demo animation, Mary](https://github.com/emyzelium/visuals/blob/main/anim_demo_Mary.gif)

As soon as Alien's, John's, and Mary's peers (*efungi*) have connected to each other via nameserver (*ecataloguz*), their cellular automatons (*realms*) can exchange cell regions (*etales*) not far from realtime.

Before the connections are established, SLUs (Since Last Update) are "large" (no updates yet), after that they should stay in 0–10 sec range. Press "1" or "2" to actually import updated region from other realm.

If you make that import automatic as well as emission, e.g. import from random other realm every 8 seconds, the process will become even more autonomous. Of course, in an environment so tiny, there is not much potential for evolution.

Note that birth/survival rules of Alien's CA, B34/S34, are different from classic B3/S23 of John's and Mary's CAs. In other words, "physics" of the realms are different.

The names "Alien", "John", "Mary" are not required and are used for convenience. The peers are identified by their public keys (see Terminal 0).

You can quit any of these 4 programs at any time and run it again after a while, the connections will be restored. The last "snapshot" of the region published by given peer is kept at each peer that has received it before being replaced by the next snapshot.

And you can mix versions in different languages, as long as no more than single instance of each peer or nameserver runs at the same time. That is, you can replace

```console
t1$ ./demo realm Alien
```

by

```console
t1$ python3 demo.py realm Alien
```

from [Emyzelium in Python](https://github.com/emyzelium/emyzelium-py).

Now we proceed to the scenario with significantly more remoteness...

### On multiple PCs connected over TCP/IP

...but this transition requires almost no additional efforts (which is one of motivations to use something like Emyzelium). Let there be 4 PCs:

* PC 0, nameserver, with known static public IP address, say, `234.56.78.90`.

* PCs 1, 2, 3 are "Alien's", "John's", "Mary's" respectively.

Repeat "Single PC" scenario up to and including `$ make` on all of them. Then

* PC 0: open ports `DEF_ECATAL_BEACON_PORT + 1, 2, 3` and `DEF_ECATAL_PUBSUB_PORT + 1, 2, 3` for incoming connections in the firewall, with the constants taken from `emyzelium.hpp`, set up port forwarding at the router if necessary, run

```console
pc0$ ./demo ecatal ▒
```

* PC 1: open port `DEF_EFUNGI_PUBSUB_PORT + 1`, run

```console
pc1$ ./demo realm Alien 234.56.78.90
```

* PC 2: open port `DEF_EFUNGI_PUBSUB_PORT + 2`, run

```console
pc2$ ./demo realm John 234.56.78.90
```

* PC 3: open port `DEF_EFUNGI_PUBSUB_PORT + 3`, run

```console
pc3$ ./demo realm Mary 234.56.78.90
```

and you should see almost exactly the same. Only nameserver's screen will show IPs of PCs 1, 2, 3 instead of `::ffff:127.0.0.1`.

## Security and keys

Emyzelium relies on ZeroMQ's Curve and ZAP encryption and authentication schemes, a variety of [public key cryptography](https://en.wikipedia.org/wiki/Public-key_cryptography) (basic knowledge of which is presumed). Therefore each "subject" within emyzelium needs and, in a sence, is defined by a secret key and a corresponding public key. There are 2 encodings of such keys: raw (32 bytes, each from 0–255 range) and printable [Z85](https://rfc.zeromq.org/spec/32/) (40 symbols, each from 85-element subset of ASCII).

Emyzelium's methods expect the keys as `string`s in Z85 encoding.

To obtain such pair of keys,

```cpp
#include <cstdio>
#include <zmq.h>

int main() {
    char publickey[41];
    char secretkey[41];
    zmq_curve_keypair(publickey, secretkey);
    // Make sure no one is watching except you...
    printf("Public key: %s\nSecret key: %s\n", publickey, secretkey);
    return 0;
}
```

Obviously, *keys are not arbitrary ASCII strings of length 40* that could be "typed by smashing on keyboard". In particular, the public one can be derived from the secret one:

```cpp
char publickey[41];
zmq_curve_public(publickey, secretkey);
```

You construct peers and nameservers with unique secret keys, one for each entity. No one except you or those whom you trust should know these keys. Anyone who wants to communicate with your entities must know the corresponding public keys. Accordingly, you must know the public keys of entities run by others if you want to communicate with them. Using "whitelist" feature (see below) based on ZAP, the owner of an entity can restrict those who are able to communicate with this entity.

Now we look closely at these

## Entities, their roles and usage

![Entities animation](https://github.com/emyzelium/visuals/blob/main/anim_entities.gif)

In short, efunguz publishes etales, stretches out ehyphae to reach other efungi and receive etales that they publish, sends beacons to ecataloguzes to make its IP address:port known, and receives from ecataloguzes addresses:ports of efungi it wants to reach. (You should feel déjà vu here, because it is Synopsis with "mycological" metaphors.)

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

So, *Efunguz*, *Ehypha*, *Etale*, and *Ecataloguz* are just fancy names of well known concepts:

---

**Efunguz**, a.k.a. peer, is the mediator between some "realm", represented by your program, and TCP/IP network, represented by ZeroMQ, to which it talks. To the former, it simplifies security, (re)connection, and data flow tasks.

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

* add and delete keys from whitelist via `add_whitelist_publickeys()` and `del_whitelist_publickeys()` methods of Efunguz object

* add and delete *ehyphae* (see below) via `add_ehypha()` and `del_ehypha()`:

```cpp
string that_publickey = "WR)%3-d9dw)%3VQ@O37dVe<09FuNzI{vh}Vfi+]0";
auto& ehypha = get<0>(efunguz.add_ehypha(that_publickey));
```

In static case, when the address:port of other efunguz is known in advance, in can be specified:

```cpp
string that_connpoint = "tcp://123.45.67.89:65432";
auto& ehypha = get<0>(efunguz.add_ehypha(that_publickey, that_connpoint));
```

* (dis)connect it to *ecataloguzes* (see below) of 2 kinds: to the ones *to* which it will send beacons so that they know its address:port, via `add_ecatal_to()` and `del_ecatal_to()`, and to the ones *from* which this efunguz will receive addresses:ports of other efungi, via `add_ecatal_from()` and `del_ecatal_from()`:

```cpp
string ecatal_publickey = "d.OT&vpji%VDDI[8QI2L8K]ZiqpwFjxhR{5ftXRp";
string ecatal_beacon_connpoint = "tcp://234.56.78.90:43210";
string ecatal_pubsub_connpoint = "tcp://234.56.78.90:32109";
efunguz.add_ecatal_to(ecatal_publickey, ecatal_beacon_connpoint);
efunguz.add_ecatal_from(ecatal_publickey, ecatal_pubsub_connpoint);
```

You can connect to as many ecataloguzes as you want. If different ecataloguzes provide different connpoints for a given efunguz, majority (among not outdated) wins.

* publish/emit etales via `emit_etale()`:

```cpp
string title = "status2";
vector<vector<uint8_t>> parts = {{2, 1}, {255, 0, 2, 1}};
efunguz.emit_etale(title, parts);
```

Title can be empty, `""`. It may be an agreement to publish some description of "normal" etales here, so that other efungi will be able to obtain the list of (publicly) available etales:

```cpp
efunguz.emit_etale("",
 {str_to_vec_u8("status2"), str_to_vec_u8("2B humidity, 4B kappa level"),
  str_to_vec_u8("advice"), str_to_vec_u8("C string with today's advice")});
```

* send beacons to all "to" ecataloguzes via `emit_beacon()`. Usually it is not required, because `update()` method does it automatically with specified interval

* update its state, ehyphae and their etales, using the data received from efungi and "from" ecataloguzes it is connected to, and send beacons, via `update()`

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

*Internally, Efunguz owns ZeroMQ context, PUB socket for etales, PUSH socket for each "to" ecataloguz and SUB socket for each "from" ecataloguz. There is also REP socket for ZAP authentication.*

---

**Ehypha**, a.k.a. the connection from one efunguz to another. Via ehypha, the former receives *etales* from the latter. It is a part of Efunguz, thus its construction was considered above.

Ehypha is mutable. You can

 * change its connection point — address:port of target efunguz — via `set_connpoint()` method of Ehypha object:

 ```cpp
 string that_new_connpoint = "tcp://12.34.56.78:12345";
 ehypha.set_connpoint(that_new_connpoint);
 ```

* subscribe and unsubscribe to etales from target efunguz via `add_etale()` and `del_etale()`:

```cpp
const auto& etale = get<0>(ehypha.add_etale("status3"));
```

At first, etale is empty (no parts). If efunguz with public key `WR)%3-d9dw)%3VQ@O37dVe<09FuNzI{vh}Vfi+]0` is available at connpoint `tcp://12.34.56.78:12345`, allows subscriptions from your efunguz, and publishes etale under the title `status3`, then, after a while, this etale will be received by you after `efunguz.update()` call, and will be updated as long as these conditions hold. Its fields are described below in *Etale* paragraph.

* pause and resume update of either single etale, or all etales, via `pause_etale[s]()` and `resume_etale[s]()`

*Internally, Ehypha owns SUB socket for etales. The context is the one of Efunguz.*

---

**Etale**, a.k.a. partitioned data chunk with metadata, is the main data unit that efungi exchange.
It has the following public fields:

* `parts` (`vector<vector<uint8_t>>&`) contains the data

* `t_out` (`int64_t`) is the time in microseconds since Unix epoch, measured at sender, when the etale was published

* `t_in` (`int64_t`) is the time in microseconds since Unix epoch, measured at receiver, when the etale was obtained

Etale is immutable from outside and is owned by Ehypha from which it was constructed.

"Tale" in the name should remind that a _tale_ may be a _lie_, regardless of intentions of a teller.

---

The main data flow then is as follows:

Realm 1 ↔ Efunguz 1 ↔ ZeroMQ ↔ TCP/IP ↔ *turtles* ↔ IP/TCP ↔ ZeroMQ ↔ Efunguz 2 ↔ Realm 2

(here it is bidirectional, but may be unidirectional as well). For this to work, each efunguz must know network addresses of all efungi it gathers etales from, that is, of efungi to which its ehyphae are connected. These addresses can be set at initialization if they are static (see `add_ehypha()` example above), but in dynamic case each efunguz can change its address from time to time and without prior knowledge neither of the instant when the change occurs, nor of the new address. The solution is usual:

**Ecataloguz**, a.k.a. nameserver, makes IP addresses:ports of efungi known to each other.
Unlike Efunguz, it does not "attach" to some program, but runs as a standalone program itself on a device with known static public IP.

On the one hand, it receives beacons from efungi and thus obtains their public keys along with network addresses:ports. On the other hand, it publishes these addresses:ports under topics equal to public keys and they go to efungi that subscribe to these topics-keys (these are keys of efungi they stretched out their ehyphae to).

```cpp
string my_secretkey = "T*t*)FNSa1RSOG9Dbxuvq1M{hE-luf{YjW+8j^@1";
Ecataloguz ecatal(my_secretkey);
ecatal.run();
```

If `EMYZELIUM_ECATAL_TUI` is defined (by default it is, at the beginning of `emyzelium.cpp`), `ncurses` is installed and the program is linked with `ncurses` library, Ecataloguz will run with the text user interface that you see on demo screencast of Terminal 0. Otherwise there will be no TUI, and it must be stopped by Ctrl+C, from Task Manager, or by sending `SIGINT`, `SIGKILL` etc.

Typical adjustments:

```cpp
map<string, string> beacon_whitelist_publickeys_with_comments = {{"iGxlt)JYh!P9xPCY%BlY4Y]c^<=W)k^$T7GirF[R", "Alien"}, {"(>?aRHs!hJ2ykb?B}t6iGgo3-5xooFh@9F/4C:DW", "John"}};
set<string> pubsub_whitelist_publickeys = {"(>?aRHs!hJ2ykb?B}t6iGgo3-5xooFh@9F/4C:DW", "WR)%3-d9dw)%3VQ@O37dVe<09FuNzI{vh}Vfi+]0"};
uint16_t beacon_port = 43210;
uint16_t pubsub_port = 32109;
Ecataloguz ecatal(my_secretkey, beacon_whitelist_publickeys_with_comments, pubsub_whitelist_publickeys, beacon_port, pubsub_port);
ecatal.run();
```

Ecataloguz is mutable... but the only 2 things you can do between constructing and running it are `read_beacon_whitelist_publickeys_with_comments()` and `read_pubsub_whitelist_publickeys()` methods of Ecataloguz object that expect the path to the file (`string`) containing these keys. See `publickeys_with_comments.txt` for an example.

The role of Ecataloguz requires that "from the point of view" of the device you run it on, network addresses of all efungi sending their beacons are such that other efungi can connect to them using these addresses. I.e. if one of efungi resides in the same LAN as ecataloguz, then its address of `192.168.0.123` kind will be useless to efungi outside of that LAN. There are also some "IPv4 vs. IPv6" complications (see `DEF_IPV6_STATUS` constant in `emyzelium.cpp`).

*Internally, Ecataloguz owns ZeroMQ context, PULL socket for beacons, PUB socket for efungi' connpoints, and REP socket for ZAP authentication.*

## PAQ (Potentially Asked Questions)

**Q.** How reliable Emyzelium is? How secure? Are there backdoors?

**A.** No "audit" has been performed, so... read the source through carefully, it is small enough. The buck then goes to underlying layers (ZeroMQ, Curve, TCP/IP etc.) Sorry, there is no other way if you trust only yourself.

Yes, there are backdoors. No, there are no backdoors.

Do not omit sanity checks of received etales and during their deserialization.

Do not use keys from demo, generate your own unique pairs.

---

**Q.** There are arguments of constructors that specify time intervals. Why their defaults (`DEF_*_INTERVAL` constants at the beginning of `emyzelium.hpp`) are so large?

**A.** In Emyzelium, time intervals are measured in microseconds: 1 sec is 1000000.

---

**Q.** Emyzelium is crap, I will never use it, but I want to exchange data with some efungi.
What do I need in addition to their public keys?

**A.** There is nothing "Emyzelium-specific" in the data that flows between the entities described here. You send 2 bytes to "beacon" port of Ecataloguz without whitelist? — It records your address as if you are an Efunguz. You subscribe to some null-terminated topic on "publisher" port of Efunguz? — It will send corresponding etale to you (again, if there is no whitelist). Write your own/use written by someone wrapper around the parts of ZeroMQ, TCP/IP etc. that you need (cf. [STREAM](http://api.zeromq.org/master:zmq-socket#toc20) sockets). In fact, Emyzelium *implementation* of Emyzelium (or whatever traditional name it has) *architecture* is here already. And to exchange data with emyzelium is to be its part. If the data then goes somewhere else, maybe you aim at a [bridge](https://en.wikipedia.org/wiki/Protocol_converter). After all, you can always rewrite it from scratch or improve what causes your antipathy, rename to "Epór"/"Ekinzhitai"/"E..." (based on "mycelium" in Irish/Japanese/...) and use that.

---

**Q.** Some scoundrels are using emyzelium to commit bad things. How can they be stopped?

**A.** Nothing special for such architectures, probably. Switch off ecataloguzes/nameservers, identify devices that participate in the network... some metadata analysis... and there are areas beside mycology.

---

**Q.** Some scoundrels are shutting down emyzelium we use to commit good things. How can they be stopped?

**A.** Nothing special for such architectures, probably. Install backup ecataloguzes/nameservers, change devices that participate in the network... some metadata obfuscation... and there are areas beside mycology.

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