#ifndef _DEBUG_MEMDUMP_H
#define _DEBUG_MEMDUMP_H
/**
 * @brief Dump memory in hex format
 *
 * @param desc description of the memory
 * @param addr address of the memory
 * @param len length of the memory
 */
void hexDump(char *desc, void *addr, int len);
#endif // _DEBUG_MEMDUMP_H
