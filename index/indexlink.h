#ifndef INDEXLINK_H
#define INDEXLINK_H
#include <opencog/atoms/pattern/BindLink.h>
#include "index.h"


class IndexLink : public opencog::BindLink
{
    std::shared_ptr<Index> index;
public:
    IndexLink(const opencog::HandleSeq&, Type=BIND_LINK, std::shared_ptr<Index> idx=nullptr );
    IndexLink(const opencog::Handle& vardecl, const Handle& body, const Handle& rewrite, std::shared_ptr<Index>);
    IndexLink(const opencog::Handle& body, const Handle& rewrite, std::shared_ptr<Index>);
};

#endif // INDEXLINK_H
