//
// Created by lukas_raider on 12/2/23.
//

#ifndef DISTRIBUTEDSHELL_BASE64_H
#define DISTRIBUTEDSHELL_BASE64_H

int Base64decode_len(const char *bufcoded);
int Base64decode(char *bufplain, const char *bufcoded);
int Base64encode_len(int len);
int Base64encode(char *encoded, const char *string, int len);

#endif //DISTRIBUTEDSHELL_BASE64_H
