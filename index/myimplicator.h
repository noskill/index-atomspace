#ifndef MYIMPLICATOR_H
#define MYIMPLICATOR_H

#include <opencog/query/Implicator.h>
#include <opencog/query/InitiateSearchCB.h>
#include <opencog/query/DefaultPatternMatchCB.h>



class Index;

class MyImplicator:
        public virtual Implicator,
        public virtual InitiateSearchCB,
        public virtual DefaultPatternMatchCB
{
    std::shared_ptr<Index> index;
    const HandleSet* dynamic;
    public:
        MyImplicator(AtomSpace* asp, std::shared_ptr<Index>);

    virtual void set_pattern(const Variables& vars,
                             const Pattern& pat)
    {
        opencog::InitiateSearchCB::set_pattern(vars, pat);
        DefaultPatternMatchCB::set_pattern(vars, pat);
        dynamic = &pat.evaluatable_terms;
    }

    Handle find_starter_recursive(const Handle& h, size_t& depth,
                                             Handle& startrm, size_t& width);
    bool neighbor_search(PatternMatchEngine * pme);
};

#endif // MYIMPLICATOR_H
