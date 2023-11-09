#### txt file format

![image-20231108195029161](/home/kane/gameprogramming/clean_magitech2/texts/image-20231108195029161.png)

starts with object name
`<:>object_name`

Each **option** should start with a line containing **only** `\n` , like line 2 in the picture. Do check if there is invisible space in those lines, it will cause the parsing fail.

change the line if you want multiple line in one option, like line 3 and line 4



There shouldn't be extra `\n` when there is no more text for an object. Like there is no extra line between line 4 and line 5



Line 6,7,8 represents one option for npc(with two lines)
