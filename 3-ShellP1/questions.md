1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  fgets captures entire lines of inputs rather than characters.  Since we are entering lines of information, it's easier to capture it all at once
    and parse the input vs trying to glue together a bunch of characters.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  This allows the flexiblity of resizing allocation, if necessary.  It also utilizes the heap, rather than the stack for storage, if larger memory spaces are needed.
    In addition, allows the data to exist beyond the execution of the input code block.


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  The storage size would be variable based on the user input, and in addtion, when the command is executed we need to use a compare (if/then, or switch).  This would be impossible
    without knowing the base commmand without extra characters around it.

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  1. Redirecting data to a file; 2. multiple redirections, for example some data goes to the screen and some to a file (or dev/null); 3. appending redirected output to an existing file.  All of these implementations will require extensive file handling and sorting capability.

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  Redirection points data streams to different files, for example from the screen to a text file.  Piping takes the output data stream of a process and points it to a second process
    as input. For example one process might capture data and the next might sort that data.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  There are situations where errors can be hidden in large standard output streams.  If you couldn't sort it through redirection, it would be difficult to determine standard ouput from errors.

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  It should report to the user that the command was unsuccessful and shut down open processes gracefully.  At a minimum errors and output should be on seperate lines and tagged by the shell as stdin or stout, which, it seems is what Linux does; redirects can then be used to sort the outputs.  Color coding would also be useful to separate the two types.