load(":test_utils.bzl", "test_error", "test_success")

test_success(
    testname = "arith",
    expected = "1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17",
)

test_success(
    testname = "comp",
    expected = "1,2,3,4,5,6,7,8,9,true",
)

test_success(
    testname = "while",
    expected = "15,120",
)

test_success(
    testname = "for",
    expected = "0,1,2,0,0,0",
)

test_success(
    testname = "ternary",
    expected = "10,10,5,5,5",
)

test_success(
    testname = "function",
    expected = "1,2,118,4,5,6",
)

test_success(
    testname = "callee_save",
    expected = "9,9",
)

test_success(
    testname = "spill",
    expected = "0,1,1,2,3,5,8,13,21,34,55,89,144,233,2584",
)

test_success(
    testname = "array",
    expected = "0,1,1,2,3,5,8,13,21,34",
)

test_success(
    testname = "cast",
    expected = "!,!,15,6,6,4096,Derived",
)

test_success(
    testname = "opt",
    expected = "2,73,89,292,88,-2,89",
)

test_success(
    testname = "update",
    expected = "1,2,4,1,9,2,3,3",
)

test_success(
    testname = "incdec",
    expected = "2,3,3,2,1,4,4,5,4,20,5,26,6",
)

test_success(
    testname = "class",
    expected = "5,5,4,4,1,1,2,2,3,18",
)

test_success(
    testname = "strlit",
    expected = "Hello world!,Good-bye!,E\"sca\tpe\nd",
)

test_success(
    testname = "inheritance",
    expected = "100,vFunc,Base,200,vFunc,Derived,dvFunc,nonVFunc,300,vFunc,GrandChild,dvFunc,nonVFunc",
)

test_success(
    testname = "base_method",
    expected = "Child of Derived,Grandchild of Base,5,true,11,false,100,false",
)

test_success(
    testname = "static",
    expected = "Base,Base,Derived",
)

test_success(
    testname = "imports",
    expected = "1,2,3,4,5,6,7,8,9,16",
)

test_error(
    testname = "redef",
    expected = [
        "('Error','2','Redefinition')",
        "('Error','9','Redefinition')",
        "('Error','13','Redefinition')",
        "('Error','17','Redefinition')",
    ],
)

test_error(
    testname = "undef",
    expected = [
        "('Error','2','Undefined')",
        "('Error','7','Undefined')",
        "('Error','12','Undefined')",
        "('Error','13','Undefined')",
        "('Error','20','Undefined')",
    ],
)

test_error(
    testname = "return",
    expected = [
        "('Error','2','Expected')",
        "('Error','7','Expected')",
        "('Error','16','Some')",
        "('Error','26','Some')",
        "('Error','32','Some')",
    ],
)

test_error(
    testname = "param",
    expected = [
        "('Error','2','Undefined')",
        "('Error','6','Undefined')",
    ],
)

test_error(
    testname = "type",
    expected = [
        "('Error','6','Expected')",
        "('Error','7','Expected')",
        "('Error','8','Operator')",
        "('Error','9','Expected')",
        "('Error','10','Expected')",
        "('Error','11','Expected')",
        "('Error','12','Expected')",
        "('Error','19','Comparison')",
        "('Error','26','Operator')",
        "('Error','27','Operator')",
        "('Error','32','Expected')",
    ],
)

test_error(
    testname = "unused",
    expected = [
        "('Warning','1','Unused')",
        "('Warning','2','Unused')",
    ],
)

test_error(
    testname = "imports",
    expected = [
        "('Error','2','No')",
        "('Note','4','Duplicate')",
        "('Error','13','Undefined')",
        "('Error','14','Undefined')",
        "('Error','15','Undefined')",
        "('Error','16','Ambiguous')",
        "('Error','17','Ambiguous')",
        "('Error','18','Undefined')",
        "('Error','20','Undefined')",
        "('Error','23','Ambiguous')",
        "('Error','23','Expected')",
    ],
)

test_error(
    testname = "narrowing",
    expected = [
        "('Warning','2','Narrowing')",
        "('Warning','6','Narrowing')",
        "('Warning','7','Narrowing')",
        "('Warning','10','Narrowing')",
        "('Warning','11','Narrowing')",
        "('Warning','11','Narrowing')",
        "('Warning','11','Narrowing')",
    ],
)

test_error(
    testname = "class",
    expected = [
        "('Error','5','Expected')",
        "('Error','9','Expected')",
        "('Warning','13','Narrowing')",
        "('Error','19','Undefined')",
        "('Error','20','Expected')",
        "('Error','22','Undefined')",
        "('Error','23','Field')",
    ],
)

test_error(
    testname = "rvalue_assign",
    expected = [
        "('Error','7','Operator')",
        "('Error','8','Left')",
        "('Error','9','Left')",
    ],
)

test_error(
    testname = "inheritance",
    expected = [
        "('Error','22','Redefinition')",
        "('Note','22','Method')",
        "('Error','26','Redefinition')",
        "('Error','32','Redefinition')",
        "('Error','34','Redefinition')",
    ],
)

test_error(
    testname = "static",
    expected = [
        "('Error','5','Type')",
        "('Error','5','Undefined')",
        "('Error','13','Undefined')",
    ],
)

test_error(
    testname = "base_method",
    expected = [
        "('Note','10','Method')",
        "('Error','12','Can')",
    ],
)

test_error(
    testname = "final",
    expected = [
        "('Error','5','Cannot')",
        "('Error','10','Cannot')",
        "('Error','15','Cannot')",
        "('Error','16','Cannot')",
        "('Error','17','Cannot')",
        "('Error','23','Cannot')",
        "('Error','24','Cannot')",
    ],
)

test_error(
    testname = "const",
    expected = [
        "('Error','4','Static')",
        "('Error','14','Redefinition')",
        "('Error','19','Expected')",
        "('Error','27','Non-method')",
        "('Error','32','Expected')",
        "('Error','35','Expected')",
        "('Error','36','Expected')",
        "('Error','37','Object')",
    ],
)

test_error(
    testname = "access",
    expected = [
        "('Error','27','Method')",
        "('Error','32','Method')",
        "('Error','39','Method')",
        "('Error','42','Undefined')",
        "('Error','46','Method')",
        "('Error','50','Field')",
        "('Error','53','Redefinition')",
        "('Error','56','Redefinition')",
        "('Error','57','Method')",
        "('Error','58','Override')",
        "('Error','63','Method')",
        "('Error','64','Method')",
        "('Error','67','Method')",
        "('Error','68','Method')",
        "('Error','71','Method')",
        "('Error','72','Method')",
        "('Error','75','Method')",
        "('Error','76','Method')",
        "('Error','79','Field')",
        "('Error','80','Field')",
    ],
)
