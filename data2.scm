(SetLink
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
)
