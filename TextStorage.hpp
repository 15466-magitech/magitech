#include <string>
#include "read_write_chunk.hpp"
#include <fstream>
#include <vector>
#include <unordered_map>



/**
 For text of a single object.
 Make sure no @ in actual code
 @@ means new option
 $ means new line
 @@$aaaaa$bbb@@$ccccc@@$ddddd
*/


struct TextHierarchyEntry{
    uint32_t name_begin;
    uint32_t name_end;
    uint32_t text_begin;
    uint32_t text_end;
}; 



// Store a map from object name to vectors of text

class TextStorage{
public:
    std::unordered_map<std::string,std::vector<std::vector<std::string>>> object_text_map;
    void load(std::string filename);
    TextStorage(std::string filename);
};