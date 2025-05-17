1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

Waitpid is called to force the parent to wait for the child to finish its operations.  If waitpid were not called the parent process would continue, which would result in an unknown state.

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

A pipe is essentially a pointer to a file, and when you use dup2 you rename that pointer to refer to a different file.  Leaving the pipe open would result in files not being closed properly which can result in lost data and errors.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

You can only update the path to your local process. If you tried to fork/exec you wouldn't be affecting the parent path.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

You would need to remove the hard coded limit to the number of pipes and implement automatic resizing of the struct data sizes.  This would likely result in slower operation, as a memory resize takes time.  You could allocate more memeory space to limit the number of resizes, however that ties up resources.
