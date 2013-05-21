
The library name is "libLLVM_abc".

Run the insertion pass in opt with the "-abc-insert" flag, and
the removal pass with "-abc-remove".

------------------------------------------------------------------------------
                 Transform passes (phase1-gth773s directory)
------------------------------------------------------------------------------

AbcInserter        Performs the bounds check insertion.

AbcInsertionPass   Defines the AbcInsertion FunctionPass.

AbcRemover         Performs the check removal.

AbcRemovalPass     Defines the AbcRemoval FunctionPass.

Pi                 Holds data about a pi node, most notably a reference to
                   the PHINode that was inserted to represent it in the IR.

Diff               A simple type to hold the pair of values A and B.

Inequality         Composed of a Diff (A and B) and an APInt (C).
                   Represents the inequality B - A <= C.
                   Used as an edge in the inequality graph.

Integer            A simple option type for holding an APInt.

LBound             Holds data and logic for memoizing the results
                   of proving inequalities for a particular Diff.

Memo               Maps Diffs to LBounds, to hold all of the memoized
                   proven constraints (the ABCD paper refers to this
                   data structure as "C").

Prover             Implementation of the ABCD demandProve algorithm.

PhaseOne           Takes care of the header file inclusion for all
                   files in the directory.

------------------------------------------------------------------------------
                       Test scripts (test directory)
------------------------------------------------------------------------------

These perl scripts for testing all have a few lines at the top that
need to be configured to point to your llvm/clang directories.

timing-metrics     Runs several combinations of static/dynamic opt
                   with -abc-insert, -gvn, -abc-remove.

compare-bytecode   Compiles a c file, inserts checks, removed checks,
                   then opens the bytecode side-by-side in gvim.

test-correctness   Runs the original, with checks, with checks removed,
                   useful for making sure that the right runs fail.

------------------------------------------------------------------------------
                Experimental ABCD algorithm (java directory)
------------------------------------------------------------------------------

Before attempting the bounds check removal in ABCD, we first tested the
concept with a Java implementation. That code and its single jar dependency
are included here for no particular reason.

