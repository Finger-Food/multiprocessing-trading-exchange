## A Trading Exchange written in C

This is a trading exchange created as part of a university assignment written in C. Though a university assignment it's a project I'm quite fond of due to the time and effort I put into enhancements beyond the scope of the task for the purposes of completeness and efficiency (e.g., writing up a generalised hashmap amongst other data structures for the purpose of storing, managing, and matching transactions).

The exchange takes in trader binaries as command line inputs and then executes them as as it starts up. Communication between between the traders and the exchange takes place through pipes, with a signal being sent following to indicate the sending of a message.

I plan to do this README properly at some point, but for now I've just left it with some answers I had for basic university questions on functionality.

---

1. Describe how your exchange works.

Initial Phase: The exchange verifies command line arguments, parses the product file,forks and executes the trader files, whilst also initialising variables and setting up signal handlers and blockers.

Main loop: check if there are traders still in exchange  -->  wait till a signal is received (indicating message or disconnection)
	-->  check the corresponding pipe of trader that sent signal (if trader hasn’t disconnected) -->  parse message (if there is one)
	-->  process command, respond to traders and adjust/create orders (if valid)
	-->  match order according to price-time priority, including fee/position calculations/adjustments and fill messages
	-->  add order to book if not completed (and remove orders that are) -->  output orderbook and positions
	-->   repeat

Throughout all this, the SIGUSR1 and SIGCHLD are monitored for via a lightweight handler and queue, signal blocking when necessary (missing signals are of lower concern than race conditions leading to incorrect behaviour)


2. Describe your design decisions for the trader and how it's fault-tolerant.

The key design decisions were using epoll instead of signals (efficiency) and also to send extra signals to the exchange so long as there is an ‘accepted’ message lacking (fault tolerance).

Signals are slower, can be missed and with the size of the message that are to be sent/received, the reads and writes will be atomic. Hence, by using epoll to determine when the exchange sends a message, the trader can respond faster than otherwise.

By receiving multiple signals simultaneously, the exchange can miss the autotrader's signals. The trader accounts for this  by following up with further signals if the message received following an order isn’t an ‘accept’ or if a response isn't received by the trader messages within a reasonable time frame (along side other checks of signal without message, incorrect number of command line args, invalid message, etc.) making it fault tolerant.


3. Describe your tests and how to run them.

The tests are made up of:
	- Unit tests to check the data structures function correctly (the hash map, linked list and queue), some of them being tailored towards their function in the exchange program
	- End to end tests (input binaries against expected output) to verify the overall control flow of the programs, both exchange and trader: I run the exchange against test traders to assert its output in various circumstances and inputs (including timing). And also the autotrader with the exchange against trader binary’s specifically to test its functionality

They are made using ‘make tests’, and run via ‘make run_tests’ (the second just being an intermediate for a bash test script I’ve created).
