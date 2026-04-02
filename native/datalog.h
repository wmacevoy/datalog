/* ============================================================
 * datalog.h — QJSON Datalog: reactive fact engine
 *
 * Facts, rules, signals, triggers.  Five verbs, two freezes.
 * The brain doesn't know about the outside world — the host
 * provides the body, signal is the nerve.
 *
 *   datalog_t *dl = datalog_open(":memory:");
 *   datalog_load(dl, "is_parent: {parent: alice, child: bob}.");
 *   datalog_signal(dl, "pump_reading", "{patient: alice, glucose: 280M}");
 *   datalog_result *r = datalog_select(dl, "is_grandparent", "{grandchild: carol}");
 *   datalog_close(dl);
 * ============================================================ */

#ifndef DATALOG_H
#define DATALOG_H

#include "qjson.h"

/* Opaque engine handle */
typedef struct datalog_t datalog_t;

/* Signal callback: host receives outbound signals */
typedef void (*datalog_signal_fn)(const char *name, qjson_val *data, void *userdata);

/* ── Lifecycle ──────────────────────────────────────────────── */

datalog_t *datalog_open(const char *db_path);
void       datalog_close(datalog_t *dl);

/* ── Load ───────────────────────────────────────────────────── */

/* Parse and execute datalog text: facts, rules, triggers, operations.
   Returns 0 on success, -1 on error. */
int datalog_load(datalog_t *dl, const char *text, int len);

/* Load from file */
int datalog_load_file(datalog_t *dl, const char *path);

/* ── Five verbs ─────────────────────────────────────────────── */

/* Assert: add a fact to the database.  Fires `when assert` triggers. */
int datalog_assert(datalog_t *dl, const char *name, const char *qjson_data);

/* Retract: remove matching facts.  Fires `when retract` triggers. */
int datalog_retract(datalog_t *dl, const char *name, const char *pattern);

/* Signal: transient fact.  Fires `when signal` triggers.  Never stored. */
int datalog_signal(datalog_t *dl, const char *name, const char *qjson_data);

/* Select: query facts and derived rules.
   Returns array of matching QJSON values (arena-allocated). */
qjson_val *datalog_select(datalog_t *dl, const char *name, const char *pattern);

/* When: register a trigger (usually via datalog_load, not directly) */
int datalog_when(datalog_t *dl, const char *event, const char *pattern,
                 const char *condition, const char *actions);

/* ── Two freezes ────────────────────────────────────────────── */

/* Mineralize: freeze a rule into concrete facts.  Immutable. */
int datalog_mineralize(datalog_t *dl, const char *name);

/* Fossilize: freeze everything.  All rules become facts. */
int datalog_fossilize(datalog_t *dl);

/* ── Embedding ──────────────────────────────────────────────── */

/* Watch for outbound signals.  The host provides the body. */
int  datalog_watch(datalog_t *dl, const char *signal_name,
                   datalog_signal_fn fn, void *userdata);
void datalog_unwatch(datalog_t *dl, int watch_id);

/* Collect outbound signals (alternative to watch — poll mode) */
qjson_val *datalog_collect_signals(datalog_t *dl);

#endif
