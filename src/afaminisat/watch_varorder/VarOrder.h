/**************************************************************************************************
MiniSat -- Copyright (c) 2003-2005, Niklas Een, Niklas Sorensson

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/

#ifndef WatchVarOrder_h
#define WatchVarOrder_h

#include "../SolverTypes.h"
#include <vector>
#include "../Constraints.h"

class Solver;

using std::vector;
using std::pair;

//=================================================================================================

extern unsigned global_bubble_move_count;
extern unsigned global_bubble_move_count_undo;

struct WatchInfo {
  unsigned pos_watch_count;
  unsigned neg_watch_count;
  bool skipped;
  bool enqueued;
};

class WatchVarOrder: Undoable {
    const vec<char>&    assigns;       // var->val. Pointer to external assignment table.
    const vec<double>&  activity;      // var->act. Pointer to external activity table.
    const vec<bool>&    pures;
    std::vector<Var> order;
    unsigned guess_line = 0;
    std::vector<unsigned> var_ixs;
    std::vector<unsigned> snapshots;
    std::vector<pair<int, int>> barriers;
    std::vector<WatchInfo> watch_infos;
    std::vector<int> skipped_candidates;
    const unsigned max_bubble_moves = 5;

    const double tolerance_decrease = 1.0;
    const double tolerance_increase = 1.05;

    const unsigned min_bubble_move_count_since_last_stage = 1;
    const unsigned min_update_count_since_last_stage = 5;

public:
    double tolerance = 10.0;
    // double tolerance = std::numeric_limits<double>::infinity();

    unsigned bubble_move_count_since_last_stage = 0;
    unsigned update_count_since_last_stage = 0;

    WatchVarOrder(
        const vec<char>& ass, const vec<double>& act, const vec<bool>& pures_
    ) : assigns(ass), activity(act), pures(pures_)
    { }

    inline void newVar(void);
    inline void init(void);
    void update(Var x, Solver &S);                  // Called when variable increased in activity.
    bool update0(int right, int right_ix, Solver &S, int declevel);                  // Called when variable increased in activity.
    void undo(Solver &S);                    // Called when variable is unassigned and may be selected again.
    bool select(Solver &S); // Selects a new, unassigned variable (or 'var_Undef' if none exists).
    void new_stage() {
      if (
        bubble_move_count_since_last_stage < min_bubble_move_count_since_last_stage
        && update_count_since_last_stage >= min_update_count_since_last_stage
      ) {
        if (verbosity >= -3) {
          printf("TOLERANCE_DECREASE %g %g\n", tolerance, tolerance * tolerance_decrease);
        }
        tolerance *= tolerance_decrease;
      }

      bubble_move_count_since_last_stage = 0;
      update_count_since_last_stage = 0;
    }
    void watch(Lit lit);
    void unwatch(Lit lit);
};


void WatchVarOrder::newVar(void)
{
    int ix = assigns.size() - 1;
    if (!pures[ix]) {
        order.push_back(ix);
    }
}


void WatchVarOrder::init() {
  if (order.empty()) return;

  var_ixs.resize(order.size(), -1);
  unsigned i = 0;
  for (Var var: order) var_ixs[var] = i++;

  snapshots.reserve(order.size());
  barriers.resize(order.size(), pair(-1, -1));

  watch_infos.resize(order.size());
}

//=================================================================================================
#endif