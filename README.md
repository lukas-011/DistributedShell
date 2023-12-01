# DistributedShell
A distributed shell

## The Plan

### Methods

- startProc: Starts an external process using fork and execve
- doCommand: Determine which command to run
  - createAgent: Called with `m_agent create ip port`
  - listAgent: Called with `m_agent list`
  - deleteAgent: Called with `m_agent delete ip`
  - copyFile: Called with `m_cp local dest`
  - runParellelProg: Called with `m_run mainProg parellelProg`

# Methodology
So we have an input from the user

we are going to parse the input out with a function called seperateArguments

base off the first parameter, we are going to do the command

## we have different commands
we are going to have a "decision tree" for each first argument

1. m_agent
   - m_agent can have different commands
     - create
       - Takes in an IP and a Port
       - Stores a reference of an agent that can receive a parallel program for 
         execution. The IP is where the agent is localed and the port is what port
         it is listening on. 
       - You can assume a max of 32 agents that are supported
     - list
       - Lists all configured agents
     - delete
       - Takes in an IP
       - Deletes an agent based on IP

2. m_cp
- Takes in 2 arguments
 - local and dest

- This will copy a file

- it copies a local file using the name provided in the dest (destination).
  the copy is made to the filesystem server and the number of partitions is equal 
  to the number of agents that were created.

3. m_run
- has 2 arguments
- mainProg and parallelProg
  - The main program is ran locally
  - parallel program is sent to each agent that exists


### Then we have to run the parallel program
- To run a program, we need to have an agent which should also be created as a seperate
  program (Not the same as the distributed shell)
## below is the API that the agent will adopt

# we will have 2 commands

1. transfer parallelProg contents_of_parallel_program
- The agent will receive the contents of the parallel program, store it on the filesystem,
  and compiles it

2. run parallelProg n
- The agent will run the program using the number n as its single argument. Any output of the 
  compiled program will be returned to the distributed shell