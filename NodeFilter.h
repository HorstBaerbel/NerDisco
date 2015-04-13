#pragma once

#include "NodeRanged.h"


class NodeFilter : public NodeRanged
{
};

void connect(NodeFilter::SPtr a, NodeRanged::SPtr b);
