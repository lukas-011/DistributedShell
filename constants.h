//
// Created by PJ, Lukas, Connor, and Anthony
//

#ifndef DISTRIBUTEDSHELL_CONSTANTS_H
#define DISTRIBUTEDSHELL_CONSTANTS_H

// Commands
#define STR_M_AGENT "m_agent"
#define STR_M_CP "m_cp"
#define STR_M_RUN "m_run"
#define STR_EXIT "exit"

// Parameters
#define STR_CREATE "create"
#define STR_LIST "list"
#define STR_DELETE "delete"

// Environmental variables
#define STR_PATH "PATH"

// Long strings
#define STR_GREETING "Welcome to the Distributed Shell!\n"
#define STR_FAILSTART "Failed to start \"%s\"\n"
#define STR_MAGENTSYNTAX "Error: Syntax is m_agent create <ip address> <port>\n"\
"Example: m_agent create 192.168.1.200 8000\n"
#endif //DISTRIBUTEDSHELL_CONSTANTS_H
