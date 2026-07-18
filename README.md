# Optimal Route Finder

An efficient C program that finds the **optimal route across a highway** of
electric-vehicle charging stations — the route with the fewest stops, breaking
ties in favor of stations closer to the start of the highway.

Built as the final project for the **Algorithms and Data Structures**
(*Algoritmi e Principi dell'Informatica*) course at Politecnico di Milano,
2022–2023. The project is graded on both **correctness** and **efficiency**
(time and memory) against large hidden test files.

## The problem

A highway is a line of service stations, each identified by its distance from
the start (a unique non-negative integer). Every station has a fleet of up to
512 electric rental cars, and each car has a **range** — the maximum distance
it can travel on a full charge. Renting a car at a station lets you reach any
station within that car's range.

The program processes a stream of commands that build and modify the highway
(add/remove stations, add/scrap cars) and answer routing queries. For a
routing query between two stations, it must return the route with the
**minimum number of stops**. When several routes tie on the number of stops, it
must pick the one that prefers earlier stops closer to the start of the
highway.

### Example

```
        20      30      45      50      distance (km)
        ●───────●───────●───────●
        │       │               │
 route: └──►────┴──────►────────┘
             20 → 30 → 50   ✓   (chosen)
             20 → 45 → 50   ✗   (same stop count, but 45 > 30)
```

Both `20 → 30 → 50` and `20 → 45 → 50` reach the destination in two hops, but
the tie-break rule prefers `30` over `45` because it's closer to the start.
Traveling the other way, `50 → 30 → 20` is the correct right-to-left route.

## Commands

The program reads commands from standard input and prints one line of output
per command:

| Command | Meaning |
|---------|---------|
| `aggiungi-stazione <dist> <n> <r1>…<rn>` | add a station at `dist` with `n` cars of the given ranges |
| `demolisci-stazione <dist>` | remove the station at `dist` |
| `aggiungi-auto <dist> <range>` | add one car of the given range to a station |
| `rottama-auto <dist> <range>` | scrap one car of the given range from a station |
| `pianifica-percorso <start> <dest>` | print the optimal route between two stations |

*(Command keywords are in Italian because they are the exact input format the
project is graded against.)*

## Approach

The solution combines a balanced-ish search structure with a greedy routing
strategy:

- **Stations → binary search tree (BST).** Stations are stored in a BST keyed
  by distance, so adding, removing, and looking up a station are all
  `O(log n)` on average rather than `O(n)`. In-order **successor** and
  **predecessor** operations give the next/previous station along the highway
  in either direction — exactly what route-walking needs.
- **Cars → per-station array.** Each station keeps its car ranges in a fixed
  array; the only value that matters for routing is the **maximum range**,
  i.e. how far the station's best car can travel.
- **Routing → greedy shortest-hop.** To plan a route, the algorithm greedily
  chooses, at each step, the stop that makes the most progress toward the
  destination in a single hop — which yields the minimum number of stops.
- **Tie-breaking → refinement passes.** Traveling right-to-left, a plain greedy
  pass can produce a minimum-stop route that isn't the one the tie-break rule
  wants. The program then runs refinement passes that push each intermediate
  stop toward the smallest valid distance, so that among all minimum-stop
  routes it returns the one preferred by the rule.

Reachability between two stations is checked separately (walking station by
station and confirming each gap is bridgeable by the best available car)
before a route is planned, so impossible queries are answered immediately.

## Building and running

```bash
gcc -Wall -Wextra -O2 -std=c11 route_planner.c -o route_planner
./route_planner < input.txt
```

The program reads commands from standard input and writes results to standard
output, so it can be run directly against a test file as shown above.
