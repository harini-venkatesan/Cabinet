# Cabinet
Structured peer-to-peer filesystem using OpenDHT and FUSE.

Authors: [Harini Venkatesan](https://github.com/harini-venkatesan/) and [Eric Chan](https://github.com/eric-m-chan) 

## Introduction

Cabinet is a C++14 structured peer-to-peer filesystem that uses [libfuse](https://github.com/libfuse/libfuse) and [openDHT](https://github.com/savoirfairelinux/opendht). 

## Documentation

* [Wiki pages](https://github.com/harini-venkatesan/Cabinet/wiki)
* [Build ](https://github.com/harini-venkatesan/Cabinet/wiki/Build) 
* [DHT functions ](https://github.com/harini-venkatesan/Cabinet/wiki/DHT-Functions) 
* [FUSE functions ](https://github.com/harini-venkatesan/Cabinet/wiki/FUSE-Functions)
* [Hash Table Structure ](https://github.com/harini-venkatesan/Cabinet/wiki/Hash-Table-Structure)

## Objective

Our objective is to create a structured peer-to-peer filesystem using OpenDHT and FUSE. Traditional file systems are generally structured in a client-server format. For example, cloud services such as Google Drive are used to host and store files. Another approach is a peer-to-peer network, in which every participating user is both a client and server. Every user will be responsible for storing files, but every user also has the ability to distribute their file for storage. Some benefits of a peer-to-peer network are:

* Scalability: the more users the better the system performs.
* Decentralization: Users are free to join and leave the network with little repercussions. Additionally, there is no single point of failure and no overarching data authority.

## References

* [libfuse](https://github.com/libfuse/libfuse)
> FUSE (Filesystem in Userspace) is an interface for userspace programs to export a filesystem to the Linux kernel. The FUSE project consists of two components: the fuse kernel module (maintained in the regular kernel repositories) and the libfuse userspace library (maintained in this repository). libfuse provides the reference implementation for communicating with the FUSE kernel module.

* [FUSE Example](https://github.com/Aveek-Saha/FUSE-Filesystem)

A FUSE filesystem that uses a binary file, by Aveek Saha.

* [FUSE Dispatcher](http://www.circlesoft.com/fusecpp.h) 

A FUSE C to C++14 dispatcher, by Gerard J. Cerchio

* [OpenDHT](https://github.com/savoirfairelinux/opendht)
> A lightweight C++14 Distributed Hash Table implementation. OpenDHT provides an easy to use distributed in-memory data store. Every node in the network can read and write values to the store. Values are distributed over the network, with redundancy.

