#include "mystorage.h"
#include <chrono>
#include <thread>
#include <experimental/optional>
#include <opencog/atoms/base/Atom.h>
#include <opencog/guile/SchemeEval.h>

std::pair<atomid, bool> MyStorage::add_atom(opencog::Handle const & atom){
    if (auto id = find_id_by_atom(atom)){
        std::make_pair(*id, false);
    }
    atomid id = slow_storage.size();
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    slow_storage.push_back(atom->to_string());
    auto hash = atom.value();
    hash_to_idx[hash].push_back(id);
    return std::make_pair(id, true);
}

optionalid MyStorage::find_id_by_atom(opencog::Handle const & atom){
    auto it = hash_to_idx.find(atom->get_hash());
    if (it == hash_to_idx.end()){
        return std::nullopt;
    }
    std::vector<atomid> & ids = it->second;
    if(ids.empty()){
        return std::nullopt;
    }
    for(atomid id: ids){
        if (atom->to_string() == slow_storage[id]){
            return id;
        }
    }
    return std::nullopt;
}


opencog::Handle MyStorage::get_atom(opencog::AtomSpace& atomspace, atomid id){
   opencog::SchemeEval * eval = opencog::SchemeEval::get_evaluator(&atomspace);
   using namespace std::chrono_literals;
   std::this_thread::sleep_for(1s);
   return eval->eval_h(slow_storage[id]);
}


std::vector<opencog::Handle> MyStorage::get_atoms(opencog::AtomSpace& atomspace, std::vector<atomid>const & ids){
    opencog::SchemeEval * eval = opencog::SchemeEval::get_evaluator(&atomspace);
    std::vector<opencog::Handle> result;
    result.reserve(ids.size());
    for(atomid id: ids){
       result.push_back(eval->eval_h(slow_storage[id]));
    }
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    return result;
}

std::set<opencog::Handle> MyStorage::get_atoms(opencog::AtomSpace& atomspace, const std::set<atomid> &ids){
    opencog::SchemeEval * eval = opencog::SchemeEval::get_evaluator(&atomspace);
    std::set<opencog::Handle> result;
    for(atomid id: ids){
       result.insert(eval->eval_h(slow_storage[id]));
    }
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    return result;
}
