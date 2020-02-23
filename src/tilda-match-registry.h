#ifndef TILDA_MATCH_REGISTRY_H
#define TILDA_MATCH_REGISTRY_H

#include <glib.h>
#include <vte/vte.h>

typedef enum {
    TILDA_MATCH_FLAVOR_AS_IS,
    TILDA_MATCH_FLAVOR_DEFAULT_TO_HTTP,
    TILDA_MATCH_FLAVOR_VOIP_CALL,
    TILDA_MATCH_FLAVOR_EMAIL,
    TILDA_MATCH_FLAVOR_NUMBER,
    TILDA_MATCH_FLAVOR_LAST
} TerminalURLFlavor;

typedef struct {
    GList *entries;
} TildaMatchRegistry;

typedef int (*TildaMatchHookFunc)(VteRegex * regex, gpointer user_data);

TildaMatchRegistry * tilda_match_registry_new (void);

void tilda_match_registry_for_each (TildaMatchRegistry * registry,
                                    TildaMatchHookFunc callback,
                                    gpointer user_data);

TerminalURLFlavor
tilda_match_registry_get_flavor_for_tag (TildaMatchRegistry * registry,
                                         gint tag);

void tilda_match_registry_free (TildaMatchRegistry * registry);

#endif
