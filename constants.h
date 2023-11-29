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
#define STR_FAILSTART "Failed to run \"%s\"\n"
#define STR_MAGENT_SYNTAX "Error: Syntax is m_agent <create|list|delete>\n"\
" - m_agent create <ip address> <port>\n"                                  \
" - m_agent list\n"                                                        \
" - m_agent delete <ip>\n"
#define STR_MAGENT_CREATE_SYNTAX "Error: Syntax is m_agent create <ip address> <port>\n"\
"Example: m_agent create 192.168.1.200 8000\n"
#define STR_MAGENT_CREATE_SUCCESS "New agent created!\n"\
"IP: %s\n"\
"Port: %s\n"
#define STR_MAGENT_LIST_NUMBER "Listed %d agent(s)\n"
#define STR_MAGENT_LIST_NONE "No agents exist\n"
#define STR_MAGENT_LIST "Agent %d:\n"\
"   - IP Address: %s\n"\
"   - Port: %s\n"\
"------------\n"
#define STR_MAGENT_DELETE_SYNTAX "Error: Syntax is m_agent delete <ip address>\n"\
"Example: m_agent delete 192.168.1.200\n"
#define STR_DELETE_NUMBER "Deleted %d agent(s)\n"

#endif //DISTRIBUTEDSHELL_CONSTANTS_H