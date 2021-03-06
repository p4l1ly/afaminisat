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

#ifndef VarOrder_h
#define VarOrder_h

#include "SolverTypes.h"
#include "Heap.h"

//=================================================================================================


struct VarOrder_lt {
    const vec<double>&  activity;
    bool operator () (Var x, Var y) { return activity[x] > activity[y]; }
    VarOrder_lt(const vec<double>&  act) : activity(act) { }
};

class VarOrder {
    const vec<char>&    assigns;       // var->val. Pointer to external assignment table.
    const vec<double>&  activity;      // var->act. Pointer to external activity table.
    const vec<bool>&    pures;
    const vec<int>&     output_map;
    Heap<VarOrder_lt>   heap;
    double              random_seed;   // For the internal random number generator

public:
    VarOrder(
        const vec<char>& ass, const vec<double>& act, const vec<bool>& pures_,
        const vec<int>& outs
    ) : assigns(ass), activity(act), pures(pures_), output_map(outs),
        heap(VarOrder_lt(act)), random_seed(91648253)
    { }

    VarOrder(const vec<char>& ass, const VarOrder& order)
    : assigns(ass)
    , activity(order.activity)
    , pures(order.pures)
    , output_map(order.output_map)
    , heap(order.heap)
    , random_seed(order.random_seed)
    { }

    inline void newVar(void);
    inline void update(Var x);                  // Called when variable increased in activity.
    inline void undo(Var x);                    // Called when variable is unassigned and may be selected again.
    inline Var  select(double random_freq =.0); // Selects a new, unassigned variable (or 'var_Undef' if none exists).
};


void VarOrder::newVar(void)
{
    heap.setBounds(assigns.size());
    int ix = assigns.size() - 1;
    // printf("pure1 %d %d %d\n", ix, pures[ix], output_map[ix]);
    if (!pures[ix] && output_map[ix] == -1)
        heap.insert(ix);
}


void VarOrder::update(Var x)
{
    // printf("pure2 %d %d\n", x, pures[x]);
    if (!pures[x] && output_map[x] == -1 && heap.inHeap(x))
        heap.increase(x);
}


void VarOrder::undo(Var x)
{
    // printf("pure3 %d %d\n", x, pures[x]);
    if (!pures[x] && output_map[x] == -1 && !heap.inHeap(x))
        heap.insert(x);
}


Var VarOrder::select(double random_var_freq)
{
    // Random decision:
    if (drand(random_seed) < random_var_freq){
        Var next = irand(random_seed,assigns.size());
        if (toLbool(assigns[next]) == l_Undef && !pures[next])
            return next;
    }

    // Activity based decision:
    while (!heap.empty()){
        Var next = heap.getmin();
        if (toLbool(assigns[next]) == l_Undef) {
            return next;
        }
    }

    return var_Undef;
}

//=================================================================================================
#endif
