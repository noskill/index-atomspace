#ifndef INDEX_H
#define INDEX_H
#include <opencog/atomspace/AtomSpace.h>
#include <opencog/atoms/base/Handle.h>
#include <map>
#include <set>
#include "storage.h"



class Index{
// index maps subgraph pattern to matching atom ids
std::map<opencog::Handle, std::set<atomid> > index;
std::shared_ptr<Storage> storage;
std::shared_ptr<opencog::AtomSpace> pattern_space;
std::shared_ptr<opencog::AtomSpace> semantic_space;

public:
    Index(std::shared_ptr<Storage> store,
          std::shared_ptr<opencog::AtomSpace> pattern_space,
          std::shared_ptr<opencog::AtomSpace> semantic_space);
    void add_pattern(opencog::Handle const & pattern);
    /*
    Extracts relevant patterns to the query and intersects their indices
    returns set of atoms ids
    */
    std::set<opencog::Handle> query(opencog::AtomSpace &result_atomspace,
                                       opencog::Handle const & a_query);
    std::set<atomid> intersect(opencog::Handle const & query);
    size_t cache_size(opencog::Handle const & pattern);
    void add_data(const opencog::Handle &data);
    void add_semantic_pattern(opencog::Handle const & atom);
    void update_index(opencog::AtomSpace & atomspace, opencog::Handle const & data, size_t idx);
    std::vector<opencog::Handle> get_patterns() const;
    void print_index();
};

#endif // INDEX_H
