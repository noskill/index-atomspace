(use-modules (opencog logger))
(cog-logger-set-level! (cog-default-logger) "fine")

(EvaluationLink
  (PredicateNode "VA: dislikes")
  (ListLink
    (ConceptNode "VA: person-2")
    (ConceptNode "Chinese")))

(EvaluationLink
  (PredicateNode "VA: likes")
  (ListLink
    (ConceptNode "VA: person-2")
    (ConceptNode "Indian")))

(EvaluationLink
  (PredicateNode "VA: likes")
  (ListLink
    (ConceptNode "VA: person-3")
    (ConceptNode "Indian")))


(EvaluationLink
  (PredicateNode "VA: likes")
  (ListLink
    (ConceptNode "VA: person-1")
    (ConceptNode "Indian")))


(EvaluationLink
  (PredicateNode "VA: user-mentioned-known-person")
  (ListLink
      (ConceptNode "VA: person-1")))

(EvaluationLink
  (PredicateNode "VA: user-mentioned-known-person")
  (ListLink
      (ConceptNode "VA: person-2")))

(InheritanceLink
            (ConceptNode "Indian")
            (ConceptNode "cuisine"))

(InheritanceLink
            (ConceptNode "Chinese")
            (ConceptNode "cuisine"))
(define bl (BindLink
    (VariableList
        (TypedVariableLink
            (VariableNode "$P")
            (TypeNode "ConceptNode"))
        (TypedVariableLink
            (VariableNode "$C")
            (TypeNode "ConceptNode")))
    (AndLink
        (EvaluationLink
            (PredicateNode "VA: user-mentioned-known-person")
            (ListLink
                (VariableNode "$P")))
        (ChoiceLink
            (EvaluationLink
                (PredicateNode "VA: likes")
                (ListLink
                    (VariableNode "$P")
                    (VariableNode "$C")))
            (EvaluationLink
                (PredicateNode "VA: dislikes")
                (ListLink
                    (VariableNode "$P")
                    (VariableNode "$C"))))
        (InheritanceLink
            (VariableNode "$C")
            (ConceptNode "cuisine")))
    (VariableNode "$P") ;-- result
))

(cog-execute! bl)
