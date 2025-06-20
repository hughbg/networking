import sys

# Do a select on the VDIF header fields output by vheader

if len(sys.argv) != 2:
    print("Usage: select_fields.py LIST_OF_HEADER_FIELD_NAMES (comma separated)")
    print("\nSelect header values from the output of vheader, based on the short field name. Example:")
    print("\n  $ ./vheader x.vdif | head -1  | python select_fields.py num_ch,num_bits")
    print("  num_ch 2 num_bits 2")
    exit(1)
    
fields = sys.argv[1].split(",")

for line in sys.stdin:
    tokens = line[:-1].split()
    assert len(tokens)%2 == 0, "Must be an even number of tokens in a line (field name followed by value, repeated)"
    
    for f in fields:
        try:
            index = tokens.index(f)
            if index == len(tokens)-1:
                print("No value found for ", f)
                exit(1)
            print(f, tokens[index+1], end=" ")
        except:
            print("Field", '"'+f+'"', "doesn't exist")
            exit(1)
            
    print()
            
    
    
