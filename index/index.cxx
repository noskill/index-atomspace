#include "index.h"
#include <opencog/atoms/atom_types/atom_types.h>
#include <opencog/atoms/execution/Instantiator.h>


using namespace opencog;


Index::Index(std::shared_ptr<Storage> store,
      std::shared_ptr<opencog::AtomSpace> pattern_space,
             std::shared_ptr<opencog::AtomSpace> semantic_space):
storage(store),
pattern_space(pattern_space),
semantic_space(semantic_space)
{}

Handle do_execute(AtomSpace& atomspace, Handle h)
{
    Instantiator inst(&atomspace);
    Handle pap(CastFromValue<Atom>(inst.execute(h)));
    if (pap and pap->is_atom())
        return atomspace.add_atom(HandleCast(pap));
    return pap;
}

unsigned char random_char() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    return static_cast<unsigned char>(dis(gen));
}

std::string generate_hex(const unsigned int len) {
    std::stringstream ss;
    for (unsigned int i = 0; i < len; i++) {
        const auto rc = random_char();
        std::stringstream hexstream;
        hexstream << std::hex << int(rc);
        auto hex = hexstream.str();
        ss << (hex.length() < 2 ? '0' + hex : hex);
    }
    return ss.str();
}

inline bool is_in_atomspace(AtomSpace & atomspace, Handle h){
     if (not h) return false;
     Type t = h->get_type();
     if (h->is_node()){
         if (atomspace.get_handle(t, h->get_name()) != Handle::UNDEFINED)
             return true;
         return false;
     }
     std::vector<Handle> handle_vector = h->getOutgoingSet();
     if (h->is_link()){
         if (atomspace.get_handle(t, handle_vector) != Handle::UNDEFINED)
             return true;
         return false;
     }
     throw std::runtime_error("Argument is not link and not node");
}


Handle rename(AtomSpace & tmp, AtomSpace & patternspace, Handle atom){
    if (atom->is_link()){
        std::vector<Handle> args;
        args.reserve(atom->get_arity());
        for(Handle h: atom->getOutgoingSet()){
            args.push_back(rename(tmp, patternspace, h));
        }
        return tmp.add_link(atom->get_type(), args);
    }
    if (atom->get_type() == opencog::VARIABLE_NODE){
        if(is_in_atomspace(patternspace, atom)){
            // todo: check for random collision
            std::string rnd_uuid = generate_hex(6);
            std::string name = atom->get_name() + rnd_uuid;
            return tmp.add_node(GLOB_NODE, name);
        }
    }
    return atom;
}

std::set<atomid> Index::intersect(opencog::Handle const & a_query){
    /*
    Extracts relevant patterns to the query and intersects their indices
    returns set of atoms ids */
    // put query in tmp atomspace, check if there are relevant patterns

    AtomSpace tmp = AtomSpace(pattern_space.get());
    Handle q = tmp.add_atom(rename(tmp, *pattern_space, a_query));
    // check query for exact match:
    Handle exact_match = (Handle)do_execute(tmp, tmp.add_link(opencog::BIND_LINK, {q, q}));
    for(Handle m: exact_match->getOutgoingSet())
        return index[pattern_space->add_link(opencog::BIND_LINK, {m, m})];
    // no exact match: search among all patterns
    // todo: search subgraphs
    std::set<atomid> res_set;
    for(std::map<Handle, std::set<atomid> >::iterator it=index.begin();
        it != index.end();
        it++){
        // pattern is relevant if it matches query
        Handle pat = it->first;
        Handle match = do_execute(tmp, pat);
        for(Handle m: match->getOutgoingSet()){
            if (m == q){
               std::set<atomid> tmpset;
               set_intersection(res_set.begin(),
                                res_set.end(),
                                it->second.begin(),
                                it->second.end(),
                                std::inserter(tmpset, tmpset.begin()));
               res_set = tmpset;
            }
        }
    }
    return res_set;
}


std::set<opencog::Handle> Index::query(opencog::AtomSpace & result_atomspace,
                                       const Handle &a_query){
    std::set<atomid> res_set = intersect(a_query);
    // add all possible results to tmp atomspace and run query
    // what is called filter step in graph indexing literature
    AtomSpace tmp = AtomSpace();
    storage->get_atoms(tmp, res_set);

    // pack query in BindLink
    Handle q = tmp.add_link(opencog::BIND_LINK, {a_query, a_query});
    Handle results = do_execute(tmp, q);
    std::set<Handle> result_atoms;
    for(Handle atom: results->getOutgoingSet())
        result_atoms.insert(result_atomspace.add_atom(atom));
    return result_atoms;
}

size_t Index::cache_size(opencog::Handle const & pattern){
    std::set<atomid> res_set = intersect(pattern);
    return res_set.size();
}

opencog::Handle replace(opencog::Handle const & atom, AtomSpace & semantic_space,
                        AtomSpace & pattern_space, std::map<opencog::Handle,
                        opencog::Handle> & varmapping){
    if (atom->is_link()){
        std::vector<opencog::Handle> args;
        for (opencog::Handle const & x: atom->getOutgoingSet()){
            args.push_back(replace(x, semantic_space,
                                   pattern_space, varmapping));
        }
        return pattern_space.add_link(atom->get_type(), args);
    }
    assert(atom->is_node());
    if (semantic_space.is_in_atomspace(atom))
        return pattern_space.add_atom(atom);
    if (varmapping.find(atom) != varmapping.end())
        return varmapping[atom];
    opencog::Handle new_varnode = pattern_space.add_node(VARIABLE_NODE, "$X" + std::to_string(varmapping.size()));
    varmapping[atom] = new_varnode;
    return new_varnode;
}

void Index::add_pattern(opencog::Handle const & pat){
    assert(pat->get_type() == BIND_LINK);
    opencog::Handle atom = pattern_space->add_atom(pat);
    if(index.find(atom) == index.end()){
        index[atom] = std::set<atomid>();
    }
}

void Index::add_semantic_pattern(opencog::Handle const & atom){
    // find all nodes, that inherit from some atom from semantic atomspace
    // replace such atoms by variable nodes
    std::map<opencog::Handle, opencog::Handle> varmapping;
    opencog::Handle result = replace(atom, *semantic_space, *pattern_space, varmapping);
    opencog::Handle pattern = pattern_space->add_link(BIND_LINK, {result, result});
    add_pattern(pattern);
}

void Index::update_index(AtomSpace & atomspace, opencog::Handle const & data, size_t idx){
    for(std::pair<opencog::Handle, std::set<atomid> > p: index){
        opencog::Handle pat = p.first;
        opencog::Handle match = do_execute(atomspace, pat);
        // patterns can match patterns, so check:
        for(opencog::Handle const & m: match->getOutgoingSet()){
            if (m == data){
                index[pat].insert(idx);
            }
        }
    }
}

void Index::add_data(opencog::Handle const & data){
    std::pair<atomid, bool> res = storage->add_atom(data);

    if (not res.second) return;

    assert(data->is_link());
    add_semantic_pattern(data);

    AtomSpace tmp = AtomSpace(pattern_space.get());
    opencog::Handle content = tmp.add_atom(data);
    size_t idx = res.first;
    update_index(tmp, content, idx);
}

std::vector<opencog::Handle> Index::get_patterns() const {
    std::vector<opencog::Handle> result;
    for (auto const& element : this->index) {
        result.push_back(element.first);
    }
    return result;
}

void Index::print_index(){
    std::cout << "print patterns" << std::endl;
    for(std::pair<Handle, std::set<atomid> >  p : index){
        std::cout << p.first->to_string() << std::endl;
        std::cout << std::to_string(p.second.size()) << std::endl;
    }
}
