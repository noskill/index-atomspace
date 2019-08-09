(BindLink
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
)