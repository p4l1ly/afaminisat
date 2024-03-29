#include <math.h>
#include "VarOrder.h"
#include "../Solver.h"
#include "../finish_varorder/VarOrder.h"

inline void check(bool expr) { assert(expr); }

void WatchVarOrder::undo(Solver &S) {
  if (verbosity >= 2) {
    std::cout << "VARORDER_UNDO"
      << " " << guess_line
      << " " << snapshots.back()
      << " " << snapshots.size() - 1
      << " " << order.size()
      << " " << (guess_line == order.size() ? -2 : barriers[guess_line].first)
      << "," << (guess_line == order.size() ? -2 : barriers[guess_line].second)
      << " " << (
        snapshots.back() == order.size()
        ? -2 : (snapshots.back() == -1 ? -3 : barriers[snapshots.back()].first)
      )
      << "," << (
        snapshots.back() == order.size()
        ? -2 : (snapshots.back() == -1 ? -3 : barriers[snapshots.back()].second)
      )
      << " " << S.decisionLevel()
      << std::endl;
  }

#ifdef MY_DEBUG
  for (int i = 0; i < guess_line; ++i) {
    if (!watch_infos[order[i]].skipped && assigns[order[i]] == 0) {
      printf("NOOO %d %d\n", i, order[i]); assert(false);
    }
  }
#endif

  if (isinf(tolerance)) {
    guess_line = snapshots.back();
    return;
  }

  unsigned bubble_move_count = 0;
  unsigned this_max_bubble_moves = max_bubble_moves;

  int left_ix = guess_line;
  int right_ix = guess_line;

  if (guess_line != order.size()) {
    pair<int, int> barrier = barriers[guess_line];
    assert(barrier.second == S.decisionLevel() - 1);
    if (barrier.first < barrier.second) {
      --barriers[guess_line].second;
      snapshots.back() = guess_line;
    } else {
      barriers[guess_line] = pair(-1, -1);
      guess_line = snapshots.back();
    }
  } else guess_line = snapshots.back();

  snapshots.pop_back();

  assert(
    guess_line == 0 || guess_line == order.size() ||
    barriers[guess_line].second == S.decisionLevel() - 2
  );

  while (left_ix > guess_line) {
    --left_ix;
    WatchInfo &watch_info = watch_infos[order[left_ix]];
    watch_info.skipped = false;
  }

  for (; right_ix != order.size(); ++right_ix) {
    if (update0(order[right_ix], right_ix, S) == 0) break;
  }

#ifdef MY_DEBUG
  for (int i = 0; i < guess_line; ++i) {
    if (!watch_infos[order[i]].skipped && assigns[order[i]] == 0) {
      printf("NOOO %d %d\n", i, order[i]); assert(false);
    }
  }
#endif
}


void WatchVarOrder::noselect(Solver &S) {
    if (verbosity >= -3) printf("VARORDER_NOSELECT %d\n", guess_line);

    if (guess_line == order.size()) return;

    // applied after assume, so the level is already incremented
    const int level = S.decisionLevel();

    while (guess_line != order.size()) {
      Var next = order[guess_line];
      if (assigns[next] == 0) {
        if (guess_line != order.size()) {
          pair<int, int> &barrier = barriers[guess_line];
          barrier.second = level;
          if (barrier.first == -1) barrier.first = level;
        }
        if (verbosity >= 2) {
          std::cout << "VAR_SELECT " << next << " " << guess_line << " " << level << std::endl;
        }

#ifdef MY_DEBUG
        for (int i = 0; i < guess_line; ++i) {
          if (!watch_infos[order[i]].skipped && assigns[order[i]] == 0) {
            printf("NOOO %d %d\n", i, order[i]); assert(false);
          }
        }
#endif

        return;
      }
      if (verbosity >= -3) printf("MOVING_GUESS_LINE1_NOSELECT %d %d %d %d\n", guess_line, level, guess_line == order.size() ? -1 : order[guess_line], next);
      ++guess_line;
    }
}


void WatchVarOrder::after_select(int old_guess_line1, Solver &S) {
  if (old_guess_line1 == order.size()) return;
  snapshots.push_back(old_guess_line1);
  S.undos.push_back(this);
}


Lit WatchVarOrder::select(Solver &S)
{
    if (verbosity >= -3) printf("VARORDER_SELECT %d\n", guess_line);

#ifdef MY_DEBUG
    for (int i = 0; i < guess_line; ++i) {
      if (!watch_infos[order[i]].skipped && assigns[order[i]] == 0) {
        printf("NOOO %d %d\n", i, order[i]); assert(false);
      }
    }
#endif

    const int level = S.decisionLevel();

    while (!skipped_candidates.empty()) {
      int candidate = skipped_candidates.back();
      if (verbosity >= 2) {
        std::cout << "VARORDER_CANDIDATE"
          << " " << candidate
          << std::endl;
      }

      skipped_candidates.pop_back();

      WatchInfo &watch_info = watch_infos[candidate];
      assert(watch_info.enqueued);
      watch_info.enqueued = false;
      if (
        !watch_info.skipped
        || watch_info.neg_watch_count == 0 && watch_info.pos_watch_count == 0
        || assigns[candidate] != 0
      ) {
        if (verbosity >= 2) {
          std::cout << "VARORDER_NOCANDIDATE"
            << " " << watch_info.skipped
            << " " << watch_info.neg_watch_count
            << "," << watch_info.pos_watch_count
            << " " << (int)assigns[candidate]
            << std::endl;
        }
        continue;
      }

      while (guess_line != order.size()) {
        Var next = order[guess_line];
        if (assigns[next] == 0) {
          if (guess_line != order.size()) {
            pair<int, int> &barrier = barriers[guess_line];
            barrier.second = level;
            if (barrier.first == -1) barrier.first = level;
          }
          if (verbosity >= 2) {
            std::cout << "VAR_SELECT " << next << " " << guess_line << " " << level << std::endl;
          }

#ifdef MY_DEBUG
          for (int i = 0; i < guess_line; ++i) {
            if (!watch_infos[order[i]].skipped && assigns[order[i]] == 0) {
              printf("NOOO %d %d\n", i, order[i]); assert(false);
            }
          }
#endif

          break;
        }
        if (verbosity >= -3) printf("MOVING_GUESS_LINE1_SKIPSELECT %d %d %d %d\n", guess_line, level, guess_line == order.size() ? -1 : order[guess_line], next);
        ++guess_line;
      }

#ifdef AFA
      VarType var_type = S.var_types[candidate];
      bool signum =
        var_type == OUTPUT_POS ? false :
        var_type == OUTPUT_NEG ? true :
        watch_info.pos_watch_count < watch_info.neg_watch_count;
      return Lit(candidate, signum);
#else
      return Lit(candidate, watch_info.pos_watch_count < watch_info.neg_watch_count);
#endif

    }

    // Activity based decision:
    while (guess_line != order.size()) {
      Var next = order[guess_line];
      if (assigns[next] == 0) {
        WatchInfo &watch_info = watch_infos[next];
        if (watch_info.neg_watch_count == 0 && watch_info.pos_watch_count == 0) {
          if (verbosity >= 2) {
            std::cout << "VARORDER_SKIP"
              << " " << next
              << " " << guess_line
              << " " << level
              << std::endl;
          }
          watch_info.skipped = true;

#ifdef AFA
#ifdef WATCH_VARORDER
          if (finishing) S.finish_order.skip(next, level);
#endif
#endif
        } else {
          if (guess_line != order.size()) {
            pair<int, int> &barrier = barriers[guess_line];
            barrier.second = level;
            if (barrier.first == -1) barrier.first = level;
          }
          if (verbosity >= 2) {
            std::cout << "VAR_SELECT"
              << " " << next
              << " " << guess_line
              << " " << level
              << " " << watch_info.pos_watch_count
              << " " << watch_info.neg_watch_count
              << std::endl;
          }

#ifdef MY_DEBUG
          for (int i = 0; i < guess_line; ++i) {
            if (!watch_infos[order[i]].skipped && assigns[order[i]] == 0) {
              printf("NOOO %d %d\n", i, order[i]); assert(false);
            }
          }
#endif

#ifdef AFA
          VarType var_type = S.var_types[next];
          bool signum =
            var_type == OUTPUT_POS ? false :
            var_type == OUTPUT_NEG ? true :
            watch_info.pos_watch_count < watch_info.neg_watch_count;
          return Lit(next, signum);
#else
          return Lit(next, watch_info.pos_watch_count < watch_info.neg_watch_count);
#endif
        }
      }
      if (verbosity >= -3) printf("MOVING_GUESS_LINE1 %d %d %d\n", guess_line, level, guess_line == order.size() ? -1 : order[guess_line]);
      ++guess_line;
    }

    if (verbosity >= 2) std::cout << "VAR_NOSELECT" << std::endl;

#ifdef MY_DEBUG
    for (int i = 0; i < guess_line; ++i) {
      if (!watch_infos[order[i]].skipped && assigns[order[i]] == 0) {
        printf("NOOO %d %d\n", i, order[i]); assert(false);
      }
    }
#endif

    return lit_Undef;
}


void WatchVarOrder::update(Var right, Solver &S) {
  if (verbosity >= -3) printf("VARORDER_UPDATE %d\n", right);

  if (isinf(tolerance)) return;

  update0(right, var_ixs[right], S);
}


bool WatchVarOrder::update0(int right, int right_ix, Solver &S) {
  const int level = assigns[right] == 0 ? std::numeric_limits<int>::max() : S.level[right];

  if (verbosity >= -3) printf(
    "VARORDER_UPDATE0 %d %d %d %d,%d\n",
    right, right_ix, level, barriers[right_ix].first, barriers[right_ix].second
  );

#ifdef MY_DEBUG
  for (int i = 0; i < guess_line; ++i) {
    if (!watch_infos[order[i]].skipped && assigns[order[i]] == 0) {
      printf("NOOO %d %d\n", i, order[i]); assert(false);
    }
  }
#endif

  assert(order[right_ix] == right);
  const double right_activity = activity[right];
  const double max_left_activity = right_activity - tolerance;
  const double max_left_activity_barrier = right_activity + tolerance;
  int bubble_move_count = 0;

  int left = -1;

  pair<int, int> right_barrier = barriers[right_ix];
  assert(right_barrier.first <= right_barrier.second);
  assert((right_barrier.second == -1) == (right_barrier.first == -1));
  if (right_barrier.second != -1) {
    if (verbosity >= -4) printf(
      "BARRIER1 %d %d,%d %d %d\n",
      right,
      right_barrier.first,
      right_barrier.second,
      level,
      right_ix
    );
    assert(right_barrier.first < level);
    goto after_bubbling;
  }
  
  while (right_ix) {
    pair<int, int> left_barrier = barriers[right_ix - 1];
    assert(left_barrier.first <= left_barrier.second);
    assert((left_barrier.second == -1) == (left_barrier.first == -1));

    left = order[right_ix - 1];

    if (left_barrier.second == -1) {
      if (activity[left] >= max_left_activity) break;
    } else  {
      // We want to shift barriers as right as possible - not to backtrack too much.
      if (activity[left] > max_left_activity_barrier) break;
    }

    ++bubble_move_count;
    order[right_ix] = left;
    var_ixs[left] = right_ix;

    if (left_barrier.second != -1) {
      if (verbosity >= -4) printf(
        "LEFT_BARRIER %d,%d %d %d\n",
        left_barrier.first,
        left_barrier.second,
        right_ix,
        level
      );

      if (left_barrier.second < level) {
        if (verbosity >= -4) printf("KEEP_BARRIER\n");
        barriers[right_ix] = pair(-1, -1);
        --right_ix;
        break;
      }

      if (left_barrier.first < level) {
        if (verbosity >= -4) printf("SPLIT_BARRIER\n");
        barriers[right_ix - 1] = pair(left_barrier.first, level - 1);
        unsigned snapshot_ix = level - S.root_level;
        snapshots[snapshot_ix] = right_ix - 1;

        unsigned new_barrier_first = level;

        for (unsigned i = right_ix; i < order.size(); ++i) {
          int ivar = order[i];
          int ilevel = assigns[ivar] == 0 ? std::numeric_limits<int>::max() : S.level[ivar];
          if (verbosity >= 2) {
            std::cout << "VARORDER_IVAR"
              << " " << i
              << " " << ivar
              << " " << ilevel
              << " " << barriers[i].first
              << "," << barriers[i].second
              << std::endl;
          }
          assert (
            barriers[i].first == -1
            || barriers[i].first == left_barrier.second + 1
               && barriers[i].first < ilevel
          );

          if (new_barrier_first < ilevel) {
            if (left_barrier.second < ilevel) {
              if (barriers[i].second == -1) {
                barriers[i] = pair(new_barrier_first, left_barrier.second);

                unsigned snapshot_ix = left_barrier.second + 1 - S.root_level;
                if (snapshot_ix == snapshots.size()) {
                  if (verbosity >= -4) printf("SET_GUESS_LINE %d %d %d\n", guess_line, level, i);
                  guess_line = i;
                } else {
                  if (verbosity >= -4) {
                    printf("SET_SNAPSHOT1 %d %d %d %d\n", snapshot_ix, snapshots[snapshot_ix], level, i);
                  }
                  snapshots[snapshot_ix] = i;
                }
              } else barriers[i].first = new_barrier_first;
              break;
            } else {
              barriers[i] = pair(new_barrier_first, ilevel - 1);
              unsigned snapshot_ix = ilevel - S.root_level;
              if (verbosity >= -4) {
                printf("SET_SNAPSHOT2 %d %d %d %d\n", snapshot_ix, snapshots[snapshot_ix], level, i);
              }
              snapshots[snapshot_ix] = i;
              new_barrier_first = ilevel;
            }
          };
        }

        --right_ix;
        break;
      }

      unsigned snapshot_ix = left_barrier.second + 1 - S.root_level;
      if (snapshot_ix == snapshots.size()) {
        assert(guess_line == right_ix - 1);
        if (verbosity >= -4) printf("MOVING_GUESS_LINE2 %d %d\n", guess_line, level);
        ++guess_line;
      } else {
        if (verbosity >= -4) {
          printf("MOVING_SNAPSHOT1 %d %d %d\n", snapshot_ix, snapshots[snapshot_ix], level);
        }
        assert(snapshots[snapshot_ix] == right_ix - 1);
        ++snapshots[snapshot_ix];
      }
    }

    barriers[right_ix - 1] = pair(-1, -1);
    barriers[right_ix] = left_barrier;
    --right_ix;
  }
after_bubbling:

  if (bubble_move_count) {
    order[right_ix] = right;
    var_ixs[right] = right_ix;
    if (bubble_move_count > max_bubble_moves) {
      if (verbosity >= -3) {
        printf(
          "UPDATE_TOLERANCE_INCREASE %g %g %d\n",
          tolerance, tolerance * tolerance_increase, bubble_move_count
        );
      }
      tolerance *= tolerance_increase;
    }
  }

  if (verbosity >= 2) {
    std::cout
      << "UPDATE0_SUMMARY"
      << " " << right
      << " " << right_ix
      << " " << bubble_move_count
      << " " << left
      << " " << tolerance
      << " " << level
      << " " << S.decisionLevel()
      << " " << guess_line
      << std::endl;
  }

#ifdef MY_DEBUG
  for (int i = 0; i < guess_line; ++i) {
    if (!watch_infos[order[i]].skipped && assigns[order[i]] == 0) {
      printf("NOOO %d %d\n", i, order[i]); assert(false);
    }
  }
#endif

  return bubble_move_count;
}

void WatchVarOrder::watch(Lit lit) {
  WatchInfo &watch_info = watch_infos[var(lit)];

  if (sign(lit)) {
    ++watch_info.neg_watch_count;
  } else {
    ++watch_info.pos_watch_count;
  }

  if (watch_info.skipped && !watch_info.enqueued) {
    watch_info.enqueued = true;
    skipped_candidates.push_back(var(lit));
  }
}

void WatchVarOrder::unwatch(Lit lit) {
  WatchInfo &watch_info = watch_infos[var(lit)];
  if (sign(lit)) {
    assert (watch_info.neg_watch_count);
    --watch_info.neg_watch_count;
  } else {
    assert (watch_info.pos_watch_count);
    --watch_info.pos_watch_count;
  }
}
