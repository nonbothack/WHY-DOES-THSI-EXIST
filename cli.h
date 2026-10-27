#ifndef CLI_H
#define CLI_H

#include "contact.h"
#include "storage.h"

/**
 * Runs the interactive CLI terminal menu loop.
 * Enables live terminal demonstrations.
 */
void runCliMenu(Contact **head, int *nextId, const char *storageFile);

#endif /* CLI_H */
