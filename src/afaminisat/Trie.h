#ifndef Trie_h
#define Trie_h

#include <limits>
#include <vector>
#include <unordered_set>
#include <utility>
#include <fstream>
#include <map>
#include <stdint.h>
#include <unordered_map>

#include "Constraints.h"
#include "LogList.h"

class Solver;

using std::pair;
using std::vector;
using std::unordered_set;
using std::map;
using std::unordered_map;

//=================================================================================================

struct Head;
const unsigned IX_NULL = std::numeric_limits<unsigned>::max();


enum WhatToDo {
  WATCH,
  DONE,
  EXHAUST,
  RIGHT,
  DOWN,
};

enum MultimoveEnd {
  E_WATCH = -3,
  E_DONE = -2,
  E_EXHAUST = 0,
};


class Trie;

struct HeadAttrs {
  Head *head;
  Solver &S;
  HeadAttrs(Head *p, Solver &_S) : head(p), S(_S) { };
  friend std::ostream& operator<<(std::ostream& os, HeadAttrs const &p);
};

struct DotHead {
  Head *head;
  Solver &S;
  DotHead(Head *p, Solver &_S) : head(p), S(_S) { };
  friend std::ostream& operator<<(std::ostream& os, DotHead const &p);
};

struct MinusSnapshot {
  Head *place;

  void undo(Solver &S);
};

struct PlusSnapshot {
  Head *place;
  int last_change_level;
  Head *dual;  // Rears have no duals in snapshots, so this implies is_van.
  MinusSnapshot *minus_snapshot;

#ifdef AFA
  Head *deepest_rightmost_guard;
#endif
};

struct Snapshot {
#ifdef AFA
  // Rears must be created before vans and in AFA onSat, when post-calculating snapshots,
  // new van snapshots are created after rear snapshots exist, so they would be triggered earlier.
  LogList<PlusSnapshot> rear_plus_snapshots;
#endif
  LogList<PlusSnapshot> plus_snapshots;
  LogList<MinusSnapshot> minus_snapshots;

  Snapshot() {}

  Snapshot(Snapshot&& old) noexcept
  : plus_snapshots(std::move(old.plus_snapshots))
  , minus_snapshots(std::move(old.minus_snapshots))
#ifdef AFA
  , rear_plus_snapshots(std::move(old.rear_plus_snapshots))
#endif
  {}

  Snapshot(Snapshot& old) = delete;
};


enum GuardType { DANGLING_GUARD, VAN_GUARD, REAR_GUARD, SOLO_GUARD };

#ifdef AFA
#define deepest_rightmost_van previous
#endif

struct Guard {
  GuardType guard_type;
  int last_change_level;
  Head *dual;
  Head *previous, *next;
  MinusSnapshot *minus_snapshot;

  void init(
    GuardType guard_type_,
    Head* dual_,
    int last_change_level_,
    MinusSnapshot *msnap_
  ) {
    guard_type = guard_type_;
    dual = dual_;
    last_change_level = last_change_level_;
    previous = NULL;
    next = NULL;
    minus_snapshot = msnap_;
  }

  void untangle();
  MinusSnapshot *get_msnap(int level, int root_level) {
    return last_change_level == level || level <= root_level ? minus_snapshot : NULL;
  }
};


class MultimoveCtx {
public:
  vec<char> &assigns;
  vector<Head*> stack;

  MultimoveCtx(vec<char> &assigns_) : assigns(assigns_) {}

  pair<Head*, WhatToDo> move_right(Head* x);
  pair<Head*, WhatToDo> move_down(Head* x);

  void branch(Head* x);

  WhatToDo get_what_to_do(Head* x);

  pair<Head *, MultimoveEnd> multimove(pair<Head *, WhatToDo> move);
  pair<Head *, MultimoveEnd> first(pair<Head *, WhatToDo> move);
  pair<Head *, MultimoveEnd> next();
  pair<Head *, MultimoveEnd> first_solo(pair<Head *, WhatToDo> move, Solver &S);
  pair<Head *, MultimoveEnd> next_solo(Solver &S);
};


struct Head : public Reason, public Constr {
public:
  // Constant fields.
  Lit tag;
  bool is_ver;
  Head *right;
  Head *down;
  Head *above;
  unsigned external;
  unsigned depth;

  // Dynamic fields.
  bool watching = false;

  // Guard's fields.
  Guard guard;

  Head()
  : tag(lit_Undef)
  , is_ver(true)
  , right(NULL)
  , down(NULL)
  , above(NULL)
  , external(0)
  , depth(0)
  , watching(false)
  , guard()
  { }

  Head(Lit tag_)
  : tag(tag_)
  , is_ver(false)
  , right(NULL)
  , down(NULL)
  , above(NULL)
  , external(0)
  , depth(0)
  , watching(false)
  , guard()
  { }

  Head(Head&& old) noexcept
  : tag(old.tag)
  , is_ver(old.is_ver)
  , right(old.right)
  , down(old.down)
  , above(old.above)
  , external(old.external)
  , depth(old.depth)
  , watching(old.watching)
  , guard(old.guard)
  { }

  Head& operator=(const Head&) {
    assert(false);
    exit(1);
    return *this;
  }

  friend std::ostream& operator<<(std::ostream& os, Head const &p);
  void calcReason(Solver& S, Lit p, vec<Lit>& out_reason);
  void *getSpecificPtr() { return this; }

  void set_watch(Solver &S);
  void unset_watch(Solver &S);

  void remove    (Solver& S, bool just_dealloc = false) { };
  bool simplify  (Solver& S) { return false; };

  Head* full_multimove_on_propagate(
    Solver &S,
    WhatToDo what_to_do,
    MinusSnapshot *msnap,
    Head *rear,
    Head *gprev,
    Head *gnext
  );
  Head* full_multimove_on_propagate_solo(
    Solver &S,
    WhatToDo what_to_do,
    MinusSnapshot *msnap
  );
  GClause propagate(Solver& S, Lit p, bool& keep_watch);

  Head* jump(Solver &S);

#ifdef AFA
  void make_rear_psnap(Solver &S, Head *old_deepest_rightmost_rear);
#else
  void make_rear_psnap(Solver &S);
#endif
  void make_van_psnap(Solver &S, int level);
  void *getSpecificPtr2() { return this; }
  MinusSnapshot *save_to_msnap(Trie &trie, MinusSnapshot *msnap);

  unsigned count();
  Head* solidify();
};

struct Horline {
  Head** ptr_to_first;
  Head* above;
  vector<Head> elems;

#ifdef FIXED_ORDER
  unordered_map<int, int> lit_to_ix;
#endif

  Horline(Head** ptr_to_first_, Head *above_) : ptr_to_first(ptr_to_first_), above(above_) {}
};

#ifdef AFA
struct AfaHorline {
  Head* leftmost;
  LogList<Head> elems;

  AfaHorline(Head* leftmost_)
  : leftmost(leftmost_)
  {}
};
#endif

class Trie : public Undoable {
public:
  Head* root = NULL;

  LogList<MinusSnapshot> root_minus_snapshots;

  MultimoveCtx multimove_ctx;
  MultimoveCtx multimove_ctx2;

  unsigned snapshot_count = 0;
  std::vector<Snapshot> snapshots;

#ifdef AFA
  Head* deepest_rightmost_rear = NULL;
  void deepest_rightmost_candidate(Head *rear);
  void onSat(
    Solver &S,
    unsigned clause_count,
    vector<unsigned> &sharing_set,
    vec<Lit> &zero_outputs,
    vector<AfaHorline> &horlines,
    vector<Head*> &verlines
  );
#endif

  Snapshot &get_last_snapshot() { return snapshots[snapshot_count - 1]; }
  Snapshot& new_snapshot();

  Trie(vec<char> &assigns) : multimove_ctx(assigns), multimove_ctx2(assigns) {}

  Head* reset(Solver &S);

  void undo(Solver& S);

  // debugging
  unsigned count();
  Head* solidify();
  void to_dot(Solver& S, const char *filename);
  void print_guards(Solver& S);

  // manual creation
  bool add_clause(
    vector<Lit> &lits,
    Solver &S,
    unsigned clause_count,
    vector<unsigned> &sharing_set,
    vector<Horline> &horlines,
    vector<Head*> &verlines,
    vec<char> &mask
  );
};

//=================================================================================================
#endif
