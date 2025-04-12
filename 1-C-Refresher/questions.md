1. Why is this safe: `if ((argc < 2) || (*argv[1] != '-')){`; what happens if argv[1] does not exist?

> It's an OR statement, so either argument can satisfy the logic resulting in the IF function running its code.
So, if too few parameters are passed or no '-' is passed, the logic will catch it.

2. What is the purpose of this check in the starter code `if (argc < 3){`? 

> This checks to make sure that the user entered sufficient information to run the code.  Specifically,
it checks to see that there are three parameters passed:  The filename, op-code, and input string.

3. All of the provided helper functions take the buffer as well as the length. Why do you think this is a good practice?

> Most operations on the buffer require knowing the length, in addition, if the buffer were corrupt it would be easier to identify.