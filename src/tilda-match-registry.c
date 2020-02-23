#include "tilda-match-registry.h"
#include "tilda-regex.h"

#define PCRE2_CODE_UNIT_WIDTH 0
#include <pcre2.h>

typedef struct {
    const char *pattern;
    TerminalURLFlavor flavor;
} MatchPattern;

static const MatchPattern match_patterns[] = {
        { REGEX_URL_AS_IS, TILDA_MATCH_FLAVOR_AS_IS },
        { REGEX_URL_HTTP,  TILDA_MATCH_FLAVOR_DEFAULT_TO_HTTP },
        { REGEX_URL_FILE,  TILDA_MATCH_FLAVOR_AS_IS },
        { REGEX_URL_VOIP,  TILDA_MATCH_FLAVOR_VOIP_CALL },
        { REGEX_EMAIL,     TILDA_MATCH_FLAVOR_EMAIL },
        { REGEX_NEWS_MAN,  TILDA_MATCH_FLAVOR_AS_IS },
        { REGEX_NUMBER, TILDA_MATCH_FLAVOR_NUMBER }
};

typedef struct {
    VteRegex * regex;
    TerminalURLFlavor flavor;
    int tag;
} RegistryEntry;

static void
tilda_match_registry_add (TildaMatchRegistry * registry,
                          const MatchPattern * pattern_item,
                          guint32 flags, GError ** error);

static void registry_entry_free (gpointer data)
{
    RegistryEntry * entry = data;

    vte_regex_unref (entry->regex);

    g_slice_free (RegistryEntry, data);
}

TildaMatchRegistry *
tilda_match_registry_new ()
{
    GError * error;
    guint32 flags;
    TildaMatchRegistry * registry = g_new0 (TildaMatchRegistry, 1);

    registry->entries = NULL;

    flags = PCRE2_CASELESS | PCRE2_MULTILINE;

    for (guint i = 0; i < G_N_ELEMENTS (match_patterns); i++)
    {
        const MatchPattern * pattern_item = match_patterns + i;

        error = NULL;

        tilda_match_registry_add (registry, pattern_item, flags, &error);

        if (error) {
            g_critical ("Could not register match pattern: %s", error->message);
            g_error_free (error);
        }
    }

    return registry;
}

/**
 * tilda_match_registry_for_each:
 * @registry: An instance of a TildaMatchRegistry.
 * @callback: A function to call for each item in the registry.
 * @user_data: user data to pass to the function.
 */
/**
 * TildaMatchHookFunc:
 * @regex: The VteRegex begin enumerated.
 * @user_data: The user data passed to tilda_match_registry_for_each()
 *
 * Declares a type of function that can be passed to
 * tilda_match_registry_for_each() to perform an action on each item
 * in the registry.
 *
 * Returns: A tag to be stored with the regex.
 */
void tilda_match_registry_for_each (TildaMatchRegistry * registry, TildaMatchHookFunc callback, gpointer user_data)
{
    GList * entries = registry->entries;

    for (GList * item = entries; item; item = item->next)
    {
        RegistryEntry * entry = item->data;

        gint tag = callback (entry->regex, user_data);

        entry->tag = tag;
    }
}

static void
tilda_match_registry_add (TildaMatchRegistry * registry,
                          const MatchPattern *pattern_item, guint32 flags, GError ** error)
{
    VteRegex * regex =  vte_regex_new_for_match (pattern_item->pattern, -1, flags, error);

    RegistryEntry * entry = g_slice_new0 (RegistryEntry);

    entry->flavor = pattern_item->flavor;
    entry->regex = regex;
    entry->tag = -1;

    registry->entries = g_list_append (registry->entries, entry);
}

TerminalURLFlavor
tilda_match_registry_get_flavor_for_tag (TildaMatchRegistry * registry,
                                         gint tag)
{
    GList * entries = registry->entries;

    if (entries == NULL) {
        return TILDA_MATCH_FLAVOR_LAST;
    }

    for (GList * item = entries; item; item = item->next)
    {
        RegistryEntry * entry = item->data;

        if (entry->tag == tag) {
            return entry->flavor;
        }
    }

    return TILDA_MATCH_FLAVOR_LAST;
}

void tilda_match_registry_free (TildaMatchRegistry * registry)
{
    g_list_free_full (registry->entries, registry_entry_free);

    g_free (registry);
}
