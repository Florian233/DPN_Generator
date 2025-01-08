# DPN Generator

This generator can generate Dataflow Process Networks (dynamic and static). The input and output actors are hardcoded and therefore not generated.
For other actors randomly in a certain range certain parameters are determined, e.g. number of actions and their number of instructions, number of input and output ports of actors
and their token rates. The token rates of two connected ports always match in order to guarantee a working network since the network has no meaningful semantics.
The maximum number of instructions per action, number of input and output actor instances, and number of overall actor instances can be defined by the user through command line parameters.
If the user requests the generation of a dynamic dataflow network, actors still consume/generate the fixed rates of tokens but might do this split up into multiple actions, all perform
a part of it. The overall token flow dynamic is introduced by fork and join actor pairs with dataflow networks in-between, possibly also containing forks and joins.
The fork and join actors are hardcoded and are therefore not subject to randomness.

The generator can generated networks with feedback loops, actors with FSMs, priorities and state variables that are used for scheduling.
The possibilities for the certain elements of the actors and the network topology are hardcoded at the moment and cannot be configured.

## Command Line Parameters

* -w <directory>     Specify the directory to write the output to. Must be speficied otherwise the generation terminates immediately.
* -m <number>        Maximum number of instructions per actor, default 100.
* -n <number>        Maximum number of actors, default 50.
* -i <number>        Number of Input nodes, default 5
* -o <number>        Number of Output nodes, default 5.
* -f                 Allows feedback loops.
* -d                 Allow dynamic nodes. Doesn't include dynamic token flow through different paths, only dynamic token rates for actors.
* -dd                Allow dynamic token flow through parallel paths by generating fork-join pairs.
* -c <complexity>    Complexity of the generated actors, simple|if|loop, default if. Simple only generates basic intructions, if inclues simple. Loop can generated foreach loops, but they are not nested. Loop also includes if.
* -z                 Disable FSM generation.
* -u                 Disable priority generation.
* -q                 Disable State Variables.
* -t <num>           Maximal tokenrate.
* -l <num>           Maximal number of feedback cycles - not necessarily the exact number of generated cycles, as this influence by randomness.
* -p <num>           Maximal number of ports of the actors.