//
// Maqam - Mobile App Quick Audio & MIDI
// Copyright (C) 2024 TAQS.IM
// SPDX-License-Identifier: MIT
//

#ifndef VALUE_TREE_PROVIDER
#define VALUE_TREE_PROVIDER

#include <juce_data_structures/juce_data_structures.h>

// This is an optional interface for supporting generic state persistence

namespace maqam {

class ValueTreeProvider {
public:
    virtual juce::ValueTree& getValueTree() noexcept = 0;

};

} // namespace

#endif // VALUE_TREE_PROVIDER
