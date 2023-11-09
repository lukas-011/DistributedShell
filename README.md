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