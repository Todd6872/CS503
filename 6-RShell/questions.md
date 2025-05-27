1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

A special character(s) is used, in our case the EOL, to mark the end of the message.  A partial message can be buffered and held until the remainder of the message arrives.

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

A message header and trailer (or character count) are used to define the beginning and end of message.  If the message beginning and end aren’t detected, it will result in corrupt data being received.

3. Describe the general differences between stateful and stateless protocols.

In a stateful protocol, the server maintains a history of the connection and session information allowing for a more efficient transfer of data.  A stateless protocol treats all incoming data as independent of previous requests, which is easier to implement.

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

UDP is useful in applications where speed and low overhead are a priority, and the occasional lost packet won’t matter very much.  Examples include streaming, VOIP, and gaming.

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

Sockets provide the interface to network communications, containing the IP address and port number in a single file descriptor.