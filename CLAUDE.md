# CLAUDE.md

## Project

QJSON Datalog — a fact language for humans, robots, and agents.

Five verbs: assert (`.`), retract, signal, select, when.
Two freezes: mineralize, fossilize.
One data format: QJSON (exact decimals, big integers, blobs, variables).

Facts are the database.  Rules derive from facts.  Signals are transient.
Triggers react to state changes.  The brain doesn't know about the outside
world — the host provides the body, `signal` is the nerve.

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
// Facts (the database)
name: {pattern}.                              assert
name: {pattern} where condition.              rule

// Triggers (the nervous system)
when signal pattern where condition:          on transient input
  assert|retract|signal action
when assert pattern where condition:          on fact added
  assert|retract|signal action
when retract pattern where condition:         on fact removed
  assert|retract|signal action

// Operations
signal name: {data}                           transient in/out
retract name: {pattern} where condition       forget
select name: {pattern}                        query
mineralize(name)                              freeze one
fossilize                                     freeze all
```

## Key concepts

**Facts**: named QJSON objects followed by `.` — "this is true."
**Rules**: facts with `where` clauses — derived, always live.
**Signals**: transient facts — enter, trigger reactions, never stored.
**When**: reacts to `signal`, `assert`, `retract` — the nervous system.
**Mineralize**: freeze a rule into immutable facts — compile-time safety.
**Fossilize**: freeze everything — snapshot the brain.

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
**Keywords**: `where`, `and`, `or`, `not`, `in`, `when`, `signal`,
`select`, `retract`, `assert`, `mineralize`, `fossilize` are reserved.

## License

MIT
