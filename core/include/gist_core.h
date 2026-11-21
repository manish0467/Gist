#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GistEngine GistEngine;
typedef void (*EventCallback)(const char *event_json);

GistEngine *gist_core_new(const char *db_path);
void gist_core_free(GistEngine *engine);
void gist_core_set_callback(GistEngine *engine, EventCallback callback);

bool gist_core_add_account(
    GistEngine *engine,
    const char *id,
    const char *name,
    const char *email,
    const char *imap_host,
    uint16_t imap_port,
    const char *username,
    const char *password
);

bool gist_core_sync_account(GistEngine *engine, const char *account_id);
char *gist_core_get_messages_json(GistEngine *engine, const char *folder);
void gist_core_free_string(char *s);

#ifdef __cplusplus
}
#endif
