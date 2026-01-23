# ft_irc — IRC Server in C++98

> A compact, concurrent IRC server for the 42 ft_irc project.

## ✨ Overview
ft_irc is an IRC server built in C++98 using TCP sockets and `poll()`.
It supports IRC-style channels, private messages, and core operator controls.

---

## ✅ Features
- Non-blocking TCP server with `poll()` (up to 1000 clients)
- PASS/NICK/USER registration flow with CAP negotiation
- Channel system: create, join, part, and broadcast messages
- Private messages to users or channels (PRIVMSG)
- Operator tools: KICK, INVITE, TOPIC, MODE
- Channel modes: invite-only (+i), topic protection (+t), key (+k), user limit (+l), operator (+o)
- INFO/HELP and WHO for discovery
- PING/PONG keepalive and QUIT handling

---

## 🧰 Tech stack
| Layer | Tech |
| --- | --- |
| Language | C++98 |
| Networking | POSIX sockets (TCP) |
| I/O multiplexing | `poll()` |

---

## 🛠️ How to build & run
```bash
make
./ircserv <port> <password>
```

---

## 🔎 How to test
**Using netcat (manual handshake):**
```bash
nc 127.0.0.1 <port>
```
Then send:
- PASS <password>
- NICK <nickname>
- USER <user> 0 * :<real name>

**Using an IRC client (e.g., Irssi):**
- /connect 127.0.0.1 <port>
- /quote PASS <password>

---

## 📚 What I learned
- Building a concurrent TCP server with `poll()` and non-blocking I/O
- Designing IRC command parsing and stateful client handling
- Implementing channel permissions, modes, and operator workflows
- Clean C++98 architecture for networking projects

---

## 👥 Authors / Credits
- Nebtakhet
- dbejar-s — https://github.com/dbejar-s
- Ginger-Leo — https://github.com/Ginger-Leo
