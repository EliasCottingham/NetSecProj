import sys


with open(sys.argv[1], "ab") as myfile:
    name = sys.argv[2]
    byte_string = bytes.fromhex(sys.argv[3])
    pattern_length = len(byte_string)
    name_length = len(name)
    myfile.write(str.encode('{0:02d}'.format(name_length)))
    myfile.write(str.encode("|"+name+"|"))
    myfile.write(str.encode(format(pattern_length, '02d')))
    myfile.write(str.encode("|"))
    myfile.write(byte_string)
    myfile.write(str.encode('\n'))
    myfile.close()