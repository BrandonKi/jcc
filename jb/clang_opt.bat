clang -Xclang -disable-O0-optnone -O0 -S -emit-llvm -o output.ll test.c

opt -passes="mem2reg,dot-cfg" -S -o output_opt.ll output.ll

dot -Tsvg .main.dot -o .main.svg