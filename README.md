# index-atomspace
graph indexing with atomspace and python

Indexer works by storing lists of patterns that match some graphs in the atomspace.

So Index is hashmap:  
atom -> set of atoms


## index creation

Upon data insertion new(possibly) pattern is created.
The pattern creation function replaces every node in the 
data atom, which is not the part of the semantic memory atomspace.
So currently type information is dismissed.  
Example
```
ImplicationLink
   AndLink
      EvaluationLink
        VariableNode "$X0"
        (ListLink
          VariableNode "$X1"
          VariableNode "$X3"
      EvaluationLink
        VariableNode "$X0"
        ListLink
          VariableNode "$X1"
          VariableNode "$X2"
    EvaluationLink
      VariableNode "$X0"
      ListLink
        VariableNode "$X3"
        VariableNode "$X2"
```

Is created for 
```scheme
(ImplicationLink
    (AndLink
     (EvaluationLink
       (PredicateNode "P")
         (ListLink
           (VariableNode "X")
           (VariableNode "Y")))
     (EvaluationLink
       (PredicateNode "P")
         (ListLink
           (VariableNode "X")
           (VariableNode "Z"))))
 (EvaluationLink
   (PredicateNode "P")
     (ListLink
       (VariableNode "Z")
       (VariableNode "Y"))))
```

Possible optimization is to keep type information in TypedVariableList



## answering query

First query is matched with all patterns to 
find relevant ones. Relevant patterns are those for which there is non-empty intersection in answer sets.  
For example the pattern

```
ImplicationLink
    AndLink
        Variable "A"
        Variable "B"
    Variable "C"
```
Is relevant for the query
```
ImplicationLink
    AndLink
        Concept "S"
        Variable "B"
    Variable "C"
```
since their application would produce answer set which
are both empty, or have common items.

Pattern matcher can solve the task of finding isomorpism query -> pattern which
provides the solution to the task of finding relevant patterns.

When relevant patterns are found their indices are extracted and the 
result of their intersection is placed in temporary atomspace. Then query is applied
to this atomspace to produce the final answer.

## complex query processing with index guided pattern matching

Complex query is query containing multiple closes;

The idea is to interleave variable substitution with batch loading from index in order to improve the performance. Pattern matcher performs search starting from the most deep non-variable atom. In presence of atoms of the same depths, atom with the smallest incoming set is taken.
While optimal process to answer this query in the presence of index and semantic atomspace should be guided by analysis of index patterns and semantic atomspace. 
Search initialization heuristic
The idea is to begin grounding always starting from link with the smallest corresponding index size.

For each non-variable node in query:
 Take enclosing Link
 replace episodic memory nodes with variable
 search among index patterns, if match is found in any of patterns, compute size of the atom set;
Start search from Node in Link with the smallest atom set;

Search procedure:
While substitutions are available:
substitute variable
choose link to start using smallest index heuristic;
repeat

It is possible that the same pattern may be loaded twice using such procedure. To avoid this problem patterns should be marked as loaded.
