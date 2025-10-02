/**DOC
# Dependency Injection

In embedded systems, code often grows into tightly coupled,
monolithic blocks where a change in one component ripples
through the entire system. This makes maintenance and reuse
difficult, especially as projects scale or requirements evolve.
**LibJuno** offers a lightweight, C99-compatible approach to
achieve modularity and dependency injection (DI) in embedded
projects. By using LibJuno’s macros and a clear pattern for
defining “modules” (essentially self-contained components with
defined APIs), you can:

* **Decouple** implementation details from consumers.
* **Enforce** clear contracts (via function pointers in an API
  struct).
* **Inject** dependencies at initialization time instead of
  hard-coding them.
* **Isolate** changes so that swapping one implementation doesn’t
  force you to rework unrelated code.

In this tutorial, we’ll walk through:

1. The basics of LibJuno’s module-system macros.
2. How to define a module API (header) and its implementation.
3. How to compose multiple modules with dependency injection.
4. A concrete example: building a “Car” that depends on an “Engine”, which in turn depends on either a gas tank or a battery.
5. Variations that demonstrate isolated changes (e.g., swapping a V6 engine for a V8, or adding a new turbocharged engine) without affecting consumers.

We’ll use the example code provided in the “gastank\_api.h”, “engine\_api.h”, “battery\_api.h”, “car\_api.h” files, plus their implementations, to illustrate these concepts. By the end, you’ll see how a modular, DI-driven design can keep your embedded software clean, testable, and maintainable.
*/

int main(void)
{
    return 0;
}
