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
