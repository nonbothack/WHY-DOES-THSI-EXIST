#ifndef SERVER_H
#define SERVER_H

#include "contact.h"
#include "storage.h"

/**
 * Starts the C HTTP server on the specified port.
 * Serves the frontend static files and handles JSON API requests.
 */
int startServer(int port, Contact **head, int *nextId, const char *storageFile);

#endif /* SERVER_H */
