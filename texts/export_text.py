import numpy as np
import sys

class option:
    lines = []

    def __init__(self):
        self.lines = []

class object:
    

    options = []
    name : str
    all_lines = []


    def __init__(self):
        self.options = []
        self.all_lines = []


    def build(self):
        #option_pos store the index of \n line
        option_pos = []

        for i in range(len(self.all_lines)):
            line = self.all_lines[i]
            if line == "\n":
                option_pos.append(i)

        for i in range(len(option_pos)):
            pos = option_pos[i]
            pos_next = option_pos[i+1] if i != len(option_pos) - 1 else -1

            current_option = option()
            if i != len(option_pos) - 1:
                current_option.lines = self.all_lines[pos+1:pos_next]
            else:
                current_option.lines = self.all_lines[pos+1:]

            for i in range(len(current_option.lines)):
                # delete the \n in the last place
                current_option.lines[i] = current_option.lines[i][:-1]

            self.options.append(current_option)

    # Following the rule @@ for option $ for line
    # @@$aaa$aaa@@$bbb
    def serialize_text(self):
        serialized_text = ""
        for i in range(len(self.options)):
            serialized_text += "@@"
            for j in range(len(self.options[i].lines)):
                serialized_text += "$"
                serialized_text += self.options[i].lines[j]

        return serialized_text
    



def parse_text_file(filename : str):
    with open(filename,"r") as f:
        txt = f.readlines()
    objects = []

    object_pos = []
    for i in range(len(txt)):
        line = txt[i]
        if line.find("<:>") != -1:
            object_pos.append(i)

    
    for i in range(len(object_pos)):
        pos = object_pos[i]
        pos_next = object_pos[i+1] if i != len(object_pos) - 1 else -1

        new_object = object()

        if i!=len(object_pos) - 1:
            all_lines = txt[pos:pos_next]
        else:
            all_lines = txt[pos:] 

        new_object.name = all_lines[0][3:-1]
        new_object.all_lines = all_lines[1:]

        new_object.build()

        objects.append(new_object)

    return objects



def write_binary(object_list : [], filename = None):
    names = ""
    texts = ""


    class metadata:
        name_begin = 0
        name_end = 0
        text_begin = 0
        text_end = 0
        
        def __init__(self,nb=0,ne=0,tb=0,te=0):
            self.name_begin = nb
            self.name_end = ne
            self.text_begin = tb
            self.text_end = te

    # build name

    metadata_list = [metadata() for i in range(len(object_list))]

    for i in range(len(object_list)):
        name_begin = len(names)
        name_end = name_begin + len(object_list[i].name)
        names += object_list[i].name

        serialized_text = object_list[i].serialize_text()

        text_begin = len(texts)
        text_end = text_begin + len(serialized_text)

        texts += serialized_text


        metadata_list[i].name_begin = name_begin
        metadata_list[i].name_end = name_end
        metadata_list[i].text_begin = text_begin
        metadata_list[i].text_end = text_end

    path = "./texts/text_binary" if filename is None else filename

    with open(path,"wb") as f:


        # Write names
        magic = "str0"
        size = len(names)
        f.write(bytearray(magic,encoding="ascii"))
        f.write(size.to_bytes(4,sys.byteorder))
        f.write(bytearray(names,encoding="ascii"))

        # Write texts
        magic = "str1"
        size = len(texts)
        f.write(bytearray(magic,encoding="ascii"))
        f.write(size.to_bytes(4,sys.byteorder))
        f.write(bytearray(texts,encoding="ascii"))

        magic = "txth"
        size = len(metadata_list) * 4 * 4 # 4 element in one metadata, each element 4 bytes(uint32_t)
        f.write(bytearray(magic,encoding="ascii"))
        f.write(size.to_bytes(4,sys.byteorder))
        for m in metadata_list:
            f.write(m.name_begin.to_bytes(4,sys.byteorder))
            f.write(m.name_end.to_bytes(4,sys.byteorder))
            f.write(m.text_begin.to_bytes(4,sys.byteorder))
            f.write(m.text_end.to_bytes(4,sys.byteorder))


# result = parse_text_file("./texts/test.txt")
# write_binary(result)      
        

if __name__ == "__main__":
    args = sys.argv[1:]
    if(len(args)!=2):
        print("Usage:export_text.py text.txt text_binary")
    else:
        result = parse_text_file(args[0])
        write_binary(result,args[1])
        print("Done")

