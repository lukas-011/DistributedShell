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
       - (put what it does here)
     - list
       - (put what it does here)
     - delete
       - (put what it does here)

2. m_cp
** This WILL have 2 arguments **
- local and dest

- This will copy a file

- it copies a local file using the name provided in the dest (destination).
  the copy is made to the filesystem server and the number of partitions is equal 
  to the number of agents that were created.

3. m_run