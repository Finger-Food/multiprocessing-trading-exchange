# Local Trading Exchange written in C

## Overview

This is a trading exchange created as part of a Systems Programming course written in C but then optimised for time efficiency out of interest. Though a university assignment it's a project I'm quite fond of due to the time and effort I put into enhancements beyond the scope of the task for the purposes of completeness and efficiency (e.g., writing up a generalised hashmap amongst other data structures for the purpose of storing, managing, and matching transactions).

At a high level, the exchange takes in filepaths representing trader binaries as command line inputs, and then excutes them on a forked child process on start up. Communication between between traders and the exchange takes place through interprocess pipes, where each message is followed by a signal to flag the transmission of a message.

I created hashmap, linkedlist, and circular queue data structures with some twists—a combined node hashmap linked list node such that items retrieved from the hashmap could be immediately extracted from the linkedlist—to allow for constant time or linear time processing of all trader commands. These data structures were tested through CMocka's unit tests, while the exchange itself tested through E2E tests.

## Setup

1. Clone the repository:
	```bash
	git clone https://github.com/Finger-Food/multiprocessing-trading-exchange
	cd multiprocessing-trading-exchange
	```

2. Compile the exchange
	```bash
	make all
	```

3. Compile the test traders and unit tests
	```bash
	make tests
	```

## Execution

The exchange is run through:
```bash
./pe_exchange traderbin_1 traderbin_2 [...]
```
where `traderbin_i` is the $i$'th trader binary in the exchange.

The tests are run through:
```bash
make run_tests
```