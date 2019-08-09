#include "index.h"
#include <opencog/atoms/atom_types/atom_types.h>
#include <opencog/atoms/execution/Instantiator.h>


using namespace opencog;

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
            return tmp.add_node(atom->get_type(), name);
        }
    }
    return atom;
}

std::set<atomid> Index::intersect(opencog::Handle & query){
    /*
    Extracts relevant patterns to the query and intersects their indices
    returns set of atoms ids */
    // put query in tmp atomspace, check if there are relevant patterns

    AtomSpace tmp = AtomSpace(pattern_space.get());
    Handle q = tmp.add_atom(rename(tmp, *pattern_space, query));
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


std::set<opencog::Handle> Index::query(std::shared_ptr<opencog::AtomSpace> result_atomspace,
                                       opencog::Handle & a_query){
    std::set<atomid> res_set = intersect(a_query);
    // add all possible results to tmp atomspace and run query
    // what is called filter step in graph indexing literature
    AtomSpace tmp = AtomSpace();
    std::vector<Handle> extracted;
    for(atomid id: res_set) extracted.push_back(storage->get_atom(id));
    for(Handle item: extracted)
        tmp.add_atom(item);
    // pack query in BindLink
    Handle q = tmp.add_link(opencog::BIND_LINK, {a_query, a_query});
    Handle results = do_execute(tmp, q);
    std::set<Handle> result_atoms;
    for(Handle atom: results->getOutgoingSet())
        result_atoms.insert(result_atomspace->add_atom(atom));
    return result_atoms;
}
