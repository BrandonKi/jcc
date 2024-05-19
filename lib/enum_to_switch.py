import re

#to_string_mode = False
to_string_mode = True

# paste the enum here
enum_string = """
enum class IROp: i8 {
	none,
	iconst8,
	iconst16,
	iconst32,
	iconst64,
	fconst32,
	fconst64,

	mov,
	
	addi,
	subi,
	muli,
	divi,
	modi,
	
	addf,
	subf,
	mulf,
	divf,
	modf,

	lt,
	lte,
	gt,
	gte,
	eq,
	
	br,
	brz,
	brnz,
	call,
	ret,
};
"""

def main():
    indent = " " * 4
    start = enum_string.index("{") + 1
    end = enum_string.rindex("}")
    mem_list = list(filter(None, re.sub(r"\s", "", enum_string[start:end]).split(",")))
    
    for mem in mem_list:
        print(indent * 2 + "case " + mem + ":")
        if to_string_mode:
            print(indent * 3 + "return \"" + mem + "\";")
    print(indent * 2 + "default:")
    if to_string_mode:
        print(indent * 3 + "unreachable")
    
if __name__ == "__main__":
    main()
