# QJSON Datalog

A fact language for humans, robots, and agents.  Precise. Load-bearing.
Four verbs, one trigger, one distributor, two freezes.

## Facts

```
assert {parent: "alice", child: "bob"} in is_parent
```

Bulk — `each` distributes, `in` at the end is the shared predicate:

```
assert each {
  {parent: "alice", child: "bob"},
  {parent: "bob", child: "carol"},
  {parent: "carol", child: "dave"}
} in is_parent
```

## Rules

A rule is a fact with a reason.  `{what} in predicate where reason`:

```
assert {grandparent: ?GP, grandchild: ?GC} in is_grandparent
  where {parent: ?GP, child: ?P} in is_parent
    and {parent: ?P, child: ?GC} in is_parent
```

Arithmetic with exact decimals:

```
assert {name: ?N, total: ?T} in has_total
  where {name: ?N, unit_price: ?U, qty: ?Q} in has_item
    and ?T = ?U * ?Q
```

Boolean conditions: `and`, `or`, `not`, parentheses.

## Programs

`assert each` with per-element `in` loads an entire brain:

```
assert each {
  {parent: "alice", child: "bob"} in is_parent,
  {parent: "bob", child: "carol"} in is_parent,

  {person: "alice"} in is_female,
  {person: "bob"} in is_male,

  {grandparent: ?GP, grandchild: ?GC} in is_grandparent
    where {parent: ?GP, child: ?P} in is_parent
      and {parent: ?P, child: ?GC} in is_parent,

  {ancestor: ?A, descendant: ?D} in is_ancestor
    where {parent: ?A, child: ?D} in is_parent,
  {ancestor: ?A, descendant: ?D} in is_ancestor
    where {parent: ?A, child: ?M} in is_parent
      and {ancestor: ?M, descendant: ?D} in is_ancestor
}
```

Facts, rules, multiple predicates — one block.

## Queries

```
select {grandparent: "alice", grandchild: ?GC} in is_grandparent
```

Concrete values filter.  Variables return all matches.

## Signals

Transient.  Never stored.  Both input and output.

```
signal {patient: "alice", glucose: 280M, time: 1710000600N} in pump_reading
```

The host decides what outbound signals mean.

## Triggers

`when` reacts to `signal`, `assert`, `retract`:

```
when signal {patient: ?P, glucose: ?G, time: ?T} in pump_reading:
  assert {patient: ?P, glucose: ?G, time: ?T} in is_reading

when assert {patient: ?P, glucose: ?G} in is_reading
  where ?G > 250M:
  signal {patient: ?P, glucose: ?G, level: "critical"} in alert
```

Triggers cascade until fixpoint.

## Retract

```
retract {patient: "alice", time: ?Old} in is_reading
  where ?Old < 1710000000N
```

Bulk:

```
retract each {
  {time: 1000N},
  {time: 2000N}
} in is_reading
```

## Freeze

```
mineralize(is_high)      // freeze one rule into immutable facts
fossilize                 // freeze everything
```

Signals still flow through frozen rules.

## The whole language

```
assert VALUE in PREDICATE                         fact
assert VALUE in PREDICATE where REASON            rule
assert each AGGREGATE in PREDICATE                bulk facts (shared predicate)
assert each { V in P where R, V in P, ... }       program (mixed predicates)

when signal PATTERN in PRED where COND:           trigger
  assert|retract|signal actions
when assert PATTERN in PRED where COND:
  assert|retract|signal actions
when retract PATTERN in PRED where COND:
  assert|retract|signal actions

signal VALUE in PREDICATE                         transient in/out
retract PATTERN in PREDICATE where COND           forget
select PATTERN in PREDICATE                       query
mineralize(PREDICATE)                             freeze one
fossilize                                         freeze all
```

Four verbs: `assert`, `retract`, `signal`, `select`.
One trigger: `when`.  One distributor: `each`.
Two freezes: `mineralize`, `fossilize`.
One relation operator: `in`.
One data format: QJSON.

Every clause reads left to right: what, where it lives, why it's true.

## Embedding

```
Signals in:    host → signal {...} in pump_reading
Brain reacts:  when signal → assert → when assert → signal
Signals out:   signal {...} in alert → host handles it
```

| Host | Signals in | Signals out |
|------|-----------|-------------|
| Browser (WASM) | fetch → signal | signal → setState |
| CLI | stdin → signal | signal → stdout |
| Embedded (C) | GPIO → signal | signal → GPIO |
| Server (Python) | HTTP → signal | signal → response |

## Example: insulin pump monitor

```
assert each {
  {patient: "alice", max_iob: 5.0M, target: {low: 80M, high: 120M}} in has_config,
  {sensor: "pump_1"} in is_trusted,

  {patient: ?P, glucose: ?G, level: "critical"} in is_high
    where {patient: ?P, glucose: ?G} in is_reading
      and ?G > 250M,

  {patient: ?P, glucose: ?G, level: "critical"} in is_low
    where {patient: ?P, glucose: ?G} in is_reading
      and ?G < 70M,

  {patient: ?P, iob: ?IOB, max: ?MAX} in is_over_max_iob
    where {patient: ?P, iob: ?IOB} in is_reading
      and {patient: ?P, max_iob: ?MAX} in has_config
      and ?IOB > ?MAX
}

mineralize(is_high)
mineralize(is_low)
mineralize(is_over_max_iob)

when signal {sensor: ?S, patient: ?P, glucose: ?G, iob: ?IOB, time: ?T} in pump_reading
  where {sensor: ?S} in is_trusted:
  retract {patient: ?P} in is_reading
  assert {patient: ?P, glucose: ?G, iob: ?IOB, time: ?T} in is_reading

when assert {patient: ?P, glucose: ?G} in is_reading
  where {patient: ?P} in is_high
     or {patient: ?P} in is_low
     or {patient: ?P} in is_over_max_iob:
  signal {patient: ?P, glucose: ?G, level: "critical"} in alert

signal {sensor: "pump_1", patient: "alice", glucose: 280M, iob: 3.2M, time: 1710000600N} in pump_reading
```

A nurse reads the rules.  An agent writes them.  An auditor
verifies the frozen facts.  The pump stays safe.
