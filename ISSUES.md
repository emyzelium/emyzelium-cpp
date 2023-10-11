Version 0.9.2 (2023.10.11)
--------------------------

* Sometimes at demo program *exit*,

```
Bad file descriptor (src/epoll.cpp:░░░)
Aborted (core dumped)
```

happens. Does not occur while demo is running. Perhaps similar assertion failures in `zmq::epoll_t::rm_fd()` are mentioned [here](https://www.mail-archive.com/zeromq-dev@lists.zeromq.org/msg31287.html), and [here](https://github.com/zeromq/libzmq/issues/1627), and [here](https://www.mail-archive.com/zeromq-dev@lists.zeromq.org/msg28846.html), and [here](https://marc.info/?l=zeromq-dev&m=138373847229120&w=2).