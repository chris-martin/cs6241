# CS 6241 : Advanced Compiler Optimizations

Project phase I : Redundant array bounds check removal

Due date : April 16th 2011

## Description

Our goal is to make C code safe by inserting array bounds checking for each array reference in the code. Array bounds check tests if the loop index is within the bounds of array declarations for static (case such as int a[100]) or dynamic arrays – (int a[n] where n is compile time unknown but run time constant). At each array reference, checks that compute the array index expression are inserted and checked with the corresponding declared bound – e.g., consider the reference, a[2i+5] – the expression 2i+5 will be checked with the lower and upper bound of the declared array before accessing the array element and if violated a runtime exception will be thrown.

For the performance reason, we should identify and remove redundant bounds checks. A bounds check is redundant at a given program point if it is already covered by the preceding checks and its removal does not lead to a removal of exception that would have occurred if it were in place. This redundancy can be removed statically or dynamically. In this project, you are supposed to implement the following;

(1) Baseline: Insert the array bounds checks statically, and measure the code size increase and performance (speed) degradation for a large array intensive benchmark supplied with LLVM. Your pass is invoked using ‘opt’ tool in LLVM.

(2) Static-opt: Remove those bounds checks that are redundant in terms of their expression using PRE (CSE+LICM). For aliasing cases, you can use the existing pointer analysis in LLVM. Again, you do this statically.

(3) Dynamic-opt: Remove redundant bounds checks dynamically. A conference paper is included to give an idea of the kind of analysis that may be useful to determine the redundancy in a dynamic compilation setting. You could develop your own technique or implement the one in the papers or its subset. This time your pass should be invoked at run-time by ‘lli’, LLVM dynamic compiler.

(4) Advanced-opt: You can improve the effectiveness of removal by undertaking more complex analysis such as using path conditions in conjunction with aggressive induction variable analysis to yield more aggressive coverage of checks. In this step you will augment the baseline results of (3) by developing your own analysis which will take out cases on covered by (3). Work with kernels and test cases to uncover the limitation of (3) and then augment it

## Deliverable

You will deliver modified LLVM with a phase built to insert and optimize the bounds checking. You will measure the performance of the schemes and separately compare the improvements with a large array intensive benchmark;  show each result of (1), (2), (3), (2)+(3), (2)+(3)+(4) to understand improvement due to the schemes and their synergy. The report should explain your techniques developed in (3) and (4), the cases found by (3) and not found by it and cases found by (4) and left out. The report should be limited to at most 5 pages.

### Demo

Each team will demo the project setting up a 20 minute slot with the TA.

---------------------------------------

The scope the array bound check: You only need to handle explicitly declared arrays of type static: a[CONST] or dynamic:a[n]. I.e., you do not need to consider heap arrays declared using malloc or other dynamic allocation techniques.

How to make your pass invoked in the dynamic compilation setting: One simple way is to register your pass when internal LLVM passes are initially registered; these passes are supposed to be invoked when a function is encountered for the first time during program execution. You can find the right location to do that in ‘LLVMTargetMachine.cpp’ in ‘CodeGen’ directory.

### Demo (tentative)

There will be demos to grade your project. At the time of the demo, I will give you a sample kernel benchmark to test your code. Two things will be tested; (1) whether or not it detects the array bounds exceeded accesses (2) whether or not your work eliminates redundant bound checks; for this, you need to show before and after the elimination, e.g., printing out the LLVM IR before and after running your pass. Also, to prove your did the work, you might need to run your pass for applications used in phase 0.

### Naming convention

There is no naming convention for the pass and the library name this time.

### Test application to be used in the report

You can pick whatever you want as long as it makes heavy uses of arrays. You might want to use the one that have various cases where the redundant checks can be removed. In addition, for testing the code size increase in (1), run your code with the pre-built LLVM byte codes of the mediabench benchmarks used in phase 0 and show the results in the report.

### Report (Please be succinct)

You are responsible for showing everything required in the description except for the things related with (4) which is now phase 2’s. In the report, you need to briefly mention how you implemented to solve each sub-problem, e.g., what features of llvm you used, clarify what part of ‘ABCD’ you did (did not) implement, and show which kind of bound checks can (cannot) be eliminated with your implementation.

### Submission

You need to submit via tsquare a tar file named "yourgtaccount.tar" that contains a single directory named "yourgtaccount". That directory in turn contains:

(1) a file named README.txt to show how to build/run your pass and what each file does in case you have multiple files.

(2) your entire pass directory named "phase0-yourgtaccount"; make sure that your pass is built successfully.

(3) report.pdf (it should be PDF), plus whatever you think necessary.

