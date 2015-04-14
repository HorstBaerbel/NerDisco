#pragma once

#include "ParameterT.h"
#include "NodeRanged.h"
#include "NodeQString.h"

typedef ParameterT<bool, NodeRanged, false> ParameterBool;
typedef ParameterT<int, NodeRanged> ParameterInt;
typedef ParameterT<float, NodeRanged> ParameterFloat;
typedef ParameterT<double, NodeRanged> ParameterDouble;
typedef ParameterT<QString, NodeQString, false> ParameterQString;
