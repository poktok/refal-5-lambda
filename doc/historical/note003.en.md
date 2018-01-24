> This is translation of historical paper [note003.txt](note003.txt) from Russian.

The purpose of this lab is to add to Simple Refal higher-order functions.
It is assumed for lambda functions to use the syntax, similar to the Refal 7
syntax, with the constraints that (a) sentences always have the form pattern =
result, (b) nameless functions.

# Terminology agreement.
Under the _lambda-function,_ hereinafter we shall mean a functional block,
declared in the result expression. The concept of a function is static, that
exists only at the time of writing the code and compiling the program.

As a _global function_ we will understand the usual function of the Simple Refal
in a way, in which it was presented in previous versions of the language.

The _closure_ will mean an instance of the function (both lambda and global
functions) that exists during the execution of the program and is present in
the view field of the abstract Refal machine. The closure can be performed
(activated) if it is after the opening angle bracket. Closure is a
generalization of the concept of “pointer to a function” of previous versions
of the Simple Refal.

The relationship between a function (global or local) and closure is the same
as the relationship between the class and the object in OOP.

Because (in this version) there is no pattern that could break up the closure
into smaller elements (smaller than the function name in the case of a global
function instance), the closure is an atom and can be mapped to an s-variable.

Initially, for lambda functions, I was going to use the term “local function”,
because The same term was used by Skorobogatov in the article “Refal language
with higher-order functions”, but there is already existing (in the internal
documentation for the Simple and Modular Refal) terminology, according to which
global functions are divided into entry functions (accessible from outside the
module) and local functions ( not accessible from the outside of the module).
In order not to cause terminological confusion, I decided to use the term
“lambda function”.

One way to implement closures that will be implemented in this lab is the
closure representation as a pair (pointer function, context), where the pointer
to a function is a pointer to some global function with a unique name that
accepts the closure argument + context as an argument, the context is a list of
free variables of the lambda function in the terminology of the lambda
calculus, so let us assume that the following function generates a closure:

    CreateClosure {
      s.1 e.2 = {e.3 = e.2 s.1 e.3; };
    }

The closure generated by the `<CreateClosure 'Hello'>` call will be represented
by a pair `(CreateClosure#1, 'H' 'ello')` where the CreateClusure#1 function
is automatically generated and looks like this:

> Misstype in Russian original: must be `(CreateClosure#1, 'H' ('ello'))`,
> (Mazdaywik)

    CreateClosure#1 {
      s.1 (e.2) e.3 = e.2 s.1 e.3;
    }

The closure call `(s.Function e.Context)` with some argument `e.Arg` comes down
to the call to `s.Function` with the argument `e.Context e.Arg`, for the
following example:

    <<CreateClosure 'Hello'> ' World'>

is equivalent of

    <CreateClosure#1 'H' ('ello') ' World'>

the result of which will be `'elloH World'`.

Since in the case of higher-order functions, functions are full-fledged data
objects, closures need to provide the ability to copy. Copy the entire context
when copying an object expression (if the result expression includes more
variables with a given name than in the pattern one), it is undesirable. this is
not only a decrease in the effectiveness of the program, but also a surprise
for the programmer: the copying of atoms must be performed for a constant time
in the list implementation.

Since there are no cyclic connections for this method of specifying lambda
functions (the context of this closure is created before the closure itself is
created and then it cannot be changed ? direct circular references are
excluded, if the context contains other closures, then they must be created
before creating the current one closures and then by induction ? indirect
circular references are forbidden), you can apply the scheme of reference
counting. When you copy an object expression, the context reference count is
incremented by 1, and when you delete the list of free blocks, the reference
counters do not change. The reference count decreases by 1 only when the node
containing the closure is reused to create a new node. When the reference
counter is reduced to zero, the content of the context is reset to the free
block list.

When a closure that contains a context is activated, the context is copied into
the view field and a pointer to the function is invoked. However, if the
context is used only by this closure instance, then the context can be moved
rather than copied into the view field.

The following implementation method is assumed. Since in the Simple Refal the
parsing is done rather irregularly (at first the automaton highlights the
top-level declarations, as well as the samples and the results of each
sentence, without taking into account the pairing of the brackets and the
correctness of the declaration of the variables, then the brackets pairings are
checked separately for each sentence action, then the correctness of the use of
the variables is checked; only after this is generated the result code in C
++), then we have two ways:

1. Implement a regular syntactical analyzer (similar to Modular Refal), i.e.
   write a syntactical analyzer using recursive descent with a parallel
   analysis of the correctness of the use of variables (which is fundamentally
   important for working with context). A part of such a regular analyzer is
   written: it is an automaton processing declarations and roughing functions.
   It will only be necessary to rewrite the processing of sentences and change
   the order of generation of the final code in C ++.
2. Irregularly add support for lambda functions. One way is to add one more
   pass in the lexical convolution with checking the balancing of the curly
   brackets, while the curly brackets are wrapped in a pseudo lexeme. In this
   case, the machine for analyzing the upper-level constructions and the rough
   analysis of the body of the function will not practically change. In this
   case, the work can be divided into two stages, the first simple, the second
   more difficult:
   1. If a lambda lexeme is found in the resulting expression, create a unique
      name for the global function; in place of the lambda lexeme, put the
      operation of binding the global function with this unique name to the
      context (which at this stage will contain ALL variables from the sample
      sentence); The unique name, the contents of the lambda-token (the
      contents of the braces) and the context to be stored (in the form of
      transferring the next term from the function to the function); then, upon
      completion of the generation of a specific function, proceed to the
      processing of the lambda function, treating it as an ordinary function,
      all the samples of which begin with arguments corresponding to the
      context.
   2. At the heart of the idea is about the same, only in the context should be
      sent really used in lambda functions variables.

      The advantage of dividing the process into two stages is that in the
      second stage will be ready (though a curve) to support higher-order
      functions, which will dramatically increase the expressiveness of the
      Simple Refal and realize the second stage will be easier. How to
      implement the second stage ? I have not thought through to the end, but I
      suspect that with any implementation of the second stage, we cannot
      immediately send the generator individual proposals, but we need to store
      the entire function in memory (at the first stage, the stored lambda
      functions are still not parsed stream of lexemes inside braces).  It is
      possible to implement the second stage as the writing of a regular
      analyzer (see point 1).

# Results

## [1] Changes made to the compiler

Simple Refal replenished with lambda functions in accordance with the goal of
the lab. In addition, an error has been fixed that is related to the
compilation problems of the g ++ compiler in interpretation mode. The error was
that pointers to a function and pointers to string literals cannot
automatically be converted to `(void *)` (the former cannot in principle, the
latter can only be cast to `(const void *)`). An explicit cast of types in the
C-style was added. In addition, the array of commands became a static constant,
instead of the `alloc_ptr1` / `alloc_ptr2` pointers, an allox array
`[2 * sizeof (raa) / sizeof (raa [0])]` was introduced. The size of the array
is allocated with a margin (because for each command, no more than two
distributions are required, and some do not need distributions at all). Because
the array is automatic and not initialized, there is no additional expenditure
of performance for the reserve, the memory costs are negligible compared to the
memory costs on the view field. This allowed us to compare the size of the
executable file generated in both modes using the g++ compiler. The
compilation was performed with the default optimization parameters (no
additional command line switches were used). The size of the file generated in
both modes exceeds the size of the executable file generated using BCC 5.5 in
the same mode.

## [2] Ideas for using lambda functions

It can be assumed that adding lambda functions to the Refal dialect, the
control structures of which largely coincide with the subset of the Basic Refal
(the current version of the Modular Refal ? 0.1.959, Simple Refal of previous
versions), will lead to a sharp increase in the expressiveness of the language
and the development of new idioms.

One of the main problems of the subset of the Basic Refal is that you have to
write a whole lot of intermediate functions (the second problem is the absence
of rollbacks). Intermediate functions are used to analyze the result of a
function call and, depending on this, perform certain actions (I write such
functions with the prefix `Sw`); to convert the result of calling one function
in one format to another format with the purpose of returning or passing to
another function (I write such functions with the suffix `-Aux`); execution of
the tail recursive cycle (I write such functions with the prefix `Do`). If
a function has both a sense prefix and a suffix `-Aux`, then priority is
defined as a rule in meaning.

In addition, the use of higher-order library functions used to hide the loop
(`Map`, `Reduce`, `MapReduce`) requires specifying a function that needs to be
declared global.

In all these cases, lambda functions can help.

## 1. Branching, depending on the result of the function.

instead

    Func {
      argument = <SwFunc (some context) <OtherFunc some argument >>;
    }

    SwFunc {
      (context) result-1 = action-1;
      (context) result-2 = action-2;
      ....
    }

write

    Func {
      argument =
        <
          {
            result-1 = action-1;
            result-2 = action-2;
            ....
          }
          <OtherFunc some argument>
        >;
    }

The context of the function block contains the argument of the function `Func`,
so its explicit transfer is not required.

In many dialects of Refal for this purpose there exists such a construction as
an action-block (with-clause Refal 5, model block in Refal 6 and Refal Plus
(relative to the latter is not sure of terminology), operation “arrow” (fetch)
in Refal 7. The syntax of such a construction in Refal 7 would look like

    Func {
      argument =
        <OtherFunc some argument> => {
          result-1 = action-1;
          result-2 = action-2;
          ....
        };};
    }

In Refal 5, 6 and Plus, instead of the arrow, you should use a colon (in
Refal 6 you can use quadratic).

Such a syntax can be simulated as follows ? enter the function `Fetch`,
defined (in some module of the library) as:

    Fetch {
      e.Argument s.Function = <s.Function e.Argument>;
    }

Using this function, the last example will look like

    Func {
      argument =
        <Fetch
          <OtherFunc some argument>
          {
            result-1 = action-1;
            result-2 = action-2;
          }
        >;
    }

## 2. Function Format Conversion

Consider only the transformation of the function's format in order to call
the second function having a different format. The transformation of the
return value is treated similarly.

Instead

    Func {
      argument =
        <Func-Aux
          (some context) <OtherFunc some argument>;
        >;
    }

    Func-Aux {
      (context) data in the format 1 =
        <OtherFunc2 context parts and data in format 2>;
    }

Write

    Func {
      argument =
        <
          {
            data in the format 1 =
              <OtherFunc2 context parts and data in format 2>;
          }
          <OtherFunc some argument>
        >;
    }

In developed dialects of Refal (the same Refal 5, 6, 7, Plus) for this
purpose also there is a special syntax (using the where-clause in Refal 5
for rebuilding (its main purpose is checking the conditions), the actual
rebuilding in Refal Plus, the double-colon or the colon in Refal 6 and 7
(in Refal 7 it is necessary to handle the double-colon more cautiously ? it
prohibits failures in all the end of the sentence, following it, in contrast
to Refal 6, where it prohibits failure only in the following exemplary
action, but is transparent for the remaining part of the sentence)). Example
on Refal Plus:

    Func
      argument,
        <OtherFunc some argument> :: format 1 =
        <OtherFunc2 context parts and data in format 2>;

It intentionally says “`format 1`”, and not “`data in format 1`”, because after
a double-colon can only be a so-called. “hard expression”, i.e. expression
without literals, repeated variables, and open e-variables. Those. actually
format.

This syntax can be emulated using the same Fetch function:


    Func {
      argument =
        <Fetch
          <OtherFunc some argument> {
            data in the format 1 =
              <OtherFunc2 context parts and data in format 2>
          }
        >;
    }

## 3. Perform tail recursive cycles.

Here we will consider both an abstract and a real example. Let look at an
abstract example:

    Func {
      argument = <DoFunc (context) initial loop invariant>;
    }

    DoFunc {
      (context) variety of the invariant 1 =
        <DoFunc (context) change of the invariant 1>;

      (context) variety of the invariant 2 =
        return value 2;

      ....
    }

Here, the function `DoFunc` is tail recursive, the function `Func` prepares a
format for it (for example, initialization of accumulators) and prepares data
that will be rotated in a cycle (including by calling other functions). The
first sentence, with one of the varieties of the invariant, continues the
cycle, invoking itself with the modified invariant (including and with the help
of calls to other functions); The second sentence completes the loop, replacing
with the result that does not contain a recursive call. Let's illustrate this
with a real example.

    Factorial {
      0 = 1;
      s.N = <DoFact s.N 1 1>; // context, battery, cycle counter
    }

    DoFact {
      s.N. s.Prod s.N = <Mul s.Prod s.N>;
      s.N s.Prod s.K =
        <DoFact s.N <Mul s.Prod s.K> <Inc s.K >>;
    }

> *Translation to English of this hunk of historical paper is prepared by*
> **Appolonov Ilia <appolon4iks@gmail.com>** _at 2018-01-18_

It is clear that it is easy (and maybe even more effective) to realize the
factorial without constant rotation of `s.N` in the cycle. Such an algorithm
was chosen solely for demonstrating the transfer of an immutable context.

Then an interesting problem arises for a recursive call, we need to call a
function with the same name as the name of the function within which the call
is made (ie, inside the function called `DoFact`, we need to call a function
`DoFact`). So far we have considered cases in which anonymous lambda functions
substituted for the name of the global intermediate (`Sw…` or `…-Aux`)
functions and everything turned out quite smoothly. And now we need to make a
recursive call to an unnamed lambda function.

Before to come to a clue, we shall consider the same examples on Refal 7, in
which there are no similar problems, since lambda functions can be named. Other
dialects of Refal (5, 6, Plus) are not considered here; in them there are no
lambda functions, and even more so named, and in them residual recursive cycles
of the is implemented as shown above.

An abstract example

    Func {
      argument =
        initial cycle invariant =>
        $func Loop {
          Variety of invariant 1 = <Loop change of invariant 1>;
          Variety of invariant 2 = return result 2;
          ....
        };
    }

Specific example

    Factorial {
      0 = 1;

      s.N = 1 1 =>
        $func Loop {
          s.Prod s.N = Mul s.Prod s.N;
          s.Prod s.K = Loop Mul s.Prod s.K Inc s.K;
        };
    }

Since the scope of the name of a named lambda function is not wider than the
name itself function, then for all such cycles you can use the same name -
`Loop`. The use of the function name with the `Do` prefix is only necessary to
distinguish intermediate functions belonging to different functions. In the
case of nested loops you will have to choose names other than `Loop`.

And now the solution. The point is that the local function can be copied, and
then when you call, pass it to yourself as an additional parameter. Thus, we
arrive at the solution

    // library function
    Loop {
      s.Function e.Argument =
        <s.Function s.Function e.Argument>;
    }

    Func {
      argument =
        <Loop
          {
            s.Me variety of invariant 1 = <Loop s.Me measurement of invariant 1>;
            s.Me variety of the invariant 2 = return result 2;
            ....
          }
          initial cycle invariant
        >;
    }

Here the auxiliary lambda function first argument of the term should always
take the `s.Me` atom, a recursive call is performed by calling the library
function Loop with the first argument `s.Me`. In the parameter s.Me, the
closure corresponding to the lambda function is placed. You can write a
combined function `Fetch-L`, combining the action of `Fetch` and `Loop`,
namely, the first indication of the argument, and then the functions and the
start of the cycle, respectively.

    Fetch-L {
      e.Argument s.Function = <Loop s.Function e.Argument>;
    }

The example with the factorial will look like this

    Fact {
      0 = 1;

      s.N =
        <Fetch-L
          1 1
          {
            s.Me s.Prod s.N = <Mul s.Prod s.N>;
            s.Me s.Prod s.K = <Loop s.Me <Mul s.Prod s.K> <Inc s.K>>;
          }
        >;
    }

The disadvantages of this solution are that (a) we must not forget about the
argument `s.Me`, (b) you can not forget to call the Loop function with the
argument `s.Me`.

The defect (a) is in principle insubstantial, but it can be eliminated as
follows heavyweight way by introducing an additional level of indirection. For
example, so

    Fact {
      0 = 1;

      s.N =
        <Fetch-L
          1 1
          {
            s.Me =
              <{
                s.Prod s.N = <Mul s.Prod s.N>;
                s.Prod s.K = <Loop s.Me <Mul s.Prod s.K> <Inc s.K>>;
              }>;
          }
        >;
    }

or even so

    Loop-i {  // improved
      s.Function e.Argument = <<s.Func s.Func> e.Argument>;
    }

    Fetch-Li {
      e.Argument s.Function = <<Loop-i s.Function> e.Argument>;
    }

    Fact {
      0 = 1;

      s.N =
        <Fetch-Li
          1 1 {
            s.Me = {
              s.Prod s.N = <Mul s.Prod s.N>;
              s.Prod s.K = <Loop-i s.Me <Mul s.Prod s.K> <Inc s.K>>;
            };};
          }
        >;
    }

The second way to deal with the defect (a) is better, because (1) uses a more
compact record and (2) demonstrates the power of using lambda functions.

Another solution is the lambda calculus – the use of a combinator Y. The
combinator Y can be implemented on the Simple Refal

    Y {
      s.Func = {e.Arg = <s.Func <Y s.Func> e.Arg; };
    }

The combiner accepts an auxiliary function closure at the input and returns
Another closure that transmits its argument and itself to the closure of the
auxiliary function. Let us explain with an example.

    Fact {
      0 = 1;

      s.N =
        <Y {
          s.Loop s.Prod s.N = <Mul s.Prod s.N>;
          s.Loop s.Prod s.K = <s.Loop <Mul s.Prod s.K> <Inc s.K>>;
        }
          1 1
        >;
    }

This solution does not lack (b) – in the recursive call it is not required to
throw any service information, but to implement it with an immediate argument.
You can eliminate and the drawback (a) in the same way as for the previous one
method.

    Y-i {
      s.Func = { e.Arg = <<s.Func <Y-i s.Func>> e.Arg>; };
    }

    Fetch-Y {
      e.Argument s.Function = <<Y-i s.Function> e.Argument>;
    }

    Fact {
      0 = 1;

      s.N =
        <Fetch-Y
          11 {
            s.Loop = {
              s.Prod s.N = <Mul s.Prod s.N>;
              s.Prod s.K = <s.Loop <s.Prod s.K> <Inc s.K>>;
            };
          }
        >;
    }

Compare the last example with the code sample on Refal 7

    Factorial {
      0 = 1;

      s.N = 1 1 =>
        $func Loop {
          s.Prod s.N = <Mul s.Prod s.N>;
          s.Prod s.K = <Loop <Mul s.Prod s.K> <Inc s.K>>;
        };};
    }

The construction with `Fetch-Y` can be considered the start of a loop, the
sentence `s.Loop = {...}` – by writing the named function `$func Loop {...}`,
and a persistent combination of the record all this ugliness as an idiomatic
way of writing a named lambda function, just as the loop
`for(i = 0; i < N; ++i)` is quite naturally perceived as a loop with a counter
`for i = 0 to N-1 do`, for c language is not.

The considered techniques are designed for simple recursion. Reciprocal
recursion with several named lambda functions, as it is possible in Refal 7,
will be much more difficult to implement by the means used, but it is possible.

## 4. Use of library functions of higher orders.

Here everything is simple – in the call of a similar function instead of the
global function name write directly the body of the function-converter. If the
latter is recursive, then we use the idiomatic construction
`Y-i {s.Loop = {body};`.

In addition, lambda functions allow you to implement OOP tools in the style of
duck typing (“if something looks like a duck and behaves like a duck, it's a
duck”). For this, an object constructor function is created that returns a
closure whose context is an object with some invariant. The return closure
takes the name of the method and returns a closure that implements the
corresponding method.

In this case, the selector methods return the parameter of interest, modifier
methods return the result object of the modification by calling the constructor
again. If the selectors do not accept parameters, then you can enable Instead
of returning the closure after specifying the name of the selector, return
immediately the value of the parameter.

Let's illustrate this by implementing a class of geometric shapes that can
move, return their area, center and dimensions.

    Circle {
      s.Radius s.X s.Y =
        {
          Move = {
            s.OffX, s.OffY =
              <Circle s.Radius <Add s.X s.OffX> <Add s.Y s.OffY>>;
          };

          Area = <Mul <Pi> s.Radius s.Radius>;
          Center = s.X s.Y;
          Size = <Mul 2 s.Radius> <Mul 2 s.Radius>;
        };
    }

    Rectangle {
      s.Width s.Height s.Left s.Top =
         {
           Move = {
             s.OffX s.OffY =
               <Rectangle
                 s.Width s.Height <Add s.Left s.OffX> <Add s.Top s.OffY>
               >;
           };

           Area = <Mul s.Width s.Height>;
           Center =
             <Map
               { s.X = <Div s.X 2>; } <Add s.Width s.Left> <Add s.Height s.Top>
             >;
           Size = s.Width s.Height;
         };
    }


Calling the shape modifier looks like `<<s.Shape Move 320 240>`, calling the
selector is like `<s.Shape Area>`. The advantage of such an implementation of
abstract types data is the fact that they are effectively copied as closures,
unlike in from the standard means of using abstract data types (square
brackets). The disadvantage is that when you embed static typing for regular
ADTs to control the correctness of types is easier than for such closures.

## [3] Implementation of runtime

###  1. Calling Closures in a Virtual Machine

As already mentioned in the goal of the lab, the closure is a pair (global
function, related context). Because the context is simple enumeration of free
variables of a function block (in this case, e-variables are enclosed in
brackets) and it is located to the left of the closing argument, then the
closure can be simply stored as the object expression `s.Function e.Context`.
When exposed, the virtual machine closes after the opening function call, a
virtual machine can extract this expression from the `s.Function e.Context` and
substitute its in place of the node of closure. In this implementation, this is
exactly the case, when this requires one step of the refal machine to open the
closure. Thus, the closure call is performed in two steps.

### 2. Representation of closures in the list implementation

To represent closures in the view, 3 new nodes: closure, the head of the
closure and the node of the unfolded closure (about the assignment the latter
will be described later in the section Collapsing and Deploying Closures).
Directly in the field of view is a node of closure, in the field `link_info`
which is the pointer to the node-head of the closure. Actually, the very
closure of an object expression, consisting of a pointer to a global function
and context, is tied to the head node whose fields next and prev indicate
respectively the beginning and the end of the given expression, thus forming a
cyclic two-linked list. The information field of the head of the closure
consists of a counter references to this closure.

When copying an object expression, only the closure node is copied, in the
`link_info` the address of an existing head is placed, thus the counter of
links in the last increases. Thus, copying the closure atom is performed in
constant time.

The reference count decreases in two cases when the object expression
containing the closure and placed in the free block list is used repeatedly and
the closure node must be returned for re-use or when this closure is performed.
When the reference counter is zeroed, the contents of the closure (pointer on
function and context) is placed in the list of free blocks. When a closure is
called with a count value greater than one, the contents of the closure are
copied to view field, at a value of one, the contents of the closure are moved
to line of sight.

Thus, even if the closure occurs in the view field once, the counter Links of
this closure can be more than one, because there may be non-destroyed instances
in the list of free blocks.

Comparison of two closures as the s-variables of the same name is performed
over the field `link_info`, which provides the expected semantics for matching
`s.X s.X : s.Y s.Y`, which always ends successfully. Abstracting from the
implementation, semantics can somehow be formulated as “Comparisons of closures
all copies of a given closure (obtained by exceeding the number of occurrences
of the variable in the result expression of the number of occurrences in the
pattern expression, and by copying during transmission through the context of
the lambda function) are considered identical in terms of comparison with the
help of the variables of the same name.” Moreover, the converse is not true
(see below).

In this implementation, there is optimization: if the lambda function does not
use the context of the environment (i.e., does not have free variables),
instead of triggering the closure, a pointer to the global function with a
unique name, which allows you to save the steps of the refal machine as
creation (see below), and on the execution (see above) of the closure.
Therefore, two closures generated at different instants of time can be
considered identical in terms of comparing the same variables.

In Refal 7, anonymous lambda functions are considered to be unequal a priori,
therefore construct of the form

    ..., s.X s.X s.Y s.Y, ...

can generate failure (if I correctly understood Skorobogatov).

### 3. Folding and unfolding of closures

Storing the contents of closures in separate cyclic doubly linked lists
separately from the main double-linked lists (the view field and the list of
free blocks) provides an opportunity for effective copying and comparison of
identity, but at the same time has one drawback – impossibility without
Recursive calls consistently overlook the entire view field (or the entire list
free blocks). The classical implementation of an abstract refal machine on
lists It is iterative and does not use recursive calls (recursive calls
functions provides the structure of the view field), which is an advantage -
there are no stack overflow errors (but at the same time memory overflow errors
are possible).

However, sequential scanning is necessary, first of all, it is the dump output
of the memory as well as output of expressions in the library (`WriteLile` or
`FWriteLine` of the `Library` library). In addition, sequential viewing
simplifies the release of at the end of the program.

The problem is solved by the introduction of clamping tools for folding and
unfolding closures. Unfolding consists in including the contents of the
closure in the main list, Thinking – in restoring the collapsed closure from
the unfolded. In this implementation, the roll-up operations are focused on
viewing the view field from left to right (in the direction of the next link).

When unfolded, the head node and the following list of the contents of the
closure are broken and placed in front of the closure node. In this case, the
closure changes its type from the usual to the expanded (from `cDataClosure` to
`cDataUnwrappedClosure`), keeping a link to the head in `link_info`. Unfolding
feature (`unwrap_closure`) takes an iterator (a pointer to the node) of the
closure and returns the iterator of the head.

When the folding is performed, the reverse action is restored to the ring list
and returns the type of closure to the node of the unfolded closure. Similarly,
the `wrap_closure` function takes an iterator of an unfolded closure and returns
an iterator next to the closure of the node.

These functions are applied as follows. At the moment when we see a closure in
a sequential scan from left to right, we unfold it and continue viewing from
the head. As a result, we first look at the head, then the contents of the
closure and stumble upon the node of the unfolded closure, which and fold. In
this implementation of the memory dump and the output of expressions by the
library, when a closure is detected, `{` is output and the closure is turned
off, the head is displayed as `[X]`, where `X` is the reference count value, if
detected a closed closure is output `}` and the closure is collapsed.

In addition, in this implementation of the Simple and Modular Refal, consumed
by the program, can only grow, which is undesirable. Further It is intended to
allocate memory from the pool and periodically to release unnecessary memory to
copy the view field, distributing it from the new pool, and the old one after
the copy is released. The possibility of linearizing such shared constructions,
(in this way not only the contents of closures, but also the contents of the
structur (and ATD-) brackets) will allow you to copy the view field using the
line view. Such cleaning can be performed on a counter (when performing N
machine steps) or by the volume of the view field and the list of free blocks
(you need to be able to track the size of expressions).

### 4. Occurrence of closures

When generating the result expression, the lambda functions are replaced with
operations generation of closures that create from a pointer to a global
function and context closure. The name of the global functions that implement
the closure is created by the following rule.

1. The closing curly braces of blocks inside the function receive consecutive
   numbers starting from zero.
2. The global function name corresponding to this block is created in the form
   `lambda_FuncName_N`, where `FuncName` is the name of the function in which
   the closure is located, N is the number of the closing bracket of the block.

Because of the syntax rules, all names (functions and identifiers) must start
with a capital letter, functions beginning with `lambda_` are guaranteed do not
match with the names of the functions defined by the user. Functions are
declared with a static modifier, so they will not cause collisions with
function names in other modules.

Because the objects from which the closure is built are the usual object
expressions (a pointer to the global function and the actual context), it was
it was decided not to create an additional operation to generate closures, but
to use for this purpose a call to the built-in function that creates content
closures proper closure. This function is included in the runtime and has a
name `refalrts::create_closure` and has a signature (a collection of arguments
and a return value) is the same as any other refal-functions. The function and
returns the constructed closure. Because has a special name (with a small
letter, and even in the namespace), it is not available from custom code on
Refal.

Thus, if the function generates closures, each closure requires one additional
step of the refal machine.

As already mentioned above, the compiler includes optimization that allows
Instead of closing without context, return directly the pointer to the global
function corresponding to the block. This refund does not require an additional
step of the refal machine.

# Conclusions.
1. If the parsing is written in regular form by the recursive descent, and not
   the way it was previously written in the Simple Refal, then introduce
   a lambda function in a parser is not difficult.
2. Runtime has undergone relatively simple changes.
3. Lambda functions (even if they are nameless) give a significant increase in
   the language, so they should be included in the Modular Refal immediately
   after the le output version 0.2 (because the syntax changes to version 0.2
   are not planned).

> *Translation to English of this hunk of historical paper is prepared by*
> **Ryzhov Ivan <ivanryzhov322@icloud.com>** _at 2018-01-19_