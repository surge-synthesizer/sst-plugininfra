/*
 * sst-plugininfra - an open source library of plugin infrastructure
 * built by Surge Synth Team.
 *
 * Copyright 2018-2025, various authors, as described in the GitHub
 * transaction log.
 *
 * sst-plugininfra is released under the MIT License. It has subordinate
 * libraries with licenses as described in libs/
 *
 * All source in sst-plugininfra available at
 * https://github.com/surge-synthesizer/sst-plugininfra
 */

#include "sst/plugininfra/cpufeatures.h"
#include <iostream>

int main(int argc, char **argv)
{
    using namespace sst::plugininfra::cpufeatures;
    std::cout << "CPU Features Platform\n";
    std::cout << "brand    : " << brand() << "\n";
    std::cout << "isArm    : " << isArm() << "\n";
    std::cout << "isX86    : " << isX86() << "\n";
}