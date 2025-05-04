1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  Fork instantiates a new process, which allows us to call on external programs like ls without shutting down the main program.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  The child process would return a failed exit code which would be printed.  It exits the process, prints the error, and returns the error code (for rc to print).

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  It searches the path variable to look for the program name.  Path is a system environment parameter.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  It allows us to keep the programs synchronized.  If we didn't wait the parent process would continue on and could conflict with the child, in our scenario it's best to wait for the child process return code before continuing on.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  It returns a non-zero number in the event of a process failure. This tells us if the child process was successful.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  It searches for a quote in arg string.  If it finds one it looks for the next quote in the string to close out the argument, writing that segment to a string variable.  It's necessary as items in quotes need to be handled verbatim.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  I previously lumped all the args together into a single argument field.  I add an arg parser that chops that up based on spacing and quotes.  For me, C is always a challenge as it's finicky about pretty much everything.

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  Signals are a type of interupt used to send event triggers to a process.  They are sent as a type of statusing and not used to move data (like pipes).

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  SIGKILL: terminates a process immediately.  SIGTERM: requests that a process terminate; allows for a graceful shutdown.  SIGINT: like SIGTERM, but the request comes form the terminal (cntrl-c).

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  It's a request to pause the process and cannot be caught or ignored.  SIGSTOP is handled directly by the kernel, so there's no catching or ignoring it.
