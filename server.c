#include "server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

static volatile int g_keepRunning = 1;
static int g_serverFd = -1;

static void handleSigint(int sig) {
    (void)sig;
    g_keepRunning = 0;
    if (g_serverFd >= 0) {
        close(g_serverFd);
        g_serverFd = -1;
    }
}

/* URL Decode helper */
static void urlDecode(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a' - 'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if (b >= 'a') b -= 'a' - 'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            *dst++ = 16 * a + b;
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

/* Simple JSON string extractor: finds "key": "value" */
static int extractJsonString(const char *json, const char *key, char *out, size_t outSize) {
    if (!json || !key || !out || outSize == 0) return 0;
    out[0] = '\0';

    char searchKey[64];
    snprintf(searchKey, sizeof(searchKey), "\"%s\"", key);
    const char *pos = strstr(json, searchKey);
    if (!pos) return 0;

    pos += strlen(searchKey);
    while (*pos && (*pos == ' ' || *pos == ':' || *pos == '\t')) pos++;

    if (*pos != '\"') return 0;
    pos++; /* skip open quote */

    size_t idx = 0;
    while (*pos && *pos != '\"' && idx < outSize - 1) {
        if (*pos == '\\' && *(pos + 1)) {
            pos++; /* Skip escape char */
        }
        out[idx++] = *pos++;
    }
    out[idx] = '\0';
    return 1;
}

/* Simple JSON integer extractor: finds "key": 123 */
static int extractJsonInt(const char *json, const char *key, int *out) {
    if (!json || !key || !out) return 0;

    char searchKey[64];
    snprintf(searchKey, sizeof(searchKey), "\"%s\"", key);
    const char *pos = strstr(json, searchKey);
    if (!pos) return 0;

    pos += strlen(searchKey);
    while (*pos && (*pos == ' ' || *pos == ':' || *pos == '\t')) pos++;

    *out = atoi(pos);
    return 1;
}

/* Send HTTP Response */
static void sendHttpResponse(int clientFd, int statusCode, const char *statusText,
                             const char *contentType, const char *body) {
    char header[512];
    int bodyLen = body ? (int)strlen(body) : 0;

    snprintf(header, sizeof(header),
             "HTTP/1.1 %d %s\r\n"
             "Content-Type: %s; charset=utf-8\r\n"
             "Content-Length: %d\r\n"
             "Access-Control-Allow-Origin: *\r\n"
             "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
             "Access-Control-Allow-Headers: Content-Type\r\n"
             "Connection: close\r\n"
             "\r\n",
             statusCode, statusText, contentType, bodyLen);

    write(clientFd, header, strlen(header));
    if (bodyLen > 0) {
        write(clientFd, body, (size_t)bodyLen);
    }
}

/* Serve static file from frontend directory */
static void serveFile(int clientFd, const char *filePath, const char *contentType) {
    FILE *fp = fopen(filePath, "rb");
    if (!fp) {
        sendHttpResponse(clientFd, 404, "Not Found", "text/plain", "File Not Found");
        return;
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buffer = (char*)malloc(fsize + 1);
    if (!buffer) {
        fclose(fp);
        sendHttpResponse(clientFd, 500, "Internal Server Error", "text/plain", "Out of memory");
        return;
    }

    size_t readBytes = fread(buffer, 1, fsize, fp);
    fclose(fp);
    buffer[readBytes] = '\0';

    char header[512];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n"
             "\r\n",
             contentType, fsize);

    write(clientFd, header, strlen(header));
    write(clientFd, buffer, readBytes);
    free(buffer);
}

/* Convert linked list to JSON array string */
static char* listToJson(const Contact *head) {
    size_t capacity = 4096;
    size_t length = 0;
    char *json = (char*)malloc(capacity);
    if (!json) return NULL;

    strcpy(json, "[");
    length = 1;

    const Contact *curr = head;
    int first = 1;

    while (curr != NULL) {
        char item[512];
        snprintf(item, sizeof(item),
                 "%s{\"id\":%d,\"name\":\"%s\",\"phone\":\"%s\",\"email\":\"%s\"}",
                 first ? "" : ",",
                 curr->id,
                 curr->name,
                 curr->phone,
                 curr->email);
        first = 0;

        size_t itemLen = strlen(item);
        if (length + itemLen + 3 >= capacity) {
            capacity *= 2;
            char *newJson = (char*)realloc(json, capacity);
            if (!newJson) {
                free(json);
                return NULL;
            }
            json = newJson;
        }

        strcat(json, item);
        length += itemLen;
        curr = curr->next;
    }

    strcat(json, "]");
    return json;
}

int startServer(int port, Contact **head, int *nextId, const char *storageFile) {
    signal(SIGINT, handleSigint);
    signal(SIGTERM, handleSigint);

    g_serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (g_serverFd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    int opt = 1;
    if (setsockopt(g_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt SO_REUSEADDR failed");
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(g_serverFd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed on port");
        close(g_serverFd);
        g_serverFd = -1;
        return -1;
    }

    if (listen(g_serverFd, 10) < 0) {
        perror("Listen failed");
        close(g_serverFd);
        g_serverFd = -1;
        return -1;
    }

    printf("\n======================================================\n");
    printf(" [C BACKEND] Singly Linked List HTTP Server Active!\n");
    printf(" Access Retro Cyber Terminal at: http://127.0.0.1:%d\n", port);
    printf("                                or http://localhost:%d\n", port);
    printf(" Storage file: %s (Auto-saves changes)\n", storageFile);
    printf(" Press Ctrl+C to stop server cleanly.\n");
    printf("======================================================\n\n");

    while (g_keepRunning) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientFd = accept(g_serverFd, (struct sockaddr *)&clientAddr, &clientLen);

        if (clientFd < 0) {
            if (!g_keepRunning) break;
            continue;
        }

        char buffer[8192];
        ssize_t bytesRead = read(clientFd, buffer, sizeof(buffer) - 1);
        if (bytesRead <= 0) {
            close(clientFd);
            continue;
        }
        buffer[bytesRead] = '\0';

        char method[16] = {0};
        char path[256] = {0};
        sscanf(buffer, "%15s %255s", method, path);

        /* Handle CORS preflight OPTIONS */
        if (strcmp(method, "OPTIONS") == 0) {
            sendHttpResponse(clientFd, 204, "No Content", "text/plain", "");
            close(clientFd);
            continue;
        }

        /* Locate HTTP body (after \r\n\r\n) */
        char *body = strstr(buffer, "\r\n\r\n");
        if (body) {
            body += 4;
        } else {
            body = "";
        }

        int isGetOrHead = (strcmp(method, "GET") == 0 || strcmp(method, "HEAD") == 0);

        /* --- Route Handling --- */

        /* 1. Static Files */
        if (isGetOrHead && (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0)) {
            serveFile(clientFd, "frontend/index.html", "text/html");
        } else if (isGetOrHead && strcmp(path, "/style.css") == 0) {
            serveFile(clientFd, "frontend/style.css", "text/css");
        } else if (isGetOrHead && strcmp(path, "/app.js") == 0) {
            serveFile(clientFd, "frontend/app.js", "application/javascript");
        }

        /* 2. API Status */
        else if (isGetOrHead && strcmp(path, "/api/status") == 0) {
            char response[256];
            snprintf(response, sizeof(response),
                     "{\"status\":\"online\",\"totalContacts\":%d,\"dataStructure\":\"Singly Linked List\",\"algorithm\":\"Dynamic Allocation in C\"}",
                     getContactCount(*head));
            sendHttpResponse(clientFd, 200, "OK", "application/json", response);
        }

        /* 3. Get All Contacts (List traversal) */
        else if (isGetOrHead && strcmp(path, "/api/contacts") == 0) {
            char *json = listToJson(*head);
            if (json) {
                sendHttpResponse(clientFd, 200, "OK", "application/json", json);
                free(json);
            } else {
                sendHttpResponse(clientFd, 500, "Internal Server Error", "application/json", "{\"error\":\"Serialization error\"}");
            }
        }

        /* 4. Search Contacts: /api/contacts/search?q=... */
        else if (isGetOrHead && strncmp(path, "/api/contacts/search", 20) == 0) {
            char *queryParam = strstr(path, "q=");
            char query[128] = {0};
            if (queryParam) {
                queryParam += 2;
                char decoded[128] = {0};
                urlDecode(decoded, queryParam);
                strncpy(query, decoded, sizeof(query) - 1);
            }

            Contact *results[200];
            int count = searchContacts(*head, query, results, 200);

            /* Build JSON array of results */
            size_t cap = 4096;
            char *resp = (char*)malloc(cap);
            if (resp) {
                strcpy(resp, "[");
                for (int i = 0; i < count; i++) {
                    char item[512];
                    snprintf(item, sizeof(item),
                             "%s{\"id\":%d,\"name\":\"%s\",\"phone\":\"%s\",\"email\":\"%s\"}",
                             (i == 0) ? "" : ",",
                             results[i]->id, results[i]->name, results[i]->phone, results[i]->email);
                    if (strlen(resp) + strlen(item) + 2 >= cap) {
                        cap *= 2;
                        resp = (char*)realloc(resp, cap);
                    }
                    strcat(resp, item);
                }
                strcat(resp, "]");
                sendHttpResponse(clientFd, 200, "OK", "application/json", resp);
                free(resp);
            } else {
                sendHttpResponse(clientFd, 500, "Server Error", "application/json", "[]");
            }
        }

        /* 5. Add Contact (POST /api/contacts) */
        else if (strcmp(method, "POST") == 0 && strcmp(path, "/api/contacts") == 0) {
            char name[50] = {0}, phone[20] = {0}, email[60] = {0};
            extractJsonString(body, "name", name, sizeof(name));
            extractJsonString(body, "phone", phone, sizeof(phone));
            extractJsonString(body, "email", email, sizeof(email));

            if (strlen(name) == 0 || strlen(phone) == 0) {
                sendHttpResponse(clientFd, 400, "Bad Request", "application/json",
                                 "{\"success\":false,\"message\":\"Name and Phone are required fields!\"}");
            } else if (isPhoneDuplicate(*head, phone, -1)) {
                sendHttpResponse(clientFd, 409, "Conflict", "application/json",
                                 "{\"success\":false,\"message\":\"A contact with this phone number already exists!\"}");
            } else {
                int assignedId = addContact(head, nextId, name, phone, email);
                if (assignedId > 0) {
                    saveContacts(*head, storageFile);
                    char response[256];
                    snprintf(response, sizeof(response),
                             "{\"success\":true,\"id\":%d,\"message\":\"Contact added to linked list successfully!\"}",
                             assignedId);
                    sendHttpResponse(clientFd, 201, "Created", "application/json", response);
                } else {
                    sendHttpResponse(clientFd, 500, "Internal Server Error", "application/json",
                                     "{\"success\":false,\"message\":\"Failed to allocate node memory.\"}");
                }
            }
        }

        /* 6. Update Contact (POST /api/contacts/update or PUT /api/contacts) */
        else if ((strcmp(method, "POST") == 0 && strcmp(path, "/api/contacts/update") == 0) ||
                 (strcmp(method, "PUT") == 0 && strncmp(path, "/api/contacts", 13) == 0)) {
            int id = 0;
            char name[50] = {0}, phone[20] = {0}, email[60] = {0};
            extractJsonInt(body, "id", &id);
            extractJsonString(body, "name", name, sizeof(name));
            extractJsonString(body, "phone", phone, sizeof(phone));
            extractJsonString(body, "email", email, sizeof(email));

            if (id <= 0) {
                sendHttpResponse(clientFd, 400, "Bad Request", "application/json",
                                 "{\"success\":false,\"message\":\"Valid contact ID is required!\"}");
            } else if (isPhoneDuplicate(*head, phone, id)) {
                sendHttpResponse(clientFd, 409, "Conflict", "application/json",
                                 "{\"success\":false,\"message\":\"Phone number already used by another contact!\"}");
            } else {
                int result = updateContact(*head, id, name, phone, email);
                if (result == 1) {
                    saveContacts(*head, storageFile);
                    sendHttpResponse(clientFd, 200, "OK", "application/json",
                                     "{\"success\":true,\"message\":\"Contact details updated successfully!\"}");
                } else if (result == -1) {
                    sendHttpResponse(clientFd, 404, "Not Found", "application/json",
                                     "{\"success\":false,\"message\":\"Contact with specified ID not found!\"}");
                } else {
                    sendHttpResponse(clientFd, 400, "Bad Request", "application/json",
                                     "{\"success\":false,\"message\":\"Invalid contact data supplied.\"}");
                }
            }
        }

        /* 7. Delete Contact (POST /api/contacts/delete or DELETE /api/contacts) */
        else if ((strcmp(method, "POST") == 0 && strcmp(path, "/api/contacts/delete") == 0) ||
                 (strcmp(method, "DELETE") == 0 && strncmp(path, "/api/contacts", 13) == 0)) {
            int id = 0;
            extractJsonInt(body, "id", &id);

            /* If not in body, check query string e.g. /api/contacts?id=123 */
            if (id <= 0) {
                char *idParam = strstr(path, "id=");
                if (idParam) id = atoi(idParam + 3);
            }

            if (id <= 0) {
                sendHttpResponse(clientFd, 400, "Bad Request", "application/json",
                                 "{\"success\":false,\"message\":\"Valid ID required for deletion.\"}");
            } else {
                int deleted = deleteContact(head, id);
                if (deleted) {
                    saveContacts(*head, storageFile);
                    sendHttpResponse(clientFd, 200, "OK", "application/json",
                                     "{\"success\":true,\"message\":\"Contact deleted and memory freed.\"}");
                } else {
                    sendHttpResponse(clientFd, 404, "Not Found", "application/json",
                                     "{\"success\":false,\"message\":\"Contact not found in linked list.\"}");
                }
            }
        }

        /* 8. Sort Contacts (POST /api/contacts/sort) */
        else if (strcmp(method, "POST") == 0 && strcmp(path, "/api/contacts/sort") == 0) {
            sortContacts(head);
            saveContacts(*head, storageFile);
            sendHttpResponse(clientFd, 200, "OK", "application/json",
                             "{\"success\":true,\"message\":\"Contacts sorted alphabetically in C backend!\"}");
        }

        /* 9. Load Demo Contacts (POST /api/contacts/demo) */
        else if (strcmp(method, "POST") == 0 && strcmp(path, "/api/contacts/demo") == 0) {
            if (*head == NULL) {
                loadDemoContacts(head, nextId);
                saveContacts(*head, storageFile);
                sendHttpResponse(clientFd, 200, "OK", "application/json",
                                 "{\"success\":true,\"message\":\"Sample demo contacts populated!\"}");
            } else {
                sendHttpResponse(clientFd, 200, "OK", "application/json",
                                 "{\"success\":false,\"message\":\"Contacts list already has data. Demo skipped.\"}");
            }
        }

        /* Fallback 404 */
        else {
            sendHttpResponse(clientFd, 404, "Not Found", "text/plain", "Resource Not Found");
        }

        close(clientFd);
    }

    if (g_serverFd >= 0) {
        close(g_serverFd);
        g_serverFd = -1;
    }

    return 0;
}
