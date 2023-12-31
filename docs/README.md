# Guideline
## Formating of source files
This follows the [LLVM coding standards](https://llvm.org/docs/CodingStandards.html) in general with an indention of 4 spaces. To ensure this, clang-format-11 is used with this `.clang-format` configuration file:

    ---
    # We'll use defaults from the LLVM style, but with 4 columns indentation.
    BasedOnStyle:  LLVM
    IndentWidth: 4
    ---
    Language: Cpp
    # Force pointers to the type for C++.
    PointerAlignment: Left

and use it, for example:

    clang-format-11 upnp/src/api/upnpapi.cpp

Just add option `-i` to modify the source files.

## C++ programming rules
In general we refere Bjarne Stroustrup [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines). In detail we mainly follow this [Style](https://lefticus.gitbooks.io/cpp-best-practices/content/03-Style.html) with some exceptions as listed below.

- Using [exceptions for error handling](https://www.acodersjourney.com/top-15-c-exception-handling-mistakes-avoid/).
- The names of global variables should start with //.
- A Unit that can produce a segmentation fault is considered to be buggy.
- Check variables against visibility, lifetime, and thread safety.
- [Declare local variables nearly where they are first used](https://isocpp.org/wiki/faq/coding-standards#declare-near-first-use), not always at the top of a function.
- Always prefix namespace std::, never "using namespace std".
- Avoid pointer casts and/or reference casts.
- I prefer to use "snake case" variable_names_with_underscores. I find it better readable than "camel case" variablesWithUpperCaseLetters.
- Clases start with upper case C: CMyClass;
- Interfaces start with upper case I: IMyClass;
- Use [C++ visibility](https://stackoverflow.com/q/69890807/5014688).
- [Pull methods outside of a class](https://www.fluentcpp.com/2017/06/20/interface-principle-cpp/), whenever it is possible.
- Friend classes and friend functions are not used.
- There is a rule saying "[Do not use underscores in test suite names and test names](https://google.github.io/googletest/faq.html#why-should-test-suite-names-and-test-names-not-contain-underscore)". Because we know the problems we are free to violate the rule for readability with the following restrictions: test suite names only written in "camel case", test names are written in "snake case" without leading and trailing underscore.
- If supported by C++ nullptr is a valid entry and should never cause a segmentation fault. Functions and methods return successful then.
- By definition errors cannot be handled by the library and throw an exception.
- Warnings are errors that can be fixed by the library. It will continue execution with the fixed error but a Warning is logged because the correction needs attention.
- The IPv6 protocol stack is always available. There is no option to disable it. Only IPv6 will be used for library internal networking. IPv4 addresses are mapped to IPv6.

## Git Commit Messages
The [Udacity Git Commit Message Style Guide](https://udacity.github.io/git-styleguide/) served as a template.

### Message Structure
A commit messages consists of three distinct parts separated by a blank line: the title, an optional body and an optional footer. The layout looks like this:

    type: Subject

    body

    footer

The title consists of the type of the message and subject.

### The Type
The type is contained within the title and can be one of these types:

- feat: A new feature
- fix: A bug fix
- docs: Changes to documentation
- style: Formatting, missing semi colons, etc; no code change
- re-en: Re-engineering program code
- test: Adding tests, refactoring test; no production code change
- build: Updating build tasks, package manager configs, etc; no production code change

### The Subject
Subjects should be no greater than 50 characters, should begin with a capital letter and do not end with a period.

Use an imperative tone to describe what a commit does, rather than what it did. For example, use change; not changed or changes.

### The Body
Not all commits are complex enough to warrant a body, therefore it is optional and only used when a commit requires a bit of explanation and context. Use the body to explain the what and why of a commit, not the how.

When writing a body, the blank line between the title and the body is required and you should limit the length of each line to no more than 72 characters.

### The Footer
The footer is optional and is used to reference issue tracker IDs.

### Example Commit Message
    feat: Summarize changes in maximal 50 characters

    More detailed explanatory text, if necessary. Wrap it to about 72
    characters or so. In some contexts, the first line is treated as the
    subject of the commit and the rest of the text as the body. The
    blank line separating the summary from the body is critical (unless
    you omit the body entirely); various tools like `log`, `shortlog`
    and `rebase` can get confused if you run the two together.

    Explain the problem that this commit is solving. Focus on why you
    are making this change as opposed to how (the code explains that).
    Are there side effects or other unintuitive consequences of this
    change? Here's the place to explain them.

    Further paragraphs come after blank lines.

     - Bullet points are okay, too

     - Typically a hyphen or asterisk is used for the bullet, preceded
       by a single space, with blank lines in between, but conventions
       vary here

    Put references to an issue at the bottom, like this:

    Resolves issue: #123
    See also: #456, #789

## Template for a TEST header
Example:
    // Steps as given by the Unit:
    // 1. sock_make_no_blocking
    // 2. try to connect
    // 3. check connection an and wait until connected or timed out
    // 4. sock_make_blocking

    // Configure expected system calls:
    // * make no blocking succeeds
    // + starting connection (no wait) returns with -1, errno = EINPROGRESS;
    // * connection succeeds
    // * make blocking succeeds

## To be thread-safe
Because it isn't possible in general to test if a software is thread-safe I will ensure this by using recognized methods:
- [Using data that is not shared between threads](https://softwareengineering.stackexchange.com/a/285942)
- Using immutable data
- [Message passing with threads](https://web.mit.edu/6.005/www/fa14/classes/20-queues-locks/message-passing/#message_passing_with_threads)

## Some references and 0ptimization
- [Optimize String Use](https://www.oreilly.com/library/view/optimized-c/9781491922057/ch04.html)
- [ Move Objects](https://newbedev.com/is-std-vector-copying-the-objects-with-a-push-back) instead of coppying.
- [Sockets - Server & Client](https://www.bogotobogo.com/cplusplus/sockets_server_client.php)
- [Blocking vs. non-blocking sockets](https://www.scottklement.com/rpg/socktut/nonblocking.html)
- [Operator Overloading](https://condor.depaul.edu/ntomuro/courses/262/notes/lecture3.html)
- [Unicode in C and C++: What You Can Do About It Today](https://www.cprogramming.com/tutorial/unicode.html)
- [How to use UTF-8 and Unicode in C++?](https://stackoverflow.com/questions/61977664/how-to-use-utf-8-and-unicode-in-c-how-big-is-c20-char8-t)
- [CMake target_link_libraries PUBLIC, PRIVATE and INTERFACE keywords](https://stackoverflow.com/q/26037954/5014688)
Rule of Zero/Three/Five
- ["Rule of Zero", R. Martinho Fernandes 08/15/2012](https://web.archive.org/web/20130211035910/http://flamingdangerzone.com/cxx11/2012/08/15/rule-of-zero.html)
- ["A Concern about the Rule of Zero", Scott Meyers, 3/13/2014](http://scottmeyers.blogspot.fr/2014/03/a-concern-about-rule-of-zero.html)
- [What is The Rule of Three?](https://stackoverflow.com/q/4172722/5014688)
- [What is the copy-and-swap idiom?](https://stackoverflow.com/q/3279543/5014688)
- [public friend swap member function](https://stackoverflow.com/q/5695548/5014688)
- [What is move semantics?](What is move semantics?)
- [Semaphores](https://pages.cs.wisc.edu/~remzi/Classes/537/Fall2008/Notes/threads-semaphores.txt)

<pre><sup>
// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, &#60;Ingo&#64;Hoeft-online.de&#62;
// Redistribution only with this Copyright remark. Last modified: 2023-09-25
</sup></sup>
