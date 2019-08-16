#include <opencog/atomspace/AtomSpace.h>

#include <opencog/atoms/core/DefineLink.h>
#include <opencog/atoms/core/LambdaLink.h>
#include <opencog/atoms/execution/EvaluationLink.h>
#include <opencog/atoms/pattern/PatternLink.h>
#include <opencog/atoms/core/FindUtils.h>
#include "index.h"
#include "myimplicator.h"

#define DEBUG 1
#ifdef DEBUG
#define DO_LOG(STUFF) STUFF
#else
#define DO_LOG(STUFF)
#endif

using namespace  opencog;

MyImplicator::MyImplicator(AtomSpace* asp, std::shared_ptr<Index> idx):
                   Implicator(asp),
                   InitiateSearchCB(asp),
                   DefaultPatternMatchCB(asp),
                   index(idx)
{}


Handle
MyImplicator::find_starter_recursive(const Handle& h, size_t& depth,
                                         Handle& startrm, size_t& width)
{
    // If its a node, then we are done. Don't modify either depth or
    // start.
    Type t = h->get_type();
    if (nameserver().isNode(t))
    {
        if (VARIABLE_NODE != t and GLOB_NODE != t)
        {
            width = index->cache_size(startrm);
            return h;
        }
        return Handle::UNDEFINED;
    }

    // Ignore all dynamically-evaluatable links up front.
    if (dynamic->find(h) != dynamic->end())
        return Handle::UNDEFINED;

    // Iterate over all the handles in the outgoing set.
    // Find the deepest one that contains a constant, and start
    // the search there.  If there are two at the same depth,
    // then start with the skinnier one.
    size_t deepest = depth;
    Handle hdeepest(Handle::UNDEFINED);
    size_t thinnest = SIZE_MAX;

    for (Handle hunt : h->getOutgoingSet())
    {
        size_t brdepth = depth + 1;
        size_t brwid = SIZE_MAX;

        // The start-term is a term that contains the starting atom...
        // but it cannot be a ChoiceLink; it must be above or below
        // any choice link.
        Handle sbr(startrm);
        if (CHOICE_LINK != t) sbr = h;

        // Blow past the QuoteLinks, since they just screw up the search start.
        if (Quotation::is_quotation_type(hunt->get_type()))
            hunt = hunt->getOutgoingAtom(0);

        Handle s(find_starter_recursive(hunt, brdepth, sbr, brwid));

        if (s)
        {
            // Each ChoiceLink is potentially disconnected from the rest
            // of the graph. Assume the worst case, explore them all.
            if (CHOICE_LINK == t)
            {
                Choice ch;
                ch.clause = _curr_clause;
                ch.best_start = s;
                ch.start_term = sbr;
                _choices.push_back(ch);
            }
            else
            if (brwid < thinnest
                or (brwid == thinnest and deepest < brdepth))
            {
                deepest = brdepth;
                hdeepest = s;
                startrm = sbr;
                thinnest = brwid;
            }
        }
    }
    depth = deepest;
    width = thinnest;
    return hdeepest;
}


bool MyImplicator::neighbor_search(PatternMatchEngine * pme)
{
    // If there are no non-constant clauses, abort; will use
    // no_search() instead.
    if (_pattern->mandatory.empty() and _pattern->optionals.empty()) {
        _search_fail = true;
        return false;
    }

    // Sometimes, the number of mandatory clauses can be zero...
    // or they might all be evaluatable.  In this case, its OK to
    // start searching with an optional clause. But if there ARE
    // mandatories, we must NOT start search on an optional, since,
    // after all, it might be absent!
    bool try_optionals = true;
    for (const Handle& m : _pattern->mandatory)
    {
        if (0 == _pattern->evaluatable_holders.count(m))
        {
            try_optionals = false;
            break;
        }
    }

    const HandleSeq& clauses =
        try_optionals ?  _pattern->optionals :  _pattern->mandatory;

    // In principle, we could start our search at some node, any node,
    // that is not a variable. In practice, the search begins by
    // iterating over the incoming set of the node, and so, if it is
    // large, a huge amount of effort might be wasted exploring
    // dead-ends.  Thus, it pays off to start the search on the
    // node with the smallest ("narrowest" or "thinnest") incoming set
    // possible.  Thus, we look at all the clauses, to find the
    // "thinnest" one.
    //
    // Note also: the user is allowed to specify patterns that have
    // no constants in them at all.  In this case, the search is
    // performed by looping over all links of the given types.
    size_t bestclause;
    Handle best_start = this->find_thinnest(clauses, _pattern->evaluatable_holders,
                                      _starter_term, bestclause);

    // Cannot find a starting point! This can happen if:
    // 1) all of the clauses contain nothing but variables,
    // 2) all of the clauses are evaluatable(!),
    // Somewhat unusual, but it can happen.  For this, we need
    // some other, alternative search strategy.
    if (nullptr == best_start and 0 == _choices.size())
    {
        _search_fail = true;
        return false;
    }

    // If only a single choice, fake it for the loop below.
    if (0 == _choices.size())
    {
        Choice ch;
        ch.clause = bestclause;
        ch.best_start = best_start;
        ch.start_term = _starter_term;
        _choices.push_back(ch);
    }
    else
    {
        // TODO -- weed out duplicates!
    }

    for (const Choice& ch : _choices)
    {
        bestclause = ch.clause;
        best_start = ch.best_start;
        _starter_term = ch.start_term;

        _root = clauses[bestclause];
        DO_LOG({LAZY_LOG_FINE << "Search start node: " << best_start->to_string();})
        DO_LOG({LAZY_LOG_FINE << "Start term is: "
                      << (_starter_term == (Atom*) nullptr ?
                          "UNDEFINED" : _starter_term->to_string());})
        DO_LOG({LAZY_LOG_FINE << "Root clause is: " <<  _root->to_string();})

        // This should be calling the over-loaded virtual method
        // get_incoming_set(), so that, e.g. it gets sorted by attentional
        // focus in the AttentionalFocusCB class...
        IncomingSet iset = get_incoming_set(best_start);
        size_t sz = iset.size();
        for (size_t i = 0; i < sz; i++)
        {
            Handle h(iset[i]);
            DO_LOG({LAZY_LOG_FINE << "xxxxxxxxxx neighbor_search xxxxxxxxxx\n"
                          << "Loop candidate (" << i+1 << "/" << sz << "):\n"
                          << h->to_string();})
            bool found = pme->explore_neighborhood(_root, _starter_term, h);

            // Terminate search if satisfied.
            if (found) return true;
        }
    }

    // If we are here, we have searched the entire neighborhood, and
    // no satisfiable groundings were found.
    return false;
}
