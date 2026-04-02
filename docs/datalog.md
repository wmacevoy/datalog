# QJSON Datalog

A fact language for humans, robots, and agents.  Precise. Load-bearing.
Four verbs, two freezes.  Everything is the database.

## Facts

A fact is a QJSON value `in` a predicate.  `assert` puts it there.

```
assert {parent: "alice", child: "bob"} in is_parent
```

Bulk — `each` distributes over the aggregate:

```
assert each {
  {parent: "alice", child: "bob"},
  {parent: "bob", child: "carol"},
  {parent: "carol", child: "dave"}
} in is_parent
```

Without `each`, the aggregate is one atomic fact.  With `each`,
each element is a separate fact.

Values are QJSON: `M` for exact decimal, `N` for big integer,
`L` for big float, `0j` for blobs, `?X` for variables.
String values are quoted.  Valid JSON is valid QJSON.

## Rules

A rule is an assertion with a `where` clause.  It derives new
facts from existing ones.  `in` references the predicate.

```
assert {grandparent: ?GP, grandchild: ?GC} in is_grandparent
  where {parent: ?GP, child: ?P} in is_parent
    and {parent: ?P, child: ?GC} in is_parent
```

`?P` appears in both patterns — that's a join.

Rules can use arithmetic.  Type widening selects exact base-10 or
base-2 automatically:

```
assert {name: ?N, total: ?T} in has_total
  where {name: ?N, unit_price: ?U, qty: ?Q} in has_item
    and ?T = ?U * ?Q
```

`0.1M + 0.2M = 0.3M`.  Not `0.30000000000000004`.

Boolean conditions: `and`, `or`, `not`, parentheses.

```
assert {patient: ?P} in is_eligible
  where {name: ?P, age: ?A} in is_patient
    and ?A >= 18
    and not {patient: ?P} in is_excluded
```

## Queries

```
select {grandparent: ?GP, grandchild: "carol"} in is_grandparent
```

Concrete values filter.  Variables return all matches.

## Signals

A signal is a transient fact.  It enters the brain, triggers
reactions, and is never stored.

```
signal {patient: "alice", glucose: 280M, time: 1710000600N} in pump_reading
```

Signals are both input and output.  They come from the world
(a sensor, an API, a user action) and go back out (an alert,
a command, a UI update).  The host decides what outbound signals
mean.

## Triggers

`when` reacts to state changes.  Three events: `signal`, `assert`,
`retract`.  Nothing else happens in a fact-based system.

```
when signal {patient: ?P, glucose: ?G, time: ?T} in pump_reading:
  assert {patient: ?P, glucose: ?G, time: ?T} in is_reading
```

A `where` clause adds conditions and computation:

```
when signal {patient: ?P, glucose: ?G, time: ?T} in pump_reading
  where {patient: ?P, time: ?Old} in is_reading
    and ?Old < ?T:
  retract {patient: ?P, time: ?Old} in is_reading
  assert {patient: ?P, glucose: ?G, time: ?T} in is_reading
```

Triggers cascade.  A signal triggers a `when`, which asserts a
fact, which triggers another `when`, which signals outward.
Until fixpoint — no new actions to take.

```
// Sensor arrives
when signal {patient: ?P, glucose: ?G, time: ?T} in pump_reading:
  assert {patient: ?P, glucose: ?G, time: ?T} in is_reading

// Reading stored → check thresholds
when assert {patient: ?P, glucose: ?G} in is_reading
  where ?G > 250M:
  signal {patient: ?P, glucose: ?G, level: "critical"} in alert

// Alert → the host handles it (SMS, LED, React component, log)
```

The brain signals into `alert`.  It doesn't know what an SMS is.
The host watches for signals in `alert` and acts.  That's the
embedding.

## Retract

Forget a fact.  Pattern matching selects what to remove.

```
retract {patient: "alice", time: ?Old} in is_reading
  where ?Old < 1710000000N
```

Bulk:

```
retract each {
  {patient: "alice", time: 1000N},
  {patient: "alice", time: 2000N}
} in is_reading
```

## Freeze

### mineralize

Freeze a rule into concrete facts.  Evaluate the derivation,
replace the rule with its results.  Immutable.  Auditable.

```
mineralize(is_high)
```

Before: a live rule that re-evaluates.  After: frozen facts
that document what the rule produced.  An FDA auditor sees
facts, not code.

### fossilize

Freeze everything.  All rules become facts.  The database becomes
a snapshot — a fossil of what the brain knew at that moment.

```
fossilize
```

Signals still flow through fossilized rules.  The rules can't
change, but the brain still reacts.  The skull protects the brain.

## The whole language

```
// Facts and rules (all just assert)
assert VALUE in PREDICATE                     one fact
assert each AGGREGATE in PREDICATE            distribute
assert PATTERN in PREDICATE where CONDITION   rule

// Triggers (the nervous system)
when signal PATTERN in PRED where COND:       on transient input
  assert|retract|signal actions
when assert PATTERN in PRED where COND:       on fact added
  assert|retract|signal actions
when retract PATTERN in PRED where COND:      on fact removed
  assert|retract|signal actions

// Operations
signal VALUE in PREDICATE                     transient in/out
retract PATTERN in PREDICATE                  forget
retract each AGGREGATE in PREDICATE           bulk forget
select PATTERN in PREDICATE                   query
mineralize(PREDICATE)                         freeze one
fossilize                                     freeze all
```

Four verbs: `assert`, `retract`, `signal`, `select`.
One trigger: `when`.
One distributor: `each`.
Two freezes: `mineralize`, `fossilize`.
One relation operator: `in`.
One data format: QJSON.

`:` is only inside QJSON objects (key maps to value).
`in` is the relationship operator (fact is in predicate).

## Embedding

The brain doesn't know about the outside world.  The host provides
the body.

```
Signals in:    host → signal {...} in pump_reading
Brain reacts:  when signal → assert → when assert → signal
Signals out:   signal {...} in alert → host handles it
```

The host is whatever runs the brain:

| Host | Signals in | Signals out |
|------|-----------|-------------|
| Browser (WASM) | fetch → signal | signal → setState |
| CLI | stdin → signal | signal → stdout |
| Embedded (C) | GPIO → signal | signal → GPIO |
| Server (Python) | HTTP → signal | signal → response |

libqjson is the brain.  The host is the body.  `signal` is the
nerve.  The language is the same everywhere.

## Example: insulin pump monitor

```
// Configuration
assert {patient: "alice", max_iob: 5.0M, target: {low: 80M, high: 120M}} in has_config
assert {sensor: "pump_1"} in is_trusted

// Alert rules
assert {patient: ?P, glucose: ?G, level: "critical"} in is_high
  where {patient: ?P, glucose: ?G} in is_reading
    and ?G > 250M

assert {patient: ?P, glucose: ?G, level: "critical"} in is_low
  where {patient: ?P, glucose: ?G} in is_reading
    and ?G < 70M

assert {patient: ?P, iob: ?IOB, max: ?MAX} in is_over_max_iob
  where {patient: ?P, iob: ?IOB} in is_reading
    and {patient: ?P, max_iob: ?MAX} in has_config
    and ?IOB > ?MAX

// Freeze the rules — immutable, auditable
mineralize(is_high)
mineralize(is_low)
mineralize(is_over_max_iob)

// Nervous system
when signal {sensor: ?S, patient: ?P, glucose: ?G, iob: ?IOB, time: ?T} in pump_reading
  where {sensor: ?S} in is_trusted:
  retract {patient: ?P} in is_reading
  assert {patient: ?P, glucose: ?G, iob: ?IOB, time: ?T} in is_reading

when assert {patient: ?P, glucose: ?G} in is_reading
  where {patient: ?P} in is_high
     or {patient: ?P} in is_low
     or {patient: ?P} in is_over_max_iob:
  signal {patient: ?P, glucose: ?G, level: "critical"} in alert

// Life
signal {sensor: "pump_1", patient: "alice", glucose: 280M, iob: 3.2M, time: 1710000600N} in pump_reading
// → assert in is_reading
// → is_high matches (280M > 250M)
// → signal in alert
// → host sends SMS, updates dashboard, logs to audit trail
```

A nurse reads the rules.  An agent writes them.  An auditor
verifies the frozen facts.  The pump stays safe.
