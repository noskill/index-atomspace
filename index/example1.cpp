#include "example1.h"
#include "index.h"
#include "mystorage.h"
#include <opencog/guile/SchemeEval.h>
#include <fstream>
#include <sstream>
#include "indexlink.h"

example1::example1()
{

}

opencog::Handle read_to_atomspace(AtomSpace &  atomspace, std::string path){
    opencog::SchemeEval * eval = opencog::SchemeEval::get_evaluator(&atomspace);
    std::ifstream file(path);
    std::string str((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    opencog::Handle data = eval->eval_h(str);
    return data;
}

int main(int argc, char** argv){
    std::shared_ptr<Storage> store = std::make_shared<MyStorage>();
    std::shared_ptr<opencog::AtomSpace> pattern_space = std::make_shared<opencog::AtomSpace>();
    std::shared_ptr<opencog::AtomSpace> semantic_space = std::make_shared<opencog::AtomSpace>();
    Index idx(store,
          pattern_space,
          semantic_space);
    opencog::AtomSpace tmp;


    // read semantic data
    std::cout << "semantic" << std::endl;
    for(opencog::Handle const & h: read_to_atomspace(*semantic_space, "semantic.scm")->getOutgoingSet()){
        std::cout << h->to_string() << std::endl;
    }

    std::cout << "data" << std::endl;
    for(opencog::Handle const & h: read_to_atomspace(tmp, "data2.scm")->getOutgoingSet()){
        std::cout << h->to_string() << std::endl;
        idx.add_data(h);
    }
    std::cout << "patterns" << std::endl;
    for(opencog::Handle & h: idx.get_patterns()){
        std::cout << h->to_string() << std::endl;
        std::cout << "points to" << std::endl;
        for(opencog::Handle const & q: idx.query(tmp, h->getOutgoingAtom(0))){
            std::cout << q->to_string() << std::endl;
        }
    }

}
