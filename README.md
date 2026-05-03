# 📁 TFTP Client-Server File Transfer System

## 📌 Overview

This project is a simplified implementation of the **TFTP (Trivial File Transfer Protocol)** using **C and UDP sockets**. It enables reliable file transfer between a client and a server using block-based communication, acknowledgments, and retry mechanisms.

The project demonstrates how reliability can be built on top of an unreliable protocol like UDP.

---

## 🚀 Features

* 📤 Upload files from client to server (**PUT / WRQ**)
* 📥 Download files from server to client (**GET / RRQ**)
* 📦 Block-based data transfer (512 bytes)
* 🔁 Acknowledgment (ACK) mechanism for reliability
* ⏱ Timeout and retry handling
* 🔄 Multiple transfer modes:

  * Normal (512 bytes)
  * Octet (1 byte at a time)
  * Netascii (newline conversion)

---

## 🛠 Tech Stack

* C Programming
* UDP Socket Programming
* Linux System Calls

---

