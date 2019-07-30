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

### todo:
It is possible to optimize query answering in certain cases, when there is general pattern, that
completely covers indices of more specific ones, or there is a complete match.


