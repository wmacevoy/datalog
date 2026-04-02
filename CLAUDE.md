# CLAUDE.md

## Project

QJSON Datalog — a fact language for humans, robots, and agents.

Four verbs: `assert`, `retract`, `signal`, `select`.
One trigger: `when`.  One distributor: `each`.
One freeze: `freeze` (modifier or verb).
One relation operator: `in`.
One data format: QJSON (exact decimals, big integers, blobs, variables).

`:` is only inside QJSON objects (key maps to value).
`in` is the relationship operator (fact is in predicate).

## Architecture

```
datalog (the language)
├── native/              C implementation (Lemon parser, resolver, reactive engine)
├── src/                 JS + Python implementations (call native via WASM/FFI)
├── test/                Tests
├── examples/            Working examples
├── docs/                Specification
└── vendor/qjson/        QJSON data format (submodule)

datalog depends on qjson.  qjson does not depend on datalog.
```

## Commands

```bash
# C tests
make test

# Examples
make family       # family tree: facts, rules, queries
make pump         # insulin pump: signals, triggers, alerts
make vending      # vending machine: state, events, cascades

# Build
make              # libdatalog.a + datalog_ext.so (SQLite extension)
```

## The language

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

## Key concepts

**assert**: put a fact in a predicate. With `where`, it's a rule.
**retract**: remove matching facts from a predicate.
**signal**: transient fact — triggers reactions, never stored.
**select**: query — pattern match against facts and derived rules.
**when**: react to `signal`, `assert`, `retract` — the nervous system.
**each**: distribute an aggregate — `assert each {A, B} in P` = assert A, assert B.
**in**: the relationship operator — "this fact is in this predicate."
**freeze**: make immutable.  `assert freeze` = const declaration (born frozen).
`freeze P` = freeze existing.  `freeze` = freeze everything.

## Embedding

The brain doesn't know about SMS, LEDs, React components, or GPIO.
The host watches for outbound signals and acts.

| Host | Signals in | Signals out |
|------|-----------|-------------|
| Browser (WASM) | fetch → signal | signal → setState |
| CLI | stdin → signal | signal → stdout |
| Embedded (C) | GPIO → signal | signal → GPIO |
| Server (Python) | HTTP → signal | signal → response |

## Constraints

**C files**: C11, arena-allocated, no global mutable state.
**QJSON values**: exact — `0.1M + 0.2M = 0.3M`.
**Keywords**: `assert`, `retract`, `signal`, `select`, `when`, `each`,
`in`, `where`, `and`, `or`, `not`, `freeze` are reserved.

## License

MIT
