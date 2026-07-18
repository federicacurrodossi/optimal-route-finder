/*
 * OptimalRoutePlanner
 * -------------------
 * Final project for the Algorithms and Data Structures course
 * (Algoritmi e Principi dell'Informatica), Politecnico di Milano, 2022-2023.
 *
 * A highway is modeled as a set of service stations, each identified by its
 * distance from the start of the highway. Every station holds a fleet of
 * electric rental cars, each with a range (max km on a full charge). Renting
 * a car at station S lets you reach any station within that car's range.
 *
 * Goal: given a start and a destination station, print the route with the
 * fewest stops. When several routes tie on the number of stops, prefer the
 * one that favors stations closer to the start of the highway.
 *
 * Data structures:
 *   - Stations are kept in a binary search tree (BST) keyed by distance.
 *   - Each station stores its cars in a fixed array of up to 512 ranges.
 *   - Routes are built as linked lists.
 *
 * NOTE: the input command keywords (aggiungi-stazione, demolisci-stazione,
 * aggiungi-auto, rottama-auto, pianifica-percorso) and the output messages
 * are kept in Italian on purpose: they are the exact I/O format the project
 * is graded against, so they must not change.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_CARS 512   /* a station holds at most 512 cars */

/* ---- A station: one node of the BST, keyed by its distance ---- */
typedef struct Station {
    struct Station *left;      /* subtree with smaller distances */
    struct Station *right;     /* subtree with larger distances  */
    struct Station *parent;    /* parent node (needed for successor/predecessor) */
    int distance;              /* distance from the start of the highway (the key) */
    int car_count;             /* how many cars are currently parked here */
    int cars[MAX_CARS];        /* each entry is a car's range; 0 means empty slot */
} Station;

typedef Station *StationPtr;

/* ---- A plain distance-only linked list (used for the forward route) ---- */
typedef struct SimpleNode {
    int distance;
    struct SimpleNode *next;
} SimpleNode;

typedef SimpleNode *SimpleList;

/* ---- A route node that also tracks "reach" (used for the backward route) ----
 * reach = distance - max_range: the farthest-back station this stop can reach. */
typedef struct PathNode {
    int distance;
    struct PathNode *next;
    int reach;
} PathNode;

typedef PathNode *PathList;

/* ---- Function declarations ---- */
PathList plan_backward(StationPtr root, int start, int dest);
StationPtr new_station(int dist, int num, int ranges[]);
StationPtr delete_node(StationPtr root, StationPtr target);
StationPtr insert(StationPtr root, StationPtr node);
int contains(StationPtr root, int dist);
SimpleList prepend(SimpleList list, int dist);
void add_car(StationPtr root, int dist, int range);
void remove_car(StationPtr root, int dist, int range);
int max_range(int cars[]);
PathList push_back(PathList list, int distance, int reach);
PathList push_front(PathList list, int distance, int reach);
StationPtr successor(StationPtr root, int distance);
StationPtr tree_min(StationPtr root);
SimpleList append(SimpleList list, int distance);
int path_exists(StationPtr root, int start, int dest);
StationPtr find_node(StationPtr root, int dist);
StationPtr tree_max(StationPtr root);
StationPtr predecessor(StationPtr root, int distance);
SimpleList plan_forward(StationPtr root, int start, int dest);
void print_simple_list(SimpleList list);
void print_path(PathList list);
PathList reverse(PathList list);

int main(void) {
    char command[20];
    int distance, count;
    int ranges[MAX_CARS];
    int range;
    int start, dest;
    StationPtr root = NULL;

    FILE *fp = stdin;

    /* Read and dispatch commands until end of input */
    while (fscanf(fp, "%s", command) != EOF) {

        /* aggiungi-stazione <distance> <count> <range_1> ... <range_count> */
        if (strcmp(command, "aggiungi-stazione") == 0) {
            if (fscanf(fp, "%d %d", &distance, &count) != EOF) {
                /* clear the range buffer before filling it */
                for (int k = 0; k < MAX_CARS; k++) {
                    ranges[k] = 0;
                }
                if (count == 0) {
                    /* a station with no cars still has a trailing token to read */
                    if (fscanf(fp, "%d", &ranges[0]) != EOF) {
                    }
                } else {
                    for (int i = 0; i < count; i++) {
                        if (fscanf(fp, "%d", &ranges[i]) != EOF) {
                        }
                    }
                }
            }
            /* a station is added only if its distance is not already present */
            if (contains(root, distance) == 1) {
                printf("non aggiunta\n");
            } else {
                StationPtr fresh = new_station(distance, count, ranges);
                root = insert(root, fresh);
                printf("aggiunta\n");
            }
        }

        /* demolisci-stazione <distance> */
        if (strcmp(command, "demolisci-stazione") == 0) {
            if (fscanf(fp, "%d", &distance) != EOF) {
                if ((contains(root, distance)) == 1) {
                    root = delete_node(root, find_node(root, distance));
                    printf("demolita\n");
                } else {
                    printf("non demolita\n");
                }
            }
        }

        /* aggiungi-auto <distance> <range> */
        if (strcmp(command, "aggiungi-auto") == 0) {
            if (fscanf(fp, "%d", &distance) != EOF) {
                if (fscanf(fp, "%d", &range) != EOF) {
                    add_car(root, distance, range);
                }
            }
        }

        /* rottama-auto <distance> <range> */
        if (strcmp(command, "rottama-auto") == 0) {
            if (fscanf(fp, "%d %d", &distance, &range) != EOF) {
                remove_car(root, distance, range);
            }
        }

        /* pianifica-percorso <start> <dest> */
        if (strcmp(command, "pianifica-percorso") == 0) {
            if (fscanf(fp, "%d %d", &start, &dest) != EOF) {
                if (start == dest) {
                    printf("%d\n", start);
                } else if (start != dest) {
                    if (path_exists(root, start, dest)) {
                        if (start < dest) {
                            /* traveling left-to-right along the highway */
                            SimpleList route = plan_forward(root, start, dest);
                            print_simple_list(route);
                        }
                        if (start > dest) {
                            /* traveling right-to-left along the highway */
                            PathList route = plan_backward(root, start, dest);
                            print_path(route);
                        }
                    } else {
                        printf("nessun percorso\n");
                    }
                }
            }
        }
    }

    fclose(fp);
    return 0;
}

/* Allocate and initialize a new station with its car ranges. */
StationPtr new_station(int dist, int num, int ranges[]) {
    StationPtr s = (StationPtr) malloc(sizeof(Station));
    s->left = NULL;
    s->right = NULL;
    s->parent = NULL;
    s->distance = dist;
    s->car_count = num;
    for (int i = 0; i < MAX_CARS; i++) {
        s->cars[i] = ranges[i];
    }
    return s;
}

/* Standard BST insert, keyed by distance. Returns the (possibly new) root. */
StationPtr insert(StationPtr root, StationPtr z) {
    StationPtr y = NULL;
    StationPtr x = root;
    if (root == NULL) {
        return z;
    }
    /* walk down to find the insertion point */
    while (x != NULL) {
        y = x;
        if (z->distance < x->distance) {
            x = x->left;
        } else {
            x = x->right;
        }
    }
    z->parent = y;
    if (y == NULL) {
        root = z;
    } else if (z->distance < y->distance) {
        y->left = z;
    } else {
        y->right = z;
    }
    return root;
}

/* Standard BST delete (CLRS-style). Returns the (possibly new) root. */
StationPtr delete_node(StationPtr root, StationPtr z) {
    StationPtr y = NULL;
    StationPtr x = NULL;
    /* y = the node actually spliced out: z itself if it has <2 children,
     * otherwise z's successor */
    if (z->left == NULL || z->right == NULL) {
        y = z;
    } else {
        y = successor(root, z->distance);
    }
    /* x = y's only child (or NULL) */
    if (y->left != NULL) {
        x = y->left;
    } else {
        x = y->right;
    }
    if (x != NULL) {
        x->parent = y->parent;
    }
    /* reconnect x into y's place */
    if (y->parent == NULL) {
        root = x;
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }
    /* if we spliced out the successor, copy its data into z */
    if (y->distance != z->distance) {
        z->distance = y->distance;
        z->car_count = y->car_count;
        for (int i = 0; i < MAX_CARS; i++) {
            z->cars[i] = y->cars[i];
        }
    }
    free(y);
    return root;
}

/* Return 1 if a station at the given distance exists, 0 otherwise. */
int contains(StationPtr s, int dist) {
    if (s == NULL) {
        return 0;
    }
    if (s->distance == dist) {
        return 1;
    }
    if (s->distance < dist) {
        return contains(s->right, dist);
    }
    return contains(s->left, dist);
}

/* Add one car of the given range to the station at `dist`. Prints the result. */
void add_car(StationPtr s, int dist, int car) {
    int i;
    if (s == NULL) {
        printf("non aggiunta\n");
        return;
    }
    if (s->distance == dist) {
        if (s->car_count < MAX_CARS) {
            /* put the car in the first empty slot (0) */
            for (i = 0; i < MAX_CARS; i++) {
                if (s->cars[i] == 0) {
                    s->cars[i] = car;
                    s->car_count++;
                    printf("aggiunta\n");
                    return;
                }
            }
        } else {
            printf("non aggiunta\n");
            return;
        }
    } else if (s->distance < dist) {
        add_car(s->right, dist, car);
        return;
    } else {
        add_car(s->left, dist, car);
        return;
    }
}

/* Remove one car of the given range from the station at `dist`. Prints result. */
void remove_car(StationPtr s, int dist, int car) {
    if (s == NULL) {
        printf("non rottamata\n");
        return;
    }
    if (s->distance == dist) {
        for (int i = 0; i < MAX_CARS; i++) {
            if (s->cars[i] == car) {
                s->cars[i] = 0;          /* free the slot */
                (s->car_count)--;
                printf("rottamata\n");
                return;
            }
        }
        printf("non rottamata\n");
        return;
    }
    if (s->distance < dist) {
        remove_car(s->right, dist, car);
        return;
    }
    remove_car(s->left, dist, car);
    return;
}

/* Return the largest range among a station's cars (its farthest reach). */
int max_range(int a[]) {
    int best = a[0];
    for (int i = 1; i < MAX_CARS; i++) {
        if (a[i] > best) {
            best = a[i];
        }
    }
    return best;
}

/* BST in-order successor of the node with the given key. */
StationPtr successor(StationPtr root, int key) {
    StationPtr x = find_node(root, key);
    if (x->right != NULL) {
        return tree_min(x->right);
    }
    StationPtr y = x->parent;
    while (y != NULL && x == y->right) {
        x = y;
        y = y->parent;
    }
    return y;
}

/* Leftmost (minimum-distance) node of a subtree. */
StationPtr tree_min(StationPtr s) {
    while ((s->left) != NULL) {
        s = s->left;
    }
    return s;
}

/* Rightmost (maximum-distance) node of a subtree. */
StationPtr tree_max(StationPtr s) {
    while (s->right != NULL) {
        s = s->right;
    }
    return s;
}

/*
 * Check whether ANY route exists from `start` to `dest`, ignoring optimality.
 * Walks station by station in the travel direction; at each step the current
 * station's best car must be able to reach the next station, otherwise there
 * is a gap and no route is possible.
 */
int path_exists(StationPtr s, int start, int dest) {
    StationPtr here = NULL;
    here = find_node(s, start);
    StationPtr destination = find_node(s, dest);
    if ((here != NULL) && (destination != NULL)) {
        /* case: destination is farther from the start than the departure */
        if (destination->distance > here->distance) {
            while (here->distance != dest) {
                int best = max_range(here->cars);
                int current_distance = here->distance;
                here = successor(s, here->distance);
                /* can the best car bridge the gap to the next station? */
                if (best < ((here->distance) - current_distance)) {
                    return 0;
                }
            }
            return 1;
        }
        /* case: destination is closer to the start (e.g. go from 50 to 20) */
        else if (destination->distance < here->distance) {
            while (here->distance != dest) {
                int best = max_range(here->cars);
                int current_distance = here->distance;
                here = predecessor(s, here->distance);
                if (best < (current_distance - here->distance)) {
                    return 0;
                }
            }
            return 1;
        }
    } else {
        return 0;
    }
    return 1;
}

/* BST search returning the node itself (or NULL if absent). */
StationPtr find_node(StationPtr s, int dist) {
    if (s == NULL) {
        return s;
    }
    if (s->distance == dist) {
        return s;
    }
    if (s->distance < dist) {
        return find_node(s->right, dist);
    }
    return find_node(s->left, dist);
}

/* BST in-order predecessor of the node with the given key. */
StationPtr predecessor(StationPtr root, int key) {
    StationPtr x = find_node(root, key);
    if (x->left != NULL) {
        return tree_max(x->left);
    }
    StationPtr y = x->parent;
    while (y != NULL && x == y->left) {
        x = y;
        y = y->parent;
    }
    return y;
}

/* Append a distance to the end of a SimpleList (recursively). */
SimpleList append(SimpleList list, int distance) {
    if (list == NULL) {
        return prepend(list, distance);
    }
    list->next = append(list->next, distance);
    return list;
}

/* Prepend a distance to the front of a SimpleList. */
SimpleList prepend(SimpleList list, int dist) {
    SimpleList node;
    node = (SimpleList) malloc(sizeof(SimpleNode));
    node->distance = dist;
    node->next = list;
    return node;
}

/* Push a (distance, reach) node to the front of a PathList. */
PathList push_front(PathList list, int distance, int reach) {
    PathList node;
    node = (PathList) malloc(sizeof(PathNode));
    node->distance = distance;
    node->reach = reach;
    node->next = list;
    return node;
}

/* Push a (distance, reach) node to the end of a PathList (recursively). */
PathList push_back(PathList list, int distance, int reach) {
    if (list == NULL) {
        return push_front(list, distance, reach);
    }
    list->next = push_back(list->next, distance, reach);
    return list;
}

/*
 * Plan a route when traveling FORWARD (start < dest), i.e. left-to-right.
 * Greedy from the destination backward: repeatedly find the earliest station
 * whose distance + best range can reach the current target, and hop there.
 */
SimpleList plan_forward(StationPtr s, int start, int dest) {
    StationPtr target = find_node(s, dest);   /* current target we must reach */
    SimpleList list = NULL;
    StationPtr p = find_node(s, start);        /* fixed starting station */
    StationPtr scan = p;
    while (target->distance != p->distance) {
        /* advance `scan` forward until it can reach `target` in one hop */
        while (scan->distance + max_range(scan->cars) < target->distance) {
            scan = successor(s, scan->distance);
        }
        /* `scan` is now the earliest station that reaches `target` */
        list = prepend(list, scan->distance);
        target = scan;
        scan = p;
    }
    list = append(list, dest);
    return list;
}

/* Print a SimpleList of distances separated by spaces. */
void print_simple_list(SimpleList list) {
    while (list != NULL) {
        printf("%d ", list->distance);
        list = list->next;
    }
    printf("\n");
}

/* Print a PathList of distances separated by spaces. */
void print_path(PathList list) {
    while (list != NULL) {
        printf("%d ", list->distance);
        list = list->next;
    }
    printf("\n");
}

/*
 * Plan a route when traveling BACKWARD (start > dest), i.e. right-to-left.
 * This is the hard case: it first builds a minimum-stops route, then makes
 * two refinement passes to enforce the tie-break rule (prefer stops closer
 * to the start of the highway when the number of stops is equal).
 *
 * For a backward-reaching station, `reach = distance - max_range` is the
 * closest-to-start distance it can still reach.
 */
PathList plan_backward(StationPtr root, int start, int dest) {
    PathList list = NULL;
    StationPtr s = find_node(root, dest);

    /* --- Pass 1: build a minimum-stops route, greedily from the start --- */
    StationPtr t = find_node(root, start);
    StationPtr b = t;      /* b scans back from the (farther) start */
    StationPtr temp = s;   /* temp tracks the current target, from dest */
    while (temp->distance != start) {
        /* move b back while it can still reach past temp */
        while ((b->distance - max_range(b->cars)) > temp->distance) {
            b = predecessor(root, b->distance);
        }
        list = push_front(list, b->distance, b->distance - max_range(b->cars));
        temp = b;
        b = t;
    }
    list = push_back(list, dest, 0);

    /* --- Pass 2: first tie-break refinement pass ---
     * For each consecutive pair, try to replace a stop with a
     * closer-to-start station that still preserves reachability. */
    PathList h = list;
    PathList next = h->next;
    while (next->distance != dest) {
        if (h->reach >= next->reach) {
            StationPtr k = find_node(root, h->distance);
            StationPtr scan = predecessor(root, k->distance);
            int best_distance = scan->distance;
            int best_reach = scan->distance - max_range(scan->cars);
            k = scan;
            while (h->reach <= scan->distance) {
                if (best_reach >= scan->distance - max_range(scan->cars)) {
                    best_distance = scan->distance;
                    best_reach = scan->distance - max_range(scan->cars);
                }
                scan = predecessor(root, scan->distance);
            }
            next->distance = best_distance;
            next->reach = best_reach;
            h = next;
            next = next->next;
        } else if (next->reach > h->reach) {
            StationPtr k = find_node(root, next->distance);
            StationPtr scan = predecessor(root, k->distance);
            int best_distance = scan->distance;
            int best_reach = scan->distance - max_range(scan->cars);
            k = scan;
            while (next->reach <= scan->distance) {
                if (best_reach >= scan->distance - max_range(scan->cars)) {
                    best_distance = scan->distance;
                    best_reach = scan->distance - max_range(scan->cars);
                }
                scan = predecessor(root, scan->distance);
            }
            next->distance = best_distance;
            next->reach = best_reach;
            h = next;
            next = next->next;
        }
    }

    /* --- Pass 3: second refinement, scanning in the reversed order ---
     * Reverse, refine each middle stop toward the smallest valid distance,
     * then reverse back to restore travel order. */
    list = reverse(list);

    PathList c = list;
    PathList mid = c->next;
    PathList after = mid->next;
    while (mid->distance != start) {
        StationPtr s2 = find_node(root, mid->distance);
        int best_distance = s2->distance;
        int best_reach = mid->reach;
        while (s2->distance >= after->reach) {
            if ((best_distance > s2->distance) &&
                (s2->distance - max_range(s2->cars) <= c->distance)) {
                best_distance = s2->distance;
                best_reach = s2->distance - max_range(s2->cars);
            }
            s2 = predecessor(root, s2->distance);
        }
        mid->distance = best_distance;
        mid->reach = best_reach;
        c = mid;
        mid = after;
        after = after->next;
    }
    list = reverse(list);

    return list;
}

/* Reverse a PathList in place, returning the new head. */
PathList reverse(PathList c) {
    PathList sec, head, tmp;
    head = c;
    sec = c->next;
    head->next = NULL;
    tmp = head;
    while (sec != NULL) {
        head = sec;
        sec = sec->next;
        head->next = tmp;
        tmp = head;
    }
    return head;
}
