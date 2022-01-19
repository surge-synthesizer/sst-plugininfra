# SST PluginInfra

This is a repository where we store several of the core bits of infrastructure we share among our plugins. Currently it
contains the following

1. Our strategy for `std::filesystem` across platforms where there still isnt' an implementation
2. A (modified) version of TinyXML 1.0, the XML parser Surge and ShortCircuit use

Other than the tixml code, the code in this repo is GPL3.