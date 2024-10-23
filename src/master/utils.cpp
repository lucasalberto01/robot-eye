#include "./utils.h"

// Helper function to extract integer values from command string
std::pair<int, int> getIntValues(const char* cmd) {
    char* cmdCopy = strdup(cmd);
    int val1 = atoi(strtok(cmdCopy, "#"));
    int val2 = atoi(strtok(NULL, "#"));
    free(cmdCopy);
    return {val1, val2};
}