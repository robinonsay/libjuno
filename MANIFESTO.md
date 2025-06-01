# The Juno Manifesto

## Embedded software development has a dependency problem
  * Libraries often take-on many dependencies without considering downstream users
## What do I mean by dependency?
  * There are different kinds of dependencies in software
  * I will be focusing on two of them:
    * Internal Depedencies
      * A depedency on another capability defined within a software application or library
      * Example:
        * I am developing a software system composed of many capabilities. I am
          writing a capability that reads and processes IMU data. This application has
          a depedency on my software system and types and for the sake of argument the math functions I wrote.
    * External Dependencies 
      * A depedency on a capability not provided by the software application or library
      * Example:
        * I am developing a software system composed of many capabilities. I am
          writing a capability that reads and processes IMU data. This application has
          a depedency on an operating system and a networking library
    * What is a capability?
      * A capability in software is a module, class, component, section, or piece of code
    that delivers some aggregated capability, possibly utilizing multiple functions.
      * An example would be a class in C++
## The problem with tight coupling
  * Embedded software is often written with very tight coupling
  * Coupling refers to directly calling on internally and externally depedent
    capabilities without defining a clear boundry
  * This can easily be seen through direct function calls within C/C++
  * Libraries are often composed of many capabilities together
    * You wouldn't rewrite an entire OS because that would take years
    * Someone else has written an OS already
    * A library will utilize the capabilities provided by an OS and take on
      an external depedency
  * These capabilities often have interdependent nature
  * It's rare to find a capability that does not build off a depdent capability
  * This tight coupling is obvious to implement
    * A function is provided to you, you call the function
    * A class is provided to you, you implement the class in your class
    * A capability is provided to you, your capability owns the depdency
      and provides a new capability
  * What happens when something changes?
    * If someone wants to modify a very specific depedency, the tight coupling
      cascades the change into an enormous development effort
    * If a requirement changes, this tight coupling means a change to a single
      capability impacts all owners of that dependency
    * If your software has capabilities that all own many nested depdencies changes
      are difficult to make
  * What happens when you need to test your software?
    * We often want to test capabilities in isolation before testing an integrated
      software system
    * When capabilities are tightly coupled, it becomes difficult to isolate one
      capability from another
      * Where does one capability start, and another end
    * When capabilities own depdencies, they own that capability as well
      * A single capability ends up owning many sub-capabilities
  * Tight coupling of software capabilities and dependcies operate under an asuumption
    that the software will not need to change or adapt
    * I dare you to name a project where all of your software remained the same
      from the moment you wrote it.
  * This resistance to adaptaion and change is literally expensive
    * Developers could spend an entire year testing or modifying software
      because the depdencies are so tightly coupled
    * Developers have to rewrite capabilities because it's easier than
      owning software that addresses a capability due to the underlying dependencies
      of that software
    * Time = Money and this tight coupling of software costs projects
      potentially millions of dollars worth of developer time.
## Scalability
  * Scalability is more than just the number of users
  * Software also scales with the project lifecycle and codebase size
  * Tightly coupled software is **ideal** for software less than 5KSLOC
    * When software size is small, you should tightly couple your system
  * Tightly coupled software **DOES NOT SCALE** to larger projects
    * As the code base size increases, so does the complexity
    * When the depdency graph of your codebase increases in complexity
      so does the codebase's resistance to change
    * When capabilities own dependent capabilities, who own dependent capabilities
      the codebase becomes much more time consuming to maintain
* Is there a way to fix the dependecy problem?
  * The answer is yes!
  * Web software requires fast-paced rapid iteration
  * I'd bet a new javascript library was released as I'm typing this sentence
  * Web software companies literally can't afford to waste money of tightly
    coupled software. It doesn't scale.
  * Web developers have solved this problem using **Dependency Injection**

## Dependency Injection to the Rescue
* What if your software capability did not need to own dependent capabilities?
  * What if these dependencies were provided to you at runtime?
* What if you "invert" the depdency so the capability that depends on mine specifies
  my dependencies?
* This practive of "Inversion of Control" is called **Dependency Injection** or **DI**
* DI allows a primary application to specify the implementation for all dependencies
* This occurs in a single location so capabilities are not duplicated
* When software is inevitably changed, the change is strictly isolated to
  the capability that owns it. It no longer cascades.
* This allows capabilities to be adaptable, reusable, and much easier to test
  * Because the capabilities do not own depdent nature the capabilty can focus
    on it's single responsibility
  * When testing software capabilities, I can inject mock or stubbed behavior to
    isolate my testing to the capability I care about, and I can do this much faster
  * When I want to test a more integrated system, minus hardware, I can do so very
    quickly and easily and test the real software stack.
* What would take a developer two months to write, test, and verify software can
  be accomplished in a month when software systems use dependency injection
  * Remeber: Software systems have more than just external dependencies; they
    also have interal dependencies
  * By inverting control of dependent nature, testing time is cut in half.
    As a result, I can write better, more meaningful tests.
  * Because my testing is improved, I am writing more reliable software and
    eliminating bugs that could easily be squashed in software-in-the-loop testing
  * When I test my software on hardware, I am spending less time debugging my
    software capabilities, and spending more time debugging the software/hardware
    interfaces
  * All of these gains add to less time spent during testing and verification
  * Additionally, DI scales really well
  * When the codebase requires changes, these well defined interfaces and inversion
    of control allow developers to only change one or two specific capabilities
    rather than the entire software system

## Trades: What's the Catch?
* There is no free lunch
* The Abstraction triangle is at play, you can only pick two:
  * Easy to understand and maintainable code
  * Static behavior
  * Well defined and isolated interfaces
* DI sacrifices static behavior for easy to maintain and understandable code
  * In other words, I am defining dependencies at runtime
  * I am trading static behavior for scalable software that is easier
    to understand vs if this was implemented at compile time
    * Note: There are methods to achieve DI at compile time
    * These methods are very complex, convulated, and difficult to maintain
      (trust me, I've tried)
* DI can also be slightly more difficult to debug
  * New developers can't clearly see which implementation is being utilized
    at first glance.
  * Luckily this problem is only a start-up cost
  * As developers become more familiar with the code-base they get better at debugging
  * The software is inherintly easier to maintain so it becomes easier to fix bugs
  * If developers know where to look for the implementation definition,
    they have an easier time debugging
* DI and abstraction has a quantifiable runtime performance cost
  * When implemented like LibJuno, it's a very tiny cost compared
    to the ease of maintaining the software

## Does Using LibJuno Gurantee Cost Savings?
* Sadly there is no magic bullet
* LibJuno is written so capabilities that depend on her
  can inject her at runtime
* Developers will always be able to write unmaintainable software
* Half the battle is education
* LibJuno implemenets dependency injection with the hope that 
  users will do so too

## DI is not a Silver Bullet
* LibJuno implements dependency injection with certain compile-time gurantees
  * Implementations are known at compile time and only a single implementation
    can be used at a time (no polymorphism)
  * All interfaces and function pointers are static and constant. They can not be 
    modified at runtime
* That being said, a function pointer has a level of indirection that cannot be avoided
* DI is not the correct solution for every capability.
* For example, if you have a motor controller that operates at 10KHz, you probably should implement the core capability directly.
  * However, the motor controller could provide an API so higher level code can easily inject that capability without a perfromance cost
* DI is not the right solution for everything, but neither is tightly coupled software dependencies.
  * Some components need to own their dependencies. If that's the case, the software should do so. However, that software can provide an API so others could inject it as a dependency.
* Many embedded developers are unfamiliar with DI
  * In that case, DI can be used sparingly and integrated over time
  * There are strageic ways to implement new design patterns without throwing the old 
    one out the window
  * It's rare that software systems are all or nothing
  * A DI paradigm has successfully been implemented alongside non-DI code on other 
    projects


## Conclusion
* Tightly coupled code is excellent for small projects (approx. >5KSLOC)
* As a codebase scales in size, tightly coupled software is timeconsuming to maintain
* Inverting the Control of dependencies and injecting them at runtime solves
  this problem
* DI scales with codebase size and complexity
* It could potentially save a project millions of dollars worth of development time
