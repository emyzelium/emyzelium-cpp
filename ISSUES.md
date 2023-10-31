1. Sometimes at Efunguz dropping, e.g. at demo program exit,

```
Bad file descriptor (src/epoll.cpp:░░░)
Aborted (core dumped)
```

happens. Does not occur while demo is running. Perhaps similar assertion failures in `zmq::epoll_t::rm_fd()` are mentioned [here](https://github.com/zeromq/libzmq/issues/1627), and [here](https://hyperledger-indy.readthedocs.io/projects/plenum/en/latest/misc/zeromq_features.html#there-is-no-ability-to-track-and-drop-clients-connections-using-zeromq-api), and [here](https://marc.info/?l=zeromq-dev&m=138373847229120&w=2), and [here](https://www.mail-archive.com/zeromq-dev@lists.zeromq.org/msg28846.html), and [here](https://www.mail-archive.com/zeromq-dev@lists.zeromq.org/msg31287.html).

**Affected versions:** 0.9.0 (2023.10.08) – 0.9.4 (2023.10.31)

**Temporary workaround** is to drop Efunguz at the very end of your program, when everything important has been saved, flushed, closed etc.