cmake_minimum_required(VERSION 3.16)

project(
  afaminisat
  VERSION 0.1.0
  DESCRIPTION "MiniSAT adjusted and optimized to solve AFA emptiness"
  LANGUAGES CXX
)

# find_package(LibLZMA REQUIRED)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DMY_DEBUG")

set(FLAGS "")

option(NEW_ANALYZE "use new analyze function" ON)
option(STRENGTHENCC "use strengthenCC" ON)
option(SOLIDIFY "solidify trie before SAT" ON)
option(ALL_SOLO "don't use rear/van guards but only solo guards (limited unit propagation)" OFF)
option(WATCH_CLAUSE_WATCH "input clauses have impact on watch bubble order" OFF)
option(WATCH_LEARNT_WATCH "learnt clauses have impact on watch bubble order" OFF)
option(NO_POSNEG_OUTPUTS "output literals that appear in both signs are not outputs" ON)
option(ONE_ORDER "guessing of output literals is not prioritized" OFF)
option(NO_OUTPUTS "output literals have no special meaning in the sat (except that they appear in the antichain)" OFF)
option(FIXED_ORDER "trie has fixed variable order" OFF)
option(REVERSE_ADD "add trie branches in reverse level order" OFF)
option(SET_SUCCESSOR_ONE "set nonzero successors to one instead of keeping them unassigned" OFF)
option(PRINT_SAT_TIMES "print timestamps of SAT calls" OFF)

set(MODE "${MODE}" CACHE STRING "modus_operandi" FORCE)

set(FLAGS "${FLAGS} -D${MODE}")
set(AFA OFF)

if (MODE STREQUAL "SAT_TRIE_HEAP")
  set(USE_TRIE ON)
  set(FLAGS "${FLAGS} -DHEAP_VARORDER")
elseif (MODE STREQUAL "SAT_TRIE_BUBBLE")
  set(USE_TRIE ON)
  set(FLAGS "${FLAGS} -DBUBBLE_VARORDER")
elseif (MODE STREQUAL "SAT_TRIE_WATCH")
  set(USE_TRIE ON)
  set(FLAGS "${FLAGS} -DWATCH_VARORDER")
elseif (MODE STREQUAL "SAT_CLAUSE_HEAP")
  set(USE_TRIE OFF)
  set(FLAGS "${FLAGS} -DHEAP_VARORDER")
elseif (MODE STREQUAL "SAT_CLAUSE_BUBBLE")
  set(USE_TRIE OFF)
  set(FLAGS "${FLAGS} -DBUBBLE_VARORDER")
elseif (MODE STREQUAL "SAT_CLAUSE_WATCH")
  set(USE_TRIE OFF)
  set(FLAGS "${FLAGS} -DWATCH_VARORDER")
  set(FLAGS "${FLAGS} -DWATCH_CLAUSE_WATCH")
elseif (MODE STREQUAL "AFA_CLAUSE_HEAP_HEAP")
  set(USE_TRIE OFF)
  set(AFA ON)
  set(FLAGS "${FLAGS} -DHEAP_VARORDER")
  set(FLAGS "${FLAGS} -DHEAP_VARORDER2")
elseif (MODE STREQUAL "AFA_CLAUSE_HEAP_BUBBLE")
  set(USE_TRIE OFF)
  set(AFA ON)
  set(FLAGS "${FLAGS} -DHEAP_VARORDER")
  set(FLAGS "${FLAGS} -DBUBBLE_VARORDER2")
elseif (MODE STREQUAL "AFA_CLAUSE_HEAP_WATCH")
  set(USE_TRIE OFF)
  set(AFA ON)
  set(FLAGS "${FLAGS} -DHEAP_VARORDER")
  set(FLAGS "${FLAGS} -DWATCH_VARORDER2")
  set(FLAGS "${FLAGS} -DWATCH_CLAUSE_WATCH")
elseif (MODE STREQUAL "AFA_CLAUSE_BUBBLE_HEAP")
  set(USE_TRIE OFF)
  set(AFA ON)
  set(FLAGS "${FLAGS} -DBUBBLE_VARORDER")
  set(FLAGS "${FLAGS} -DHEAP_VARORDER2")
elseif (MODE STREQUAL "AFA_CLAUSE_BUBBLE_BUBBLE")
  set(USE_TRIE OFF)
  set(AFA ON)
  set(FLAGS "${FLAGS} -DBUBBLE_VARORDER")
  set(FLAGS "${FLAGS} -DBUBBLE_VARORDER2")
elseif (MODE STREQUAL "AFA_CLAUSE_BUBBLE_WATCH")
  set(USE_TRIE OFF)
  set(AFA ON)
  set(FLAGS "${FLAGS} -DBUBBLE_VARORDER")
  set(FLAGS "${FLAGS} -DWATCH_VARORDER2")
  set(FLAGS "${FLAGS} -DWATCH_CLAUSE_WATCH")
elseif (MODE STREQUAL "AFA_CLAUSE_WATCH_HEAP")
  set(USE_TRIE OFF)
  set(AFA ON)
  set(FLAGS "${FLAGS} -DWATCH_VARORDER")
  set(FLAGS "${FLAGS} -DHEAP_VARORDER2")
  set(FLAGS "${FLAGS} -DWATCH_CLAUSE_WATCH")
elseif (MODE STREQUAL "AFA_CLAUSE_WATCH_BUBBLE")
  set(USE_TRIE OFF)
  set(AFA ON)
  set(FLAGS "${FLAGS} -DWATCH_VARORDER")
  set(FLAGS "${FLAGS} -DBUBBLE_VARORDER2")
  set(FLAGS "${FLAGS} -DWATCH_CLAUSE_WATCH")
elseif (MODE STREQUAL "AFA_CLAUSE_WATCH_WATCH")
  set(USE_TRIE OFF)
  set(AFA ON)
  set(FLAGS "${FLAGS} -DWATCH_VARORDER")
  set(FLAGS "${FLAGS} -DWATCH_VARORDER2")
  set(FLAGS "${FLAGS} -DWATCH_CLAUSE_WATCH")
elseif (MODE STREQUAL "AFA_TRIE_HEAP_HEAP")
  set(USE_TRIE ON)
  set(AFA ON)
  set(FLAGS "${FLAGS} -DHEAP_VARORDER")
  set(FLAGS "${FLAGS} -DHEAP_VARORDER2")
elseif (MODE STREQUAL "AFA_TRIE_HEAP_BUBBLE")
  set(USE_TRIE ON)
  set(AFA ON)
  set(FLAGS "${FLAGS} -DHEAP_VARORDER")
  set(FLAGS "${FLAGS} -DBUBBLE_VARORDER2")
elseif (MODE STREQUAL "AFA_TRIE_HEAP_WATCH")
  set(USE_TRIE ON)
  set(AFA ON)
  set(FLAGS "${FLAGS} -DHEAP_VARORDER")
  set(FLAGS "${FLAGS} -DWATCH_VARORDER2")
  set(FLAGS "${FLAGS} -DWATCH_CLAUSE_WATCH")
elseif (MODE STREQUAL "AFA_TRIE_BUBBLE_HEAP")
  set(USE_TRIE ON)
  set(AFA ON)
  set(FLAGS "${FLAGS} -DBUBBLE_VARORDER")
  set(FLAGS "${FLAGS} -DHEAP_VARORDER2")
elseif (MODE STREQUAL "AFA_TRIE_BUBBLE_BUBBLE")
  set(USE_TRIE ON)
  set(AFA ON)
  set(FLAGS "${FLAGS} -DBUBBLE_VARORDER")
  set(FLAGS "${FLAGS} -DBUBBLE_VARORDER2")
elseif (MODE STREQUAL "AFA_TRIE_BUBBLE_WATCH")
  set(USE_TRIE ON)
  set(AFA ON)
  set(FLAGS "${FLAGS} -DBUBBLE_VARORDER")
  set(FLAGS "${FLAGS} -DWATCH_VARORDER2")
  set(FLAGS "${FLAGS} -DWATCH_CLAUSE_WATCH")
elseif (MODE STREQUAL "AFA_TRIE_WATCH_HEAP")
  set(USE_TRIE ON)
  set(AFA ON)
  set(FLAGS "${FLAGS} -DWATCH_VARORDER")
  set(FLAGS "${FLAGS} -DHEAP_VARORDER2")
  set(FLAGS "${FLAGS} -DWATCH_CLAUSE_WATCH")
elseif (MODE STREQUAL "AFA_TRIE_WATCH_BUBBLE")
  set(USE_TRIE ON)
  set(AFA ON)
  set(FLAGS "${FLAGS} -DWATCH_VARORDER")
  set(FLAGS "${FLAGS} -DBUBBLE_VARORDER2")
  set(FLAGS "${FLAGS} -DWATCH_CLAUSE_WATCH")
elseif (MODE STREQUAL "AFA_TRIE_WATCH_WATCH")
  set(USE_TRIE ON)
  set(AFA ON)
  set(FLAGS "${FLAGS} -DWATCH_VARORDER")
  set(FLAGS "${FLAGS} -DWATCH_VARORDER2")
  set(FLAGS "${FLAGS} -DWATCH_CLAUSE_WATCH")
else()
  message(FATAL_ERROR "Unknown MODE ${MODE}")
endif()

set(TRIESAT_DEPS "")

if (AFA)
  set(TRIESAT_DEPS "${TRIESAT_DEPS};src/afaminisat/MainAfa.C")
else()
  set(TRIESAT_DEPS "${TRIESAT_DEPS};src/afaminisat/MainSat.C")
endif()

set(TRIESAT_DEPS "${TRIESAT_DEPS};src/afaminisat/Solver.C")
set(TRIESAT_DEPS "${TRIESAT_DEPS};src/afaminisat/Constraints.C")
set(TRIESAT_DEPS "${TRIESAT_DEPS};src/afaminisat/LogList.C")
set(TRIESAT_DEPS "${TRIESAT_DEPS};src/afaminisat/VarOrder.C")
set(TRIESAT_DEPS "${TRIESAT_DEPS};src/afaminisat/watch_varorder/VarOrder.C")

if (AFA)
  set(FLAGS "${FLAGS} -DAFA")
  set(TRIESAT_DEPS "${TRIESAT_DEPS};src/afaminisat/finish_varorder/VarOrder.C")
  if (USE_TRIE)
    set(TRIESAT_DEPS "${TRIESAT_DEPS};src/afaminisat/TrieOnSat.C")
  endif()

  set(CELL_CONTAINER "${CELL_CONTAINER}")
  if (CELL_CONTAINER STREQUAL "DFS")
    set(FLAGS "${FLAGS} -DCELL_CONTAINER_DFS")
  elseif (CELL_CONTAINER STREQUAL "BFS")
    set(FLAGS "${FLAGS} -DCELL_CONTAINER_BFS")
  elseif (CELL_CONTAINER STREQUAL "SET")
    set(FLAGS "${FLAGS} -DCELL_CONTAINER_SET")
  elseif (CELL_CONTAINER STREQUAL "")
    set(FLAGS "${FLAGS} -DCELL_CONTAINER_SET")
  else()
    message(FATAL_ERROR "Unknown CELL_CONTAINER ${CELL_CONTAINER}")
  endif()

  option(NOGUESS_VARS "" ON)
  if (NOGUESS_VARS)
    option(POSQ_OUTPUTS "" ON)

    set(FLAGS "${FLAGS} -DNOGUESS_VARS")
    if (POSQ_OUTPUTS)
      set(FLAGS "${FLAGS} -DPOSQ_OUTPUTS")
    endif()
  endif()

  option(OPTIONAL_CLAUSES "" ON)
  if (OPTIONAL_CLAUSES)
    option(UPWARD_CLAUSES "" ON)

    set(FLAGS "${FLAGS} -DOPTIONAL_CLAUSES")
    if (UPWARD_CLAUSES)
      set(FLAGS "${FLAGS} -DUPWARD_CLAUSES")
    endif()
  endif()
endif()

if (USE_TRIE)
  set(TRIESAT_DEPS "${TRIESAT_DEPS};src/afaminisat/Trie.C")
  set(FLAGS "${FLAGS} -DUSE_TRIE")
  set(FLAGS "${FLAGS} -DTRIE_FOR_INPUT")
endif()

if (NEW_ANALYZE)
  set(FLAGS "${FLAGS} -DNEW_ANALYZE")
endif()

if (ALL_SOLO AND USE_TRIE)
  set(FLAGS "${FLAGS} -DALL_SOLO")
endif()

if (STRENGTHENCC)
  set(FLAGS "${FLAGS} -DSTRENGTHENCC")
endif()

if (SOLIDIFY AND USE_TRIE)
  set(FLAGS "${FLAGS} -DSOLIDIFY")
endif()

if (WATCH_LEARNT_WATCH)
  set(FLAGS "${FLAGS} -DWATCH_LEARNT_WATCH")
endif()

if (WATCH_CLAUSE_WATCH)
  set(FLAGS "${FLAGS} -DWATCH_CLAUSE_WATCH")
endif()

if (NO_POSNEG_OUTPUTS)
  set(FLAGS "${FLAGS} -DNO_POSNEG_OUTPUTS")
endif()

if (ONE_ORDER)
  set(FLAGS "${FLAGS} -DONE_ORDER")
endif()

if (NO_OUTPUTS)
  set(FLAGS "${FLAGS} -DNO_POSNEG_OUTPUTS -DNO_OUTPUTS")
endif()

if (FIXED_ORDER)
  set(FLAGS "${FLAGS} -DFIXED_ORDER")
endif()

if (REVERSE_ADD)
  set(FLAGS "${FLAGS} -DREVERSE_ADD")
endif()

if (SET_SUCCESSOR_ONE)
  set(FLAGS "${FLAGS} -DSET_SUCCESSOR_ONE")
endif()

if (PRINT_SAT_TIMES)
  set(FLAGS "${FLAGS} -DPRINT_SAT_TIMES")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${FLAGS}")

add_executable(
  triesat
  ${TRIESAT_DEPS}
)
set_property(TARGET triesat PROPERTY CXX_STANDARD 20)
