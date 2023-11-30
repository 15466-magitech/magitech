#include "TextStorage.hpp"

TextStorage::TextStorage(std::string filename){
    this->load(filename);
}

void TextStorage::load(std::string filename){
    std::ifstream file(filename,std::ios::binary);
    std::vector<char> names;
    std::vector<char> texts;
    read_chunk(file,"str0",&names);

    read_chunk(file,"str1",&texts);


    std::vector<TextHierarchyEntry> hierarchy;
    read_chunk(file,"txth",&hierarchy);

    for(auto &h : hierarchy){
        std::string object_name,object_text;
        if(h.name_begin <= h.name_end && h.name_end <= names.size()){
            object_name = std::string(names.begin() + h.name_begin, names.begin() + h.name_end);
        }

        if(h.text_begin <= h.text_end && h.text_end <= texts.size()){
            object_text = std::string(texts.begin() + h.text_begin, texts.begin() + h.text_end);
        }

        std::vector<std::vector<std::string>> option_vector;
        
        std::vector<size_t> option_positions;
        std::string option_delimiter = "@@";
        std::string line_delimiter = "$";

        size_t pos = object_text.find(option_delimiter,0);
        while(pos!=std::string::npos){
            option_positions.push_back(pos);
            pos = object_text.find(option_delimiter,pos + option_delimiter.size());
        }

        for(size_t i = 0; i < option_positions.size();i++){
            std::vector<size_t> line_positions;
            line_positions.clear();
            pos = option_positions[i];
            size_t pos_next = std::string::npos;
            if(i < option_positions.size()-1)
                pos_next = option_positions[i+1];

            // string of one option
            size_t index_begin = pos + option_delimiter.size();
            size_t length = pos_next - index_begin;
            std::string option = object_text.substr(index_begin,length);

            pos = option.find(line_delimiter,0);
            while(pos!=std::string::npos){
                line_positions.push_back(pos);
                pos = option.find(line_delimiter,pos + line_delimiter.size());
            }

            std::vector<std::string> line_vector;

            // Now we have all line position store in line_positions
            for(size_t j = 0; j < line_positions.size();j++){
                pos = line_positions[j];
                pos_next = std::string::npos;
                if (j<line_positions.size()-1) 
                    pos_next = line_positions[j+1];

                index_begin = pos + line_delimiter.size();
                length = pos_next - index_begin;
                std::string line = option.substr(index_begin,length);
                line_vector.push_back(line);
            }

            option_vector.push_back(line_vector);
        }


        this->object_text_map[object_name] = option_vector;

    }


}