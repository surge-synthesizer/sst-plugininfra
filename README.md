# SST PluginInfra

This is a repository where we store several of the core bits of infrastructure we share among our plugins. Currently it
contains the following

1. Our strategy for `std::filesystem` across platforms where there still isnt' an implementation
2. A (modified) version of TinyXML 1.0, the XML parser Surge and ShortCircuit use 
3. A (modified) version of a natrual string sort (ZLib)

For code outside of libs/ which has a license, the remainder is MIT
